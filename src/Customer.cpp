#include "Customer.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace {

// Kiem tra chuoi chi gom dung N chu so hay khong
bool isDigitsWithLength(const std::string& str, std::size_t expectedLen) {
    if (str.size() != expectedLen) {
        return false;
    }
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

// Kiem tra chuoi rong hoac chi gom khoang trang
bool isBlank(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

} // namespace

Customer::Customer(const std::string& cccd, const std::string& name, const std::string& phone)
    : cccd(""), name(""), phone("") {
    validateCccd(cccd);
    validateName(name);
    validatePhone(phone);

    this->cccd = cccd;
    this->name = name;
    this->phone = phone;
}

const std::string& Customer::getCccd() const {
    return cccd;
}

const std::string& Customer::getName() const {
    return name;
}

const std::string& Customer::getPhone() const {
    return phone;
}

void Customer::setName(const std::string& newName) {
    validateName(newName);
    name = newName;
}

void Customer::setPhone(const std::string& newPhone) {
    validatePhone(newPhone);
    phone = newPhone;
}

void Customer::validateCccd(const std::string& cccd) {
    if (!isDigitsWithLength(cccd, 12)) {
        throw std::invalid_argument("CCCD phai gom dung 12 chu so");
    }
}

void Customer::validatePhone(const std::string& phone) {
    if (!isDigitsWithLength(phone, 10)) {
        throw std::invalid_argument("So dien thoai phai gom dung 10 chu so");
    }
}

void Customer::validateName(const std::string& name) {
    if (name.empty() || isBlank(name)) {
        throw std::invalid_argument("Ten khach hang khong duoc de trong");
    }
}

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "CCCD: " << cccd
        << " | Ten: " << name
        << " | SDT: " << phone;
    return oss.str();
}

nlohmann::json Customer::toJson() const {
    return nlohmann::json{
        {"cccd", cccd},
        {"name", name},
        {"phone", phone}
    };
}

Customer Customer::fromJson(const nlohmann::json& j) {
    return Customer(
        j.at("cccd").get<std::string>(),
        j.at("name").get<std::string>(),
        j.at("phone").get<std::string>()
    );
}

bool Customer::operator==(const Customer& other) const {
    return cccd == other.cccd;
}

std::ostream& operator<<(std::ostream& os, const Customer& customer) {
    os << customer.toString();
    return os;
}

