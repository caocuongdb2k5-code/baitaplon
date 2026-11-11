#include "Service.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

bool isBlank(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

std::string formatAmount(double amount) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << amount;
    std::string text = oss.str();

    auto dotPos = text.find('.');
    if (dotPos != std::string::npos) {
        while (!text.empty() && text.back() == '0') {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.') {
            text.pop_back();
        }
    }
    return text;
}

} // namespace

Service::Service(const std::string& serviceId,
                 const std::string& name,
                 double unitPrice,
                 ServiceCategory category)
    : serviceId(""), name(""), unitPrice(0.0), category(category) {
    if (serviceId.empty() || isBlank(serviceId)) {
        throw std::invalid_argument("Ma dich vu khong duoc de trong");
    }
    this->serviceId = serviceId;
    setName(name);
    setUnitPrice(unitPrice);
}

const std::string& Service::getServiceId() const {
    return serviceId;
}

const std::string& Service::getName() const {
    return name;
}

double Service::getUnitPrice() const {
    return unitPrice;
}

ServiceCategory Service::getCategory() const {
    return category;
}

void Service::setName(const std::string& newName) {
    if (newName.empty() || isBlank(newName)) {
        throw std::invalid_argument("Ten dich vu khong duoc de trong");
    }
    name = newName;
}

void Service::setUnitPrice(double newUnitPrice) {
    if (newUnitPrice <= 0.0) {
        throw std::invalid_argument("Don gia dich vu phai lon hon 0");
    }
    unitPrice = newUnitPrice;
}

void Service::setCategory(ServiceCategory newCategory) {
    category = newCategory;
}

std::string Service::toString() const {
    std::ostringstream oss;
    oss << serviceId
        << " | Ten: " << name
        << " | Nhom: " << serviceCategoryToDisplay(category)
        << " | Gia: " << formatAmount(unitPrice) << " VND";
    return oss.str();
}

nlohmann::json Service::toJson() const {
    return nlohmann::json{
        {"serviceId", serviceId},
        {"name", name},
        {"unitPrice", unitPrice},
        {"category", serviceCategoryToString(category)}
    };
}

Service Service::fromJson(const nlohmann::json& j) {
    return Service(
        j.at("serviceId").get<std::string>(),
        j.at("name").get<std::string>(),
        j.at("unitPrice").get<double>(),
        stringToServiceCategory(j.at("category").get<std::string>())
    );
}

bool Service::operator==(const Service& other) const {
    return serviceId == other.serviceId;
}

std::ostream& operator<<(std::ostream& os, const Service& service) {
    os << service.toString();
    return os;
}

