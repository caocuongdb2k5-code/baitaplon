#include "Promotion.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

std::string formatPercent(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
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

Promotion::Promotion(int quarter,
                     const std::string& roomType,
                     double discountPercent,
                     const std::string& description)
    : quarter(0), roomType(""), discountPercent(0.0), description(description) {
    validateQuarter(quarter);
    std::string normalizedType = normalizeRoomType(roomType);
    validateDiscountPercent(discountPercent);

    this->quarter = quarter;
    this->roomType = normalizedType;
    this->discountPercent = discountPercent;
}

int Promotion::getQuarter() const {
    return quarter;
}

const std::string& Promotion::getRoomType() const {
    return roomType;
}

double Promotion::getDiscountPercent() const {
    return discountPercent;
}

const std::string& Promotion::getDescription() const {
    return description;
}

void Promotion::setDiscountPercent(double newDiscountPercent) {
    validateDiscountPercent(newDiscountPercent);
    discountPercent = newDiscountPercent;
}

void Promotion::setDescription(const std::string& newDescription) {
    description = newDescription;
}

void Promotion::validateQuarter(int quarter) {
    if (quarter < 1 || quarter > 4) {
        throw std::invalid_argument("Quy khuyen mai phai tu 1 den 4");
    }
}

std::string Promotion::normalizeRoomType(const std::string& roomType) {
    std::string upper = roomType;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    if (upper != "SINGLE" && upper != "DOUBLE" && upper != "VIP" && upper != "ALL") {
        throw std::invalid_argument(
            "Loai phong khuyen mai khong hop le (chi ho tro SINGLE, DOUBLE, VIP, ALL): " + roomType
        );
    }
    return upper;
}

void Promotion::validateDiscountPercent(double discountPercent) {
    if (discountPercent < 0.0 || discountPercent > 100.0) {
        throw std::invalid_argument("Phan tram giam gia phai tu 0 den 100");
    }
}

std::string Promotion::toString() const {
    std::ostringstream oss;
    oss << "Quy " << quarter
        << " | Loai phong: " << roomType
        << " | Giam: " << formatPercent(discountPercent) << "%"
        << " | Mo ta: " << description;
    return oss.str();
}

nlohmann::json Promotion::toJson() const {
    return nlohmann::json{
        {"quarter", quarter},
        {"roomType", roomType},
        {"discountPercent", discountPercent},
        {"description", description}
    };
}

Promotion Promotion::fromJson(const nlohmann::json& j) {
    return Promotion(
        j.at("quarter").get<int>(),
        j.at("roomType").get<std::string>(),
        j.at("discountPercent").get<double>(),
        j.at("description").get<std::string>()
    );
}

bool Promotion::operator==(const Promotion& other) const {
    return quarter == other.quarter && roomType == other.roomType;
}

std::ostream& operator<<(std::ostream& os, const Promotion& promotion) {
    os << promotion.toString();
    return os;
}

