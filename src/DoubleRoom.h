#ifndef DOUBLE_ROOM_H
#define DOUBLE_ROOM_H

#include "Room.h"

// Lop phong doi ke thua tu Room
class DoubleRoom : public Room {
public:
    DoubleRoom(const std::string& id, double basePrice, RoomStatus status = RoomStatus::AVAILABLE);
    ~DoubleRoom() override = default;

    std::string getType() const override;
    double getPricePerDay() const override;
    std::string getAmenities() const override;
};

#endif // DOUBLE_ROOM_H

