#include "Booking.h"
#include "Customer.h"
#include "TimeUtils.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <stdexcept>

namespace {

bool isBlank(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

void validateTimeOrDash(const std::string& timeStr, const std::string& fieldName) {
    if (timeStr == "-") {
        return;
    }
    if (!TimeUtils::isValidTimeString(timeStr)) {
        throw std::invalid_argument(fieldName + " khong hop le (dinh dang YYYY-MM-DD HH:MM hoac '-'): " + timeStr);
    }
}

} // namespace

Booking::Booking(const std::string& bookingId,
                 const std::string& cccd,
                 const std::string& roomId,
                 const std::string& expectedCheckIn,
                 const std::string& expectedCheckOut,
                 const std::string& actualCheckIn,
                 const std::string& actualCheckOut,
                 BookingStatus status)
    : bookingId(""),
      cccd(""),
      roomId(""),
      expectedCheckIn(""),
      expectedCheckOut(""),
      actualCheckIn("-"),
      actualCheckOut("-"),
      status(status) {
    if (bookingId.empty() || isBlank(bookingId)) {
        throw std::invalid_argument("Ma booking khong duoc de trong");
    }
    Customer::validateCccd(cccd);
    if (roomId.empty() || isBlank(roomId)) {
        throw std::invalid_argument("Ma phong trong booking khong duoc de trong");
    }

    std::time_t expInTime = TimeUtils::parseTime(expectedCheckIn);
    std::time_t expOutTime = TimeUtils::parseTime(expectedCheckOut);
    if (std::difftime(expOutTime, expInTime) <= 0.0) {
        throw std::invalid_argument("Thoi gian du kien tra phong phai sau thoi gian nhan phong");
    }

    validateTimeOrDash(actualCheckIn, "actualCheckIn");
    validateTimeOrDash(actualCheckOut, "actualCheckOut");

    this->bookingId = bookingId;
    this->cccd = cccd;
    this->roomId = roomId;
    this->expectedCheckIn = expectedCheckIn;
    this->expectedCheckOut = expectedCheckOut;
    this->actualCheckIn = actualCheckIn;
    this->actualCheckOut = actualCheckOut;
}

const std::string& Booking::getBookingId() const {
    return bookingId;
}

const std::string& Booking::getCccd() const {
    return cccd;
}

const std::string& Booking::getRoomId() const {
    return roomId;
}

const std::string& Booking::getExpectedCheckIn() const {
    return expectedCheckIn;
}

const std::string& Booking::getExpectedCheckOut() const {
    return expectedCheckOut;
}

const std::string& Booking::getActualCheckIn() const {
    return actualCheckIn;
}

const std::string& Booking::getActualCheckOut() const {
    return actualCheckOut;
}

BookingStatus Booking::getStatus() const {
    return status;
}

void Booking::setActualCheckIn(const std::string& newActualCheckIn) {
    validateTimeOrDash(newActualCheckIn, "actualCheckIn");
    actualCheckIn = newActualCheckIn;
}

void Booking::setActualCheckOut(const std::string& newActualCheckOut) {
    validateTimeOrDash(newActualCheckOut, "actualCheckOut");
    actualCheckOut = newActualCheckOut;
}

void Booking::setStatus(BookingStatus newStatus) {
    status = newStatus;
}

std::string Booking::toString() const {
    std::ostringstream oss;
    oss << "Booking " << bookingId
        << " | CCCD: " << cccd
        << " | Phong: " << roomId
        << " | Du kien: " << expectedCheckIn << " -> " << expectedCheckOut
        << " | Thuc te: " << actualCheckIn << " -> " << actualCheckOut
        << " | Trang thai: " << bookingStatusToDisplay(status);
    return oss.str();
}

nlohmann::json Booking::toJson() const {
    return nlohmann::json{
        {"bookingId", bookingId},
        {"cccd", cccd},
        {"roomId", roomId},
        {"expectedCheckIn", expectedCheckIn},
        {"expectedCheckOut", expectedCheckOut},
        {"actualCheckIn", actualCheckIn},
        {"actualCheckOut", actualCheckOut},
        {"status", bookingStatusToString(status)}
    };
}

Booking Booking::fromJson(const nlohmann::json& j) {
    return Booking(
        j.at("bookingId").get<std::string>(),
        j.at("cccd").get<std::string>(),
        j.at("roomId").get<std::string>(),
        j.at("expectedCheckIn").get<std::string>(),
        j.at("expectedCheckOut").get<std::string>(),
        j.at("actualCheckIn").get<std::string>(),
        j.at("actualCheckOut").get<std::string>(),
        stringToBookingStatus(j.at("status").get<std::string>())
    );
}

bool Booking::operator==(const Booking& other) const {
    return bookingId == other.bookingId;
}

std::ostream& operator<<(std::ostream& os, const Booking& booking) {
    os << booking.toString();
    return os;
}

