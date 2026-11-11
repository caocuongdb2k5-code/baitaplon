#ifndef ENUMS_H
#define ENUMS_H

#include <stdexcept>
#include <string>

// Trang thai phong khach san
enum class RoomStatus { AVAILABLE, BOOKED, OCCUPIED, CLEANING };

// Chuyen RoomStatus sang chuoi hien thi tieng Viet khong dau
inline std::string roomStatusToDisplay(RoomStatus status) {
  switch (status) {
  case RoomStatus::AVAILABLE:
    return "Trong";
  case RoomStatus::BOOKED:
    return "Dang dat";
  case RoomStatus::OCCUPIED:
    return "Dang thue";
  case RoomStatus::CLEANING:
    return "Dang don";
  }
  throw std::invalid_argument("Trang thai phong khong hop le");
}

// Chuyen RoomStatus sang chuoi luu file
inline std::string roomStatusToString(RoomStatus status) {
  switch (status) {
  case RoomStatus::AVAILABLE:
    return "AVAILABLE";
  case RoomStatus::BOOKED:
    return "BOOKED";
  case RoomStatus::OCCUPIED:
    return "OCCUPIED";
  case RoomStatus::CLEANING:
    return "CLEANING";
  }
  throw std::invalid_argument("Trang thai phong khong hop le");
}

// Chuyen chuoi luu file sang RoomStatus
inline RoomStatus stringToRoomStatus(const std::string &str) {
  if (str == "AVAILABLE")
    return RoomStatus::AVAILABLE;
  if (str == "BOOKED")
    return RoomStatus::BOOKED;
  if (str == "OCCUPIED")
    return RoomStatus::OCCUPIED;
  if (str == "CLEANING")
    return RoomStatus::CLEANING;
  throw std::invalid_argument("Chuoi trang thai phong khong hop le: " + str);
}

// Trang thai don dat phong
enum class BookingStatus { RESERVED, CHECKED_IN, CHECKED_OUT, CANCELLED };

// Chuyen BookingStatus sang chuoi hien thi tieng Viet khong dau
inline std::string bookingStatusToDisplay(BookingStatus status) {
  switch (status) {
  case BookingStatus::RESERVED:
    return "Da dat";
  case BookingStatus::CHECKED_IN:
    return "Dang o";
  case BookingStatus::CHECKED_OUT:
    return "Da tra phong";
  case BookingStatus::CANCELLED:
    return "Da huy";
  }
  throw std::invalid_argument("Trang thai booking khong hop le");
}

// Chuyen BookingStatus sang chuoi luu file
inline std::string bookingStatusToString(BookingStatus status) {
  switch (status) {
  case BookingStatus::RESERVED:
    return "RESERVED";
  case BookingStatus::CHECKED_IN:
    return "CHECKED_IN";
  case BookingStatus::CHECKED_OUT:
    return "CHECKED_OUT";
  case BookingStatus::CANCELLED:
    return "CANCELLED";
  }
  throw std::invalid_argument("Trang thai booking khong hop le");
}

// Chuyen chuoi luu file sang BookingStatus
inline BookingStatus stringToBookingStatus(const std::string &str) {
  if (str == "RESERVED")
    return BookingStatus::RESERVED;
  if (str == "CHECKED_IN")
    return BookingStatus::CHECKED_IN;
  if (str == "CHECKED_OUT")
    return BookingStatus::CHECKED_OUT;
  if (str == "CANCELLED")
    return BookingStatus::CANCELLED;
  throw std::invalid_argument("Chuoi trang thai booking khong hop le: " + str);
}

// Nhom dich vu khach san
enum class ServiceCategory { FOOD, LAUNDRY, OTHER };

// Chuyen ServiceCategory sang chuoi hien thi tieng Viet khong dau
inline std::string serviceCategoryToDisplay(ServiceCategory category) {
  switch (category) {
  case ServiceCategory::FOOD:
    return "An uong";
  case ServiceCategory::LAUNDRY:
    return "Giat ui";
  case ServiceCategory::OTHER:
    return "Khac";
  }
  throw std::invalid_argument("Loai dich vu khong hop le");
}

// Chuyen ServiceCategory sang chuoi luu file
inline std::string serviceCategoryToString(ServiceCategory category) {
  switch (category) {
  case ServiceCategory::FOOD:
    return "FOOD";
  case ServiceCategory::LAUNDRY:
    return "LAUNDRY";
  case ServiceCategory::OTHER:
    return "OTHER";
  }
  throw std::invalid_argument("Loai dich vu khong hop le");
}

// Chuyen chuoi luu file sang ServiceCategory
inline ServiceCategory stringToServiceCategory(const std::string &str) {
  if (str == "FOOD")
    return ServiceCategory::FOOD;
  if (str == "LAUNDRY")
    return ServiceCategory::LAUNDRY;
  if (str == "OTHER")
    return ServiceCategory::OTHER;
  throw std::invalid_argument("Chuoi loai dich vu khong hop le: " + str);
}

#endif // ENUMS_H
