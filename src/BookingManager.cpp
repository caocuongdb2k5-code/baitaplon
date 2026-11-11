#include "BookingManager.h"
#include "Enums.h"
#include "JsonUtils.h"
#include "TimeUtils.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

BookingManager::BookingManager(RoomManager& roomManager,
                               CustomerManager& customerManager,
                               const std::string& roomsFilePath,
                               const std::string& bookingsFilePath)
    : roomManager(roomManager),
      customerManager(customerManager),
      roomsFilePath(roomsFilePath),
      bookingsFilePath(bookingsFilePath),
      nextBookingNumber(1) {
}

int BookingManager::extractBookingNumber(const std::string& bookingId) {
    if (bookingId.size() < 2 || bookingId[0] != 'B') {
        return 0;
    }
    for (std::size_t i = 1; i < bookingId.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(bookingId[i]))) {
            return 0;
        }
    }
    try {
        return std::stoi(bookingId.substr(1));
    } catch (...) {
        return 0;
    }
}

std::string BookingManager::generateNextBookingId() {
    while (true) {
        std::ostringstream oss;
        oss << 'B' << std::setw(3) << std::setfill('0') << nextBookingNumber;
        std::string candidate = oss.str();
        ++nextBookingNumber;
        if (findBooking(candidate) == nullptr) {
            return candidate;
        }
    }
}

Booking& BookingManager::requireBookingMutable(const std::string& bookingId) {
    Booking* booking = findBooking(bookingId);
    if (booking == nullptr) {
        throw std::runtime_error("Khong tim thay booking: " + bookingId);
    }
    return *booking;
}

void BookingManager::saveAll() const {
    roomManager.saveToFile(roomsFilePath);
    saveToFile(bookingsFilePath);
}

const Booking& BookingManager::createBooking(const std::string& cccd,
                                             const std::string& roomId,
                                             const std::string& expIn,
                                             const std::string& expOut) {
    // 1. Kiem tra khach hang ton tai
    if (customerManager.findByCccd(cccd) == nullptr) {
        throw std::runtime_error("Khong tim thay khach hang co CCCD: " + cccd);
    }

    // 2. Kiem tra phong ton tai
    const Room* room = roomManager.findRoom(roomId);
    if (room == nullptr) {
        throw std::runtime_error("Khong tim thay phong: " + roomId);
    }

    // 3. Kiem tra phong dang AVAILABLE
    RoomStatus status = room->getStatus();
    if (status == RoomStatus::BOOKED) {
        throw std::runtime_error("Phong da duoc dat, khong the dat moi: " + roomId);
    }
    if (status == RoomStatus::OCCUPIED) {
        throw std::runtime_error("Phong dang co khach: " + roomId);
    }
    if (status == RoomStatus::CLEANING) {
        throw std::runtime_error("Phong dang don dep, chua the dat: " + roomId);
    }

    // 4. Kiem tra expOut > expIn
    std::time_t inTime = TimeUtils::parseTime(expIn);
    std::time_t outTime = TimeUtils::parseTime(expOut);
    if (std::difftime(outTime, inTime) <= 0.0) {
        throw std::invalid_argument("Thoi gian du kien tra phong phai sau thoi gian nhan phong");
    }

    // 5. Tao ma tu tang khong trung, chuyen trang thai phong va luu file
    std::string newId = generateNextBookingId();
    Booking newBooking(newId, cccd, roomId, expIn, expOut, "-", "-", BookingStatus::RESERVED);

    roomManager.markBooked(roomId);
    bookings.push_back(std::move(newBooking));
    saveAll();
    return bookings.back();
}

void BookingManager::cancelBooking(const std::string& bookingId) {
    Booking& booking = requireBookingMutable(bookingId);
    if (booking.getStatus() != BookingStatus::RESERVED) {
        throw std::runtime_error("Chi co the huy booking o trang thai Da dat (RESERVED): " + bookingId);
    }

    roomManager.markAvailable(booking.getRoomId());
    booking.setStatus(BookingStatus::CANCELLED);
    saveAll();
}

