#ifndef VIP_ROOM_H
#define VIP_ROOM_H

#include "Room.h"

// Lop phong VIP ke thua tu Room
class VipRoom : public Room {
public:
    VipRoom(const std::string& id, double basePrice, RoomStatus status = RoomStatus::AVAILABLE);
    ~VipRoom() override = default;

    std::string getType() const override;
    double getPricePerDay() const override;
    std::string getAmenities() const override;
};

#endif // VIP_ROOM_H

