#include "InvoiceManager.h"
#include "Enums.h"
#include "JsonUtils.h"
#include "TimeUtils.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

InvoiceManager::InvoiceManager(const BookingManager& bookingManager,
                               const RoomManager& roomManager,
                               const CustomerManager& customerManager,
                               const ServiceManager& serviceManager,
                               const PromotionManager& promotionManager,
                               const std::string& invoicesFilePath,
                               const std::string& invoicesDir)
    : bookingManager(bookingManager),
      roomManager(roomManager),
      customerManager(customerManager),
      serviceManager(serviceManager),
      promotionManager(promotionManager),
      invoicesFilePath(invoicesFilePath),
      invoicesDir(invoicesDir),
      nextInvoiceNumber(1) {
}

int InvoiceManager::extractInvoiceNumber(const std::string& invoiceId) {
    if (invoiceId.size() < 4 || invoiceId.rfind("INV", 0) != 0) {
        return 0;
    }
    for (std::size_t i = 3; i < invoiceId.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(invoiceId[i]))) {
            return 0;
        }
    }
    try {
        return std::stoi(invoiceId.substr(3));
    } catch (...) {
        return 0;
    }
}

const Invoice* InvoiceManager::findByInvoiceId(const std::string& invoiceId) const {
    for (const auto& inv : invoices) {
        if (inv.getInvoiceId() == invoiceId) {
            return &inv;
        }
    }
    return nullptr;
}

std::string InvoiceManager::generateNextInvoiceId() {
    while (true) {
        std::ostringstream oss;
        oss << "INV" << std::setw(3) << std::setfill('0') << nextInvoiceNumber;
        std::string candidate = oss.str();
        ++nextInvoiceNumber;
        if (findByInvoiceId(candidate) == nullptr) {
            return candidate;
        }
    }
}

const Invoice& InvoiceManager::createInvoice(const std::string& bookingId) {
    const Booking* booking = bookingManager.findBooking(bookingId);
    if (booking == nullptr) {
        throw std::runtime_error("Khong tim thay booking de xuat hoa don: " + bookingId);
    }
    if (booking->getStatus() != BookingStatus::CHECKED_OUT) {
        throw std::runtime_error("Chi co the tao hoa don khi booking da check-out (CHECKED_OUT): " + bookingId);
    }
    if (findInvoice(bookingId) != nullptr) {
        throw std::runtime_error("Booking da co hoa don truoc do: " + bookingId);
    }

    const Customer* customer = customerManager.findByCccd(booking->getCccd());
    if (customer == nullptr) {
        throw std::runtime_error("Khong tim thay khach hang co CCCD: " + booking->getCccd());
    }

    const Room* room = roomManager.findRoom(booking->getRoomId());
    if (room == nullptr) {
        throw std::runtime_error("Khong tim thay phong: " + booking->getRoomId());
    }

    int days = TimeUtils::calculateRentalDays(booking->getActualCheckIn(), booking->getActualCheckOut());
    double pricePerDay = room->getPricePerDay();
    double roomCost = static_cast<double>(days) * pricePerDay;
    double serviceCost = serviceManager.calcServiceTotal(bookingId);

    // Khuyen mai lay theo quy cua NGAY CHECK-OUT THUC TE
    int quarter = TimeUtils::getQuarter(booking->getActualCheckOut());
    double discountPercent = promotionManager.getDiscount(quarter, room->getType());
    double discountAmount = (roomCost + serviceCost) * discountPercent / 100.0;
    double total = roomCost + serviceCost - discountAmount;

    std::string invoiceId = generateNextInvoiceId();
    std::string issuedAt = TimeUtils::getCurrentTimeString();

    Invoice newInvoice(
        invoiceId,
        bookingId,
        customer->getCccd(),
        customer->getName(),
        room->getId(),
        room->getType(),
        pricePerDay,
        booking->getActualCheckIn(),
        booking->getActualCheckOut(),
        days,
        roomCost,
        serviceCost,
        discountPercent,
        discountAmount,
        total,
        issuedAt
    );

    // Luu kem chi tiet dich vu trong bo nho de operator<< cung in duoc day du
    std::vector<ServiceUsage> details;
    for (const ServiceUsage* usage : serviceManager.getUsagesByBooking(bookingId)) {
        details.push_back(*usage);
    }
    newInvoice.setServiceDetails(details);

    invoices.push_back(std::move(newInvoice));
    saveToFile(invoicesFilePath);
    return invoices.back();
}

void InvoiceManager::printInvoice(const std::string& bookingId, std::ostream& os) const {
    const Invoice* invoice = findInvoice(bookingId);
    if (invoice == nullptr) {
        throw std::runtime_error("Khong tim thay hoa don cua booking: " + bookingId);
    }
    std::vector<const ServiceUsage*> usages = serviceManager.getUsagesByBooking(bookingId);
    invoice->print(os, usages);
}

std::string InvoiceManager::saveInvoiceToFile(const std::string& bookingId) const {
    const Invoice* invoice = findInvoice(bookingId);
    if (invoice == nullptr) {
        throw std::runtime_error("Khong tim thay hoa don cua booking: " + bookingId);
    }

    std::filesystem::path dirPath(invoicesDir);
    if (!dirPath.empty()) {
        std::filesystem::create_directories(dirPath);
    }

    std::filesystem::path filePath = dirPath / ("hoadon_" + bookingId + ".txt");
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Khong the tao file hoa don: " + filePath.string());
    }

    std::vector<const ServiceUsage*> usages = serviceManager.getUsagesByBooking(bookingId);
    invoice->print(outFile, usages);
    outFile.flush();
    if (!outFile.good()) {
        throw std::runtime_error("Loi khi ghi file hoa don: " + filePath.string());
    }
    return filePath.string();
}

Invoice* InvoiceManager::findInvoice(const std::string& bookingId) {
    for (auto& inv : invoices) {
        if (inv.getBookingId() == bookingId) {
            return &inv;
        }
    }
    return nullptr;
}

const Invoice* InvoiceManager::findInvoice(const std::string& bookingId) const {
    for (const auto& inv : invoices) {
        if (inv.getBookingId() == bookingId) {
            return &inv;
        }
    }
    return nullptr;
}

const std::vector<Invoice>& InvoiceManager::getAll() const {
    return invoices;
}

void InvoiceManager::loadFromFile(const std::string& path) {
    invoicesFilePath = path;
    nlohmann::json arr = JsonUtils::readJsonArray(path);
    invoices.clear();

    int maxNumber = 0;
    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            Invoice loaded = Invoice::fromJson(item);
            if (findInvoice(loaded.getBookingId()) != nullptr ||
                findByInvoiceId(loaded.getInvoiceId()) != nullptr) {
                throw std::runtime_error("Trung ma hoa don hoac booking trong file: " + loaded.getInvoiceId());
            }
            maxNumber = std::max(maxNumber, extractInvoiceNumber(loaded.getInvoiceId()));
            invoices.push_back(std::move(loaded));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 hoa don loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 hoa don khong hop le trong " + path + ": " + ex.what());
        }
    }
    nextInvoiceNumber = maxNumber + 1;
}

void InvoiceManager::saveToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& inv : invoices) {
        arr.push_back(inv.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

