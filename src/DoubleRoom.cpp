#include "DoubleRoom.h"

DoubleRoom::DoubleRoom(const std::string& id, double basePrice, RoomStatus status)
    : Room(id, basePrice, status) {
}

std::string DoubleRoom::getType() const {
    return "DOUBLE";
}

double DoubleRoom::getPricePerDay() const {
    return getBasePrice() * 1.2;
}

std::string DoubleRoom::getAmenities() const {
    return "1 giuong doi, TV, dieu hoa";
}