void BookingManager::checkIn(const std::string& bookingId, const std::string& actualCheckInTime) {
    Booking& booking = requireBookingMutable(bookingId);
    if (booking.getStatus() == BookingStatus::CHECKED_IN) {
        throw std::runtime_error("Booking da check-in truoc do: " + bookingId);
    }
    if (booking.getStatus() == BookingStatus::CHECKED_OUT) {
        throw std::runtime_error("Booking da check-out, khong the check-in: " + bookingId);
    }
    if (booking.getStatus() == BookingStatus::CANCELLED) {
        throw std::runtime_error("Booking da bi huy, khong the check-in: " + bookingId);
    }

    const Room* room = roomManager.findRoom(booking.getRoomId());
    if (room == nullptr || room->getStatus() != RoomStatus::BOOKED) {
        throw std::runtime_error("Phong chua duoc dat, khong the check-in: " + booking.getRoomId());
    }

    std::string checkInStr = actualCheckInTime.empty()
                                 ? TimeUtils::getCurrentTimeString()
                                 : actualCheckInTime;
    TimeUtils::parseTime(checkInStr);

    roomManager.markOccupied(booking.getRoomId());
    booking.setActualCheckIn(checkInStr);
    booking.setStatus(BookingStatus::CHECKED_IN);
    saveAll();
}

Booking BookingManager::checkOut(const std::string& bookingId, const std::string& actualCheckOutTime) {
    Booking& booking = requireBookingMutable(bookingId);
    if (booking.getStatus() == BookingStatus::CHECKED_OUT) {
        throw std::runtime_error("Booking da check-out truoc do, khong the check-out 2 lan: " + bookingId);
    }
    if (booking.getStatus() == BookingStatus::RESERVED) {
        throw std::runtime_error("Booking chua check-in, khong the check-out: " + bookingId);
    }
    if (booking.getStatus() == BookingStatus::CANCELLED) {
        throw std::runtime_error("Booking da bi huy, khong the check-out: " + bookingId);
    }

    std::string checkOutStr = actualCheckOutTime.empty()
                                  ? TimeUtils::getCurrentTimeString()
                                  : actualCheckOutTime;
    std::time_t inTime = TimeUtils::parseTime(booking.getActualCheckIn());
    std::time_t outTime = TimeUtils::parseTime(checkOutStr);
    if (std::difftime(outTime, inTime) < 0.0) {
        throw std::invalid_argument("Thoi gian check-out khong duoc truoc thoi gian check-in");
    }

    roomManager.markCleaning(booking.getRoomId());
    booking.setActualCheckOut(checkOutStr);
    booking.setStatus(BookingStatus::CHECKED_OUT);
    saveAll();
    return booking;
}

void BookingManager::finishCleaning(const std::string& roomId) {
    const Room* room = roomManager.findRoom(roomId);
    if (room == nullptr) {
        throw std::runtime_error("Khong tim thay phong: " + roomId);
    }
    if (room->getStatus() != RoomStatus::CLEANING) {
        throw std::runtime_error("Phong khong o trang thai Dang don (CLEANING): " + roomId);
    }

    roomManager.markAvailable(roomId);
    saveAll();
}

Booking* BookingManager::findBooking(const std::string& bookingId) {
    for (auto& booking : bookings) {
        if (booking.getBookingId() == bookingId) {
            return &booking;
        }
    }
    return nullptr;
}

const Booking* BookingManager::findBooking(const std::string& bookingId) const {
    for (const auto& booking : bookings) {
        if (booking.getBookingId() == bookingId) {
            return &booking;
        }
    }
    return nullptr;
}

std::vector<const Booking*> BookingManager::getByCustomer(const std::string& cccd) const {
    std::vector<const Booking*> result;
    for (const auto& booking : bookings) {
        if (booking.getCccd() == cccd) {
            result.push_back(&booking);
        }
    }
    return result;
}

const std::vector<Booking>& BookingManager::getAll() const {
    return bookings;
}

std::vector<const Booking*> BookingManager::getActiveBookings() const {
    std::vector<const Booking*> result;
    for (const auto& booking : bookings) {
        BookingStatus status = booking.getStatus();
        if (status == BookingStatus::RESERVED || status == BookingStatus::CHECKED_IN) {
            result.push_back(&booking);
        }
    }
    return result;
}

void BookingManager::loadFromFile(const std::string& path) {
    bookingsFilePath = path;
    nlohmann::json arr = JsonUtils::readJsonArray(path);
    bookings.clear();

    int maxNumber = 0;
    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            Booking booking = Booking::fromJson(item);
            if (findBooking(booking.getBookingId()) != nullptr) {
                throw std::runtime_error("Trung ma booking trong file: " + booking.getBookingId());
            }
            maxNumber = std::max(maxNumber, extractBookingNumber(booking.getBookingId()));
            bookings.push_back(std::move(booking));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 booking loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 booking khong hop le trong " + path + ": " + ex.what());
        }
    }
    nextBookingNumber = maxNumber + 1;
}

void BookingManager::saveToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& booking : bookings) {
        arr.push_back(booking.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

