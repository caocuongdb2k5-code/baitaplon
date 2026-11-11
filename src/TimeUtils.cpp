#include "TimeUtils.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int year, int month) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return 0;
    }
}

struct DateTimeParts {
    int year;
    int month;
    int day;
    int hour;
    int minute;
};

DateTimeParts parseParts(const std::string& timeStr) {
    // Dinh dang bat buoc: "YYYY-MM-DD HH:MM" (16 ky tu)
    if (timeStr.size() != 16 ||
        timeStr[4] != '-' || timeStr[7] != '-' ||
        timeStr[10] != ' ' || timeStr[13] != ':') {
        throw std::invalid_argument("Thoi gian phai theo dinh dang YYYY-MM-DD HH:MM: " + timeStr);
    }

    const int digitIndices[] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15};
    for (int idx : digitIndices) {
        if (!std::isdigit(static_cast<unsigned char>(timeStr[idx]))) {
            throw std::invalid_argument("Thoi gian chua ky tu khong hop le: " + timeStr);
        }
    }

    DateTimeParts parts{};
    parts.year = std::stoi(timeStr.substr(0, 4));
    parts.month = std::stoi(timeStr.substr(5, 2));
    parts.day = std::stoi(timeStr.substr(8, 2));
    parts.hour = std::stoi(timeStr.substr(11, 2));
    parts.minute = std::stoi(timeStr.substr(14, 2));

    if (parts.year < 1970 || parts.month < 1 || parts.month > 12) {
        throw std::invalid_argument("Ngay thang nam khong hop le: " + timeStr);
    }
    int maxDay = daysInMonth(parts.year, parts.month);
    if (parts.day < 1 || parts.day > maxDay) {
        throw std::invalid_argument("Ngay trong thang khong hop le: " + timeStr);
    }
    if (parts.hour < 0 || parts.hour > 23 || parts.minute < 0 || parts.minute > 59) {
        throw std::invalid_argument("Gio phut khong hop le: " + timeStr);
    }

    return parts;
}

} // namespace

namespace TimeUtils {

std::string getCurrentTimeString() {
    return formatTime(std::time(nullptr));
}

bool isValidTimeString(const std::string& timeStr) {
    try {
        (void)parseParts(timeStr);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

std::time_t parseTime(const std::string& timeStr) {
    DateTimeParts parts = parseParts(timeStr);
    std::tm tmVal{};
    tmVal.tm_year = parts.year - 1900;
    tmVal.tm_mon = parts.month - 1;
    tmVal.tm_mday = parts.day;
    tmVal.tm_hour = parts.hour;
    tmVal.tm_min = parts.minute;
    tmVal.tm_sec = 0;
    tmVal.tm_isdst = -1;

    std::time_t result = std::mktime(&tmVal);
    if (result == static_cast<std::time_t>(-1)) {
        throw std::invalid_argument("Khong the chuyen doi thoi gian sang time_t: " + timeStr);
    }
    return result;
}

std::string formatTime(std::time_t t) {
    std::tm* tmPtr = std::localtime(&t);
    if (tmPtr == nullptr) {
        throw std::runtime_error("Khong the dinh dang gia tri time_t");
    }
    std::ostringstream oss;
    oss << std::put_time(tmPtr, "%Y-%m-%d %H:%M");
    return oss.str();
}

int getMonth(const std::string& timeStr) {
    return parseParts(timeStr).month;
}

int getYear(const std::string& timeStr) {
    return parseParts(timeStr).year;
}

int getQuarter(const std::string& timeStr) {
    int month = getMonth(timeStr);
    return (month - 1) / 3 + 1;
}

int calculateRentalDays(const std::string& checkInStr, const std::string& checkOutStr) {
    std::time_t inTime = parseTime(checkInStr);
    std::time_t outTime = parseTime(checkOutStr);

    double diffSeconds = std::difftime(outTime, inTime);
    if (diffSeconds < 0.0) {
        throw std::invalid_argument("Thoi gian check-out khong duoc truoc thoi gian check-in");
    }

    int days = static_cast<int>(std::ceil(diffSeconds / 86400.0));
    return std::max(1, days);
}

} // namespace TimeUtils

