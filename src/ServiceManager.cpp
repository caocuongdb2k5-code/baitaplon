#include "ServiceManager.h"
#include "JsonUtils.h"
#include "TimeUtils.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <stdexcept>
#include <system_error>

ServiceManager::ServiceManager(const BookingManager& bookingManager,
                               const std::string& servicesFilePath,
                               const std::string& usagesFilePath)
    : bookingManager(bookingManager),
      servicesFilePath(servicesFilePath),
      usagesFilePath(usagesFilePath) {
}

void ServiceManager::seedDefaultServices() {
    services.clear();
    services.emplace_back("S01", "Giat ui", 50000.0, ServiceCategory::LAUNDRY);
    services.emplace_back("S02", "An sang", 80000.0, ServiceCategory::FOOD);
    services.emplace_back("S03", "Nuoc suoi", 15000.0, ServiceCategory::FOOD);
    services.emplace_back("S04", "Bua toi", 150000.0, ServiceCategory::FOOD);
}

void ServiceManager::addService(const std::string& serviceId,
                                const std::string& name,
                                double unitPrice,
                                ServiceCategory category) {
    Service newService(serviceId, name, unitPrice, category);
    if (findService(serviceId) != nullptr) {
        throw std::runtime_error("Ma dich vu da ton tai: " + serviceId);
    }
    services.push_back(std::move(newService));
    saveServicesToFile(servicesFilePath);
}

void ServiceManager::updateService(const std::string& serviceId,
                                   const std::string& newName,
                                   double newUnitPrice,
                                   ServiceCategory newCategory) {
    Service* service = findService(serviceId);
    if (service == nullptr) {
        throw std::runtime_error("Khong tim thay dich vu: " + serviceId);
    }

    // Validate truoc bang doi tuong tam de khong sua do dang neu du lieu loi
    Service validated(serviceId, newName, newUnitPrice, newCategory);
    service->setName(validated.getName());
    service->setUnitPrice(validated.getUnitPrice());
    service->setCategory(validated.getCategory());
    saveServicesToFile(servicesFilePath);
}

void ServiceManager::removeService(const std::string& serviceId) {
    for (auto it = services.begin(); it != services.end(); ++it) {
        if (it->getServiceId() == serviceId) {
            services.erase(it);
            saveServicesToFile(servicesFilePath);
            return;
        }
    }
    throw std::runtime_error("Khong tim thay dich vu: " + serviceId);
}

Service* ServiceManager::findService(const std::string& serviceId) {
    for (auto& service : services) {
        if (service.getServiceId() == serviceId) {
            return &service;
        }
    }
    return nullptr;
}

const Service* ServiceManager::findService(const std::string& serviceId) const {
    for (const auto& service : services) {
        if (service.getServiceId() == serviceId) {
            return &service;
        }
    }
    return nullptr;
}

const std::vector<Service>& ServiceManager::getAll() const {
    return services;
}

const ServiceUsage& ServiceManager::addUsage(const std::string& bookingId,
                                             const std::string& serviceId,
                                             int quantity) {
    const Booking* booking = bookingManager.findBooking(bookingId);
    if (booking == nullptr) {
        throw std::runtime_error("Khong tim thay booking: " + bookingId);
    }

    BookingStatus status = booking->getStatus();
    if (status == BookingStatus::RESERVED) {
        throw std::runtime_error("Booking chua check-in, khong the ghi dich vu: " + bookingId);
    }
    if (status == BookingStatus::CHECKED_OUT) {
        throw std::runtime_error("Booking da tra phong, khong the ghi dich vu: " + bookingId);
    }
    if (status == BookingStatus::CANCELLED) {
        throw std::runtime_error("Booking da bi huy, khong the ghi dich vu: " + bookingId);
    }

    if (quantity <= 0) {
        throw std::invalid_argument("So luong dich vu phai lon hon 0");
    }

    const Service* service = findService(serviceId);
    if (service == nullptr) {
        throw std::runtime_error("Khong tim thay dich vu: " + serviceId);
    }

    // Luu don gia tai thoi diem su dung (snapshot)
    ServiceUsage usage(
        bookingId,
        service->getServiceId(),
        service->getName(),
        quantity,
        service->getUnitPrice(),
        TimeUtils::getCurrentTimeString()
    );
    usages.push_back(std::move(usage));
    saveUsagesToFile(usagesFilePath);
    return usages.back();
}

std::vector<const ServiceUsage*> ServiceManager::getUsagesByBooking(const std::string& bookingId) const {
    std::vector<const ServiceUsage*> result;
    for (const auto& usage : usages) {
        if (usage.getBookingId() == bookingId) {
            result.push_back(&usage);
        }
    }
    return result;
}

double ServiceManager::calcServiceTotal(const std::string& bookingId) const {
    double total = 0.0;
    for (const auto& usage : usages) {
        if (usage.getBookingId() == bookingId) {
            total += usage.getTotal();
        }
    }
    return total;
}

void ServiceManager::loadServicesFromFile(const std::string& path) {
    servicesFilePath = path;
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        seedDefaultServices();
        saveServicesToFile(path);
        return;
    }

    nlohmann::json arr = JsonUtils::readJsonArray(path);
    services.clear();
    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            Service service = Service::fromJson(item);
            if (findService(service.getServiceId()) != nullptr) {
                throw std::runtime_error("Trung ma dich vu trong file: " + service.getServiceId());
            }
            services.push_back(std::move(service));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 dich vu loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 dich vu khong hop le trong " + path + ": " + ex.what());
        }
    }
}

void ServiceManager::saveServicesToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& service : services) {
        arr.push_back(service.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

void ServiceManager::loadUsagesFromFile(const std::string& path) {
    usagesFilePath = path;
    nlohmann::json arr = JsonUtils::readJsonArray(path);
    usages.clear();

    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            usages.push_back(ServiceUsage::fromJson(item));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 dong su dung dich vu loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 dong su dung dich vu khong hop le trong " + path + ": " + ex.what());
        }
    }
}

void ServiceManager::saveUsagesToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& usage : usages) {
        arr.push_back(usage.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

