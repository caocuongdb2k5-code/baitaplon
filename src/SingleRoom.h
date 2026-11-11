#ifndef SINGLE_ROOM_H
#define SINGLE_ROOM_H

#include "Room.h"

// Lop phong don ke thua tu Room
class SingleRoom : public Room {
public:
    SingleRoom(const std::string& id, double basePrice, RoomStatus status = RoomStatus::AVAILABLE);
    ~SingleRoom() override = default;

    std::string getType() const override;
    double getPricePerDay() const override;
    std::string getAmenities() const override;
};

#endif // SINGLE_ROOM_H

