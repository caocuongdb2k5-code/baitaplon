#include "VipRoom.h"

VipRoom::VipRoom(const std::string& id, double basePrice, RoomStatus status)
    : Room(id, basePrice, status) {
}

std::string VipRoom::getType() const {
    return "VIP";
}

double VipRoom::getPricePerDay() const {
    return getBasePrice() * 1.5;
}

std::string VipRoom::getAmenities() const {
    return "giuong lon, bon tam, an sang, minibar";
}

