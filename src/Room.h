#ifndef ROOM_H
#define ROOM_H

#include "Enums.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop co so truu tuong dai dien cho phong khach san
class Room {
private:
    std::string id;
    double basePrice;
    RoomStatus status;

public:
    Room(const std::string& id, double basePrice, RoomStatus status = RoomStatus::AVAILABLE);
    virtual ~Room() = default;

    // Getter
    const std::string& getId() const;
    double getBasePrice() const;
    RoomStatus getStatus() const;

    // Setter co kiem tra du lieu hop le
    void setId(const std::string& id);
    void setBasePrice(double basePrice);
    void setStatus(RoomStatus status);

    // Cac ham thuan ao bat buoc lop con override
    virtual std::string getType() const = 0;
    virtual double getPricePerDay() const = 0;
    virtual std::string getAmenities() const = 0;

    // Cac ham ao hien thi thong tin phong va chuyen doi JSON
    virtual std::string toString() const;
    virtual void display(std::ostream& os = std::cout) const;
    virtual nlohmann::json toJson() const;

    friend std::ostream& operator<<(std::ostream& os, const Room& room);
};

#endif // ROOM_H

