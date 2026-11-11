#ifndef BOOKING_H
#define BOOKING_H

#include "Enums.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop dai dien cho mot don dat phong (chi luu tham chieu CCCD va ma phong)
class Booking {
private:
    std::string bookingId;
    std::string cccd;
    std::string roomId;
    std::string expectedCheckIn;
    std::string expectedCheckOut;
    std::string actualCheckIn;
    std::string actualCheckOut;
    BookingStatus status;

public:
    Booking(const std::string& bookingId,
            const std::string& cccd,
            const std::string& roomId,
            const std::string& expectedCheckIn,
            const std::string& expectedCheckOut,
            const std::string& actualCheckIn = "-",
            const std::string& actualCheckOut = "-",
            BookingStatus status = BookingStatus::RESERVED);

    // Getter
    const std::string& getBookingId() const;
    const std::string& getCccd() const;
    const std::string& getRoomId() const;
    const std::string& getExpectedCheckIn() const;
    const std::string& getExpectedCheckOut() const;
    const std::string& getActualCheckIn() const;
    const std::string& getActualCheckOut() const;
    BookingStatus getStatus() const;

    // Setter cho cac moc thoi gian thuc te va trang thai
    void setActualCheckIn(const std::string& actualCheckIn);
    void setActualCheckOut(const std::string& actualCheckOut);
    void setStatus(BookingStatus status);

    std::string toString() const;
    nlohmann::json toJson() const;
    static Booking fromJson(const nlohmann::json& j);

    bool operator==(const Booking& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Booking& booking);
};

#endif // BOOKING_H

