#include "SingleRoom.h"

SingleRoom::SingleRoom(const std::string& id, double basePrice, RoomStatus status)
    : Room(id, basePrice, status) {
}

std::string SingleRoom::getType() const {
    return "SINGLE";
}

double SingleRoom::getPricePerDay() const {
    return getBasePrice();
}

std::string SingleRoom::getAmenities() const {
    return "1 giuong don, TV";
}

