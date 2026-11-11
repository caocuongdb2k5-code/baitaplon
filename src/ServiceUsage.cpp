#include "ServiceUsage.h"
#include "TimeUtils.h"
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

ServiceUsage::ServiceUsage(const std::string& bookingId,
                           const std::string& serviceId,
                           const std::string& serviceName,
                           int quantity,
                           double unitPriceAtUse,
                           const std::string& usedAt)
    : bookingId(""),
      serviceId(""),
      serviceName(""),
      quantity(0),
      unitPriceAtUse(0.0),
      usedAt("") {
    if (bookingId.empty() || isBlank(bookingId)) {
        throw std::invalid_argument("Ma booking trong ServiceUsage khong duoc de trong");
    }
    if (serviceId.empty() || isBlank(serviceId)) {
        throw std::invalid_argument("Ma dich vu trong ServiceUsage khong duoc de trong");
    }
    if (serviceName.empty() || isBlank(serviceName)) {
        throw std::invalid_argument("Ten dich vu trong ServiceUsage khong duoc de trong");
    }
    if (quantity <= 0) {
        throw std::invalid_argument("So luong dich vu phai lon hon 0");
    }
    if (unitPriceAtUse <= 0.0) {
        throw std::invalid_argument("Don gia dich vu phai lon hon 0");
    }
    if (!TimeUtils::isValidTimeString(usedAt)) {
        throw std::invalid_argument("Thoi gian su dung dich vu khong hop le: " + usedAt);
    }

    this->bookingId = bookingId;
    this->serviceId = serviceId;
    this->serviceName = serviceName;
    this->quantity = quantity;
    this->unitPriceAtUse = unitPriceAtUse;
    this->usedAt = usedAt;
}

const std::string& ServiceUsage::getBookingId() const {
    return bookingId;
}

const std::string& ServiceUsage::getServiceId() const {
    return serviceId;
}

const std::string& ServiceUsage::getServiceName() const {
    return serviceName;
}

int ServiceUsage::getQuantity() const {
    return quantity;
}

double ServiceUsage::getUnitPriceAtUse() const {
    return unitPriceAtUse;
}

const std::string& ServiceUsage::getUsedAt() const {
    return usedAt;
}

double ServiceUsage::getTotal() const {
    return static_cast<double>(quantity) * unitPriceAtUse;
}

std::string ServiceUsage::toString() const {
    std::ostringstream oss;
    oss << "Booking " << bookingId
        << " | " << serviceId << " - " << serviceName
        << " | SL: " << quantity
        << " | Don gia luc dung: " << formatAmount(unitPriceAtUse)
        << " | Thanh tien: " << formatAmount(getTotal()) << " VND"
        << " | Luc: " << usedAt;
    return oss.str();
}

nlohmann::json ServiceUsage::toJson() const {
    return nlohmann::json{
        {"bookingId", bookingId},
        {"serviceId", serviceId},
        {"serviceName", serviceName},
        {"quantity", quantity},
        {"unitPriceAtUse", unitPriceAtUse},
        {"usedAt", usedAt}
    };
}

ServiceUsage ServiceUsage::fromJson(const nlohmann::json& j) {
    return ServiceUsage(
        j.at("bookingId").get<std::string>(),
        j.at("serviceId").get<std::string>(),
        j.at("serviceName").get<std::string>(),
        j.at("quantity").get<int>(),
        j.at("unitPriceAtUse").get<double>(),
        j.at("usedAt").get<std::string>()
    );
}

std::ostream& operator<<(std::ostream& os, const ServiceUsage& usage) {
    os << usage.toString();
    return os;
}

