#ifndef BOOKING_MANAGER_H
#define BOOKING_MANAGER_H

#include "Booking.h"
#include "CustomerManager.h"
#include "RoomManager.h"
#include <string>
#include <vector>

// Lop quan ly danh sach dat phong va dieu phoi may trang thai phong
class BookingManager {
private:
    RoomManager& roomManager;
    CustomerManager& customerManager;
    std::vector<Booking> bookings;
    std::string roomsFilePath;
    std::string bookingsFilePath;
    int nextBookingNumber;

    static int extractBookingNumber(const std::string& bookingId);
    std::string generateNextBookingId();
    Booking& requireBookingMutable(const std::string& bookingId);
    void saveAll() const;

public:
    BookingManager(RoomManager& roomManager,
                   CustomerManager& customerManager,
                   const std::string& roomsFilePath = "data/rooms.json",
                   const std::string& bookingsFilePath = "data/bookings.json");

    // Nghiep vu dat, huy, check-in, check-out, don phong
    const Booking& createBooking(const std::string& cccd,
                                 const std::string& roomId,
                                 const std::string& expIn,
                                 const std::string& expOut);
    void cancelBooking(const std::string& bookingId);
    void checkIn(const std::string& bookingId, const std::string& actualCheckInTime = "");
    Booking checkOut(const std::string& bookingId, const std::string& actualCheckOutTime = "");
    void finishCleaning(const std::string& roomId);

    // Tim kiem & truy van
    Booking* findBooking(const std::string& bookingId);
    const Booking* findBooking(const std::string& bookingId) const;
    std::vector<const Booking*> getByCustomer(const std::string& cccd) const;
    const std::vector<Booking>& getAll() const;
    std::vector<const Booking*> getActiveBookings() const;

    // Doc va luu file JSON
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
};

#endif // BOOKING_MANAGER_H

