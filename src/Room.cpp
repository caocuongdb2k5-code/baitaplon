#include "Room.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

// Kiem tra chuoi rong hoac chi chua khoang trang
bool isBlank(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

// Dinh dang gia tien gon gang, tranh ky hieu khoa hoc (vd 5e+05)
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

Room::Room(const std::string& id, double basePrice, RoomStatus status)
    : id(""), basePrice(0.0), status(status) {
    setId(id);
    setBasePrice(basePrice);
}

const std::string& Room::getId() const {
    return id;
}

double Room::getBasePrice() const {
    return basePrice;
}

RoomStatus Room::getStatus() const {
    return status;
}

void Room::setId(const std::string& newId) {
    if (newId.empty() || isBlank(newId)) {
        throw std::invalid_argument("Ma phong khong duoc de trong");
    }
    id = newId;
}

void Room::setBasePrice(double newBasePrice) {
    if (newBasePrice <= 0.0) {
        throw std::invalid_argument("Gia co ban phai lon hon 0");
    }
    basePrice = newBasePrice;
}

void Room::setStatus(RoomStatus newStatus) {
    status = newStatus;
}

std::string Room::toString() const {
    std::ostringstream oss;
    oss << "Phong " << id
        << " | Loai: " << getType()
        << " | Gia/ngay: " << formatAmount(getPricePerDay()) << " VND"
        << " | Trang thai: " << roomStatusToDisplay(status)
        << " | Tien ich: " << getAmenities();
    return oss.str();
}

void Room::display(std::ostream& os) const {
    os << toString() << '\n';
}

nlohmann::json Room::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"type", getType()},
        {"basePrice", basePrice},
        {"status", roomStatusToString(status)}
    };
}

std::ostream& operator<<(std::ostream& os, const Room& room) {
    os << room.toString();
    return os;
}

