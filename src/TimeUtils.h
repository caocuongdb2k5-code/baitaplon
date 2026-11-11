#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <ctime>
#include <string>

// Tien ich xu ly chuoi thoi gian dang "YYYY-MM-DD HH:MM"
namespace TimeUtils {

// Lay thoi gian hien tai dang "YYYY-MM-DD HH:MM"
std::string getCurrentTimeString();

// Kiem tra chuoi co dung dinh dang "YYYY-MM-DD HH:MM" va ngay gio hop le hay
// khong
bool isValidTimeString(const std::string &timeStr);

// Chuyen chuoi "YYYY-MM-DD HH:MM" sang time_t (nem std::invalid_argument neu
// sai)
std::time_t parseTime(const std::string &timeStr);

// Chuyen time_t sang chuoi "YYYY-MM-DD HH:MM"
std::string formatTime(std::time_t t);

// Lay thang (1..12), nam, quy (1..4) tu chuoi thoi gian
int getMonth(const std::string &timeStr);
int getYear(const std::string &timeStr);
int getQuarter(const std::string &timeStr);

// Tinh so ngay thue: max(1, ceil((out - in) / 86400.0))
int calculateRentalDays(const std::string &checkInStr,
                        const std::string &checkOutStr);

} // namespace TimeUtils

#endif // TIME_UTILS_H
