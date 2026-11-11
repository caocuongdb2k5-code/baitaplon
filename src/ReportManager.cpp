#include "ReportManager.h"
#include "Booking.h"
#include "Enums.h"
#include "Invoice.h"
#include "Room.h"
#include "TimeUtils.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {

struct DateYMD {
    int year;
    int month;
    int day;
};

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInCalendarMonth(int year, int month) {
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

DateYMD extractDate(const std::string& timeStr) {
    // Kiem tra tinh hop le cua chuoi thoi gian truoc khi cat ngay
    TimeUtils::parseTime(timeStr);
    return DateYMD{
        std::stoi(timeStr.substr(0, 4)),
        std::stoi(timeStr.substr(5, 2)),
        std::stoi(timeStr.substr(8, 2))
    };
}

int compareDate(const DateYMD& a, const DateYMD& b) {
    if (a.year != b.year) return a.year < b.year ? -1 : 1;
    if (a.month != b.month) return a.month < b.month ? -1 : 1;
    if (a.day != b.day) return a.day < b.day ? -1 : 1;
    return 0;
}

DateYMD nextDate(DateYMD d) {
    ++d.day;
    if (d.day > daysInCalendarMonth(d.year, d.month)) {
        d.day = 1;
        ++d.month;
        if (d.month > 12) {
            d.month = 1;
            ++d.year;
        }
    }
    return d;
}

} // namespace

ReportManager::ReportManager(const RoomManager& roomManager,
                             const BookingManager& bookingManager,
                             const InvoiceManager& invoiceManager,
                             const CustomerManager& customerManager)
    : roomManager(roomManager),
      bookingManager(bookingManager),
      invoiceManager(invoiceManager),
      customerManager(customerManager) {
}

void ReportManager::validateYear(int year) {
    if (year < 1970 || year > 9999) {
        throw std::invalid_argument("Nam thong ke khong hop le (phai tu 1970 den 9999)");
    }
}

int ReportManager::getDaysInMonth(int year, int month) {
    return daysInCalendarMonth(year, month);
}

std::map<int, double> ReportManager::getMonthlyRevenue(int year) const {
    validateYear(year);
    std::map<int, double> revenue;
    for (int m = 1; m <= 12; ++m) {
        revenue[m] = 0.0;
    }

    for (const Invoice& inv : invoiceManager.getAll()) {
        int invYear = TimeUtils::getYear(inv.getCheckOut());
        if (invYear == year) {
            int invMonth = TimeUtils::getMonth(inv.getCheckOut());
            revenue[invMonth] += inv.getTotal();
        }
    }
    return revenue;
}

std::map<int, MonthlyGuestStats> ReportManager::getMonthlyGuestStats(int year) const {
    validateYear(year);
    std::map<int, MonthlyGuestStats> stats;
    std::map<int, std::set<std::string>> cccdByMonth;
    for (int m = 1; m <= 12; ++m) {
        stats[m] = MonthlyGuestStats{};
    }

    for (const Booking& booking : bookingManager.getAll()) {
        BookingStatus status = booking.getStatus();
        if (status != BookingStatus::CHECKED_IN && status != BookingStatus::CHECKED_OUT) {
            continue;
        }
        if (booking.getActualCheckIn() == "-") {
            continue;
        }

        int inYear = TimeUtils::getYear(booking.getActualCheckIn());
        if (inYear == year) {
            int inMonth = TimeUtils::getMonth(booking.getActualCheckIn());
            ++stats[inMonth].checkedInBookings;
            if (customerManager.findByCccd(booking.getCccd()) != nullptr || !booking.getCccd().empty()) {
                cccdByMonth[inMonth].insert(booking.getCccd());
            }
        }
    }

    for (int m = 1; m <= 12; ++m) {
        stats[m].distinctCustomers = static_cast<int>(cccdByMonth[m].size());
    }
    return stats;
}

std::map<int, long long> ReportManager::getMonthlyRoomNights(int year) const {
    validateYear(year);
    std::map<int, long long> nights;
    for (int m = 1; m <= 12; ++m) {
        nights[m] = 0;
    }

    for (const Booking& booking : bookingManager.getAll()) {
        BookingStatus status = booking.getStatus();
        if (status != BookingStatus::CHECKED_IN && status != BookingStatus::CHECKED_OUT) {
            continue;
        }
        if (booking.getActualCheckIn() == "-") {
            continue;
        }

        std::string endTimeStr = (status == BookingStatus::CHECKED_OUT && booking.getActualCheckOut() != "-")
                                     ? booking.getActualCheckOut()
                                     : booking.getExpectedCheckOut();

        DateYMD startDate = extractDate(booking.getActualCheckIn());
        DateYMD endDate = extractDate(endTimeStr);

        // Neu check-in va check-out cung ngay thi tinh 1 phong-dem tai ngay do
        if (compareDate(endDate, startDate) <= 0) {
            if (startDate.year == year) {
                ++nights[startDate.month];
            }
            continue;
        }

        for (DateYMD d = startDate; compareDate(d, endDate) < 0; d = nextDate(d)) {
            if (d.year == year) {
                ++nights[d.month];
            }
        }
    }

    return nights;
}

std::map<int, double> ReportManager::getMonthlyOccupancyRate(int year) const {
    validateYear(year);
    std::map<int, long long> nights = getMonthlyRoomNights(year);
    std::map<int, double> rates;
    int totalRooms = static_cast<int>(roomManager.getAll().size());

    for (int m = 1; m <= 12; ++m) {
        if (totalRooms == 0) {
            rates[m] = 0.0;
        } else {
            double capacity = static_cast<double>(totalRooms) * static_cast<double>(getDaysInMonth(year, m));
            rates[m] = (static_cast<double>(nights[m]) / capacity) * 100.0;
        }
    }
    return rates;
}

std::map<int, double> ReportManager::getMonthlyVacancyRate(int year) const {
    std::map<int, double> occupancy = getMonthlyOccupancyRate(year);
    std::map<int, double> vacancy;
    for (int m = 1; m <= 12; ++m) {
        vacancy[m] = 100.0 - occupancy[m];
    }
    return vacancy;
}

CurrentAvailabilityStats ReportManager::getCurrentAvailability() const {
    CurrentAvailabilityStats result;
    const auto& allRooms = roomManager.getAll();
    result.totalRooms = static_cast<int>(allRooms.size());
    for (const auto& room : allRooms) {
        if (room->getStatus() == RoomStatus::AVAILABLE) {
            ++result.availableRooms;
        }
    }
    result.availablePercent = (result.totalRooms == 0)
                                  ? 0.0
                                  : (static_cast<double>(result.availableRooms) * 100.0 /
                                     static_cast<double>(result.totalRooms));
    return result;
}

std::map<int, int> ReportManager::getMonthlyBookingCounts(int year) const {
    validateYear(year);
    std::map<int, int> counts;
    for (int m = 1; m <= 12; ++m) {
        counts[m] = 0;
    }

    for (const Booking& booking : bookingManager.getAll()) {
        if (booking.getStatus() == BookingStatus::CANCELLED) {
            continue;
        }
        int bookYear = TimeUtils::getYear(booking.getExpectedCheckIn());
        if (bookYear == year) {
            int bookMonth = TimeUtils::getMonth(booking.getExpectedCheckIn());
            ++counts[bookMonth];
        }
    }
    return counts;
}

std::vector<MonthlySummaryRow> ReportManager::getAnnualSummary(int year) const {
    validateYear(year);
    auto revenue = getMonthlyRevenue(year);
    auto guests = getMonthlyGuestStats(year);
    auto nights = getMonthlyRoomNights(year);
    auto occupancy = getMonthlyOccupancyRate(year);
    auto vacancy = getMonthlyVacancyRate(year);
    auto bookingCounts = getMonthlyBookingCounts(year);

    std::vector<MonthlySummaryRow> rows;
    rows.reserve(12);
    for (int m = 1; m <= 12; ++m) {
        MonthlySummaryRow row;
        row.month = m;
        row.revenue = revenue[m];
        row.checkedInBookings = guests[m].checkedInBookings;
        row.distinctCustomers = guests[m].distinctCustomers;
        row.roomNights = nights[m];
        row.occupancyPercent = occupancy[m];
        row.vacancyPercent = vacancy[m];
        row.bookingCount = bookingCounts[m];
        rows.push_back(row);
    }
    return rows;
}

void ReportManager::printRevenueReport(int year, std::ostream& os) const {
    auto revenue = getMonthlyRevenue(year);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    oss << "--- 1. DOANH THU THEO THANG NAM " << year << " (theo ngay check-out) ---\n";
    double totalRevenue = 0.0;
    for (int m = 1; m <= 12; ++m) {
        totalRevenue += revenue[m];
        oss << "  Thang " << std::setw(2) << m << ": "
            << std::setw(14) << std::right << revenue[m] << " VND\n";
    }
    oss << "  TONG CONG : " << std::setw(12) << std::right << totalRevenue << " VND\n";
    os << oss.str();
}

void ReportManager::printGuestReport(int year, std::ostream& os) const {
    auto guests = getMonthlyGuestStats(year);
    std::ostringstream oss;
    oss << "--- 2. SO KHACH THUE THEO THANG NAM " << year << " ---\n";
    oss << "  (Dinh nghia: Luot check-in = tong booking da check-in trong thang;\n"
        << "               Khach khac nhau = so CCCD khac nhau check-in trong thang)\n";
    for (int m = 1; m <= 12; ++m) {
        oss << "  Thang " << std::setw(2) << m
            << " | Luot check-in: " << std::setw(4) << guests[m].checkedInBookings
            << " | Khach khac nhau (CCCD): " << std::setw(4) << guests[m].distinctCustomers << '\n';
    }
    os << oss.str();
}

void ReportManager::printOccupancyReport(int year, std::ostream& os) const {
    auto nights = getMonthlyRoomNights(year);
    auto occupancy = getMonthlyOccupancyRate(year);
    auto vacancy = getMonthlyVacancyRate(year);
    CurrentAvailabilityStats current = getCurrentAvailability();
    int totalRooms = static_cast<int>(roomManager.getAll().size());

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "--- 3 & 4. TY LE LAP DAY VA PHONG TRONG NAM " << year << " ---\n";
    oss << "  Tong so phong hien co: " << totalRooms << '\n';
    for (int m = 1; m <= 12; ++m) {
        long long capacity = static_cast<long long>(totalRooms) * getDaysInMonth(year, m);
        oss << "  Thang " << std::setw(2) << m
            << " (" << std::setw(2) << getDaysInMonth(year, m) << " ngay)"
            << " | Phong-dem: " << std::setw(4) << nights[m] << "/" << std::setw(4) << capacity
            << " | Lap day: " << std::setw(6) << occupancy[m] << "%"
            << " | Trong: " << std::setw(6) << vacancy[m] << "%\n";
    }
    oss << "  Ty le phong trong HIEN TAI (theo trang thai AVAILABLE): "
        << current.availableRooms << "/" << current.totalRooms
        << " (" << current.availablePercent << "%)\n";
    os << oss.str();
}

void ReportManager::printBookingChart(int year, std::ostream& os) const {
    auto counts = getMonthlyBookingCounts(year);
    int maxCount = 0;
    for (int m = 1; m <= 12; ++m) {
        maxCount = std::max(maxCount, counts[m]);
    }

    const int maxBarWidth = 30;
    int displayWidth = std::max(10, std::min(maxCount, maxBarWidth));

    std::ostringstream oss;
    oss << "--- 5. SO SANH SO LUOT DAT PHONG QUA CAC THANG NAM " << year
        << " (khong tinh CANCELLED) ---\n";
    for (int m = 1; m <= 12; ++m) {
        int count = counts[m];
        int barLen = 0;
        if (count > 0) {
            if (maxCount <= maxBarWidth) {
                barLen = count;
            } else {
                barLen = static_cast<int>(
                    std::round(static_cast<double>(count) * maxBarWidth / static_cast<double>(maxCount))
                );
                barLen = std::max(1, barLen);
            }
        }
        std::string bar(static_cast<std::size_t>(barLen), '#');
        oss << "  Thang " << std::setw(2) << m << " | "
            << std::left << std::setw(displayWidth + 2) << bar
            << " (" << count << " luot)\n";
    }
    os << oss.str();
}

void ReportManager::printSummaryTable(int year, std::ostream& os) const {
    auto rows = getAnnualSummary(year);
    std::ostringstream oss;
    oss << "--- 6. BANG TONG HOP 12 THANG NAM " << year << " ---\n";
    oss << "================================================================================\n";
    oss << std::left << std::setw(8) << "Thang"
        << std::right << std::setw(16) << "Doanh thu (VND)"
        << std::right << std::setw(18) << "Khach (Luot/CCCD)"
        << std::right << std::setw(13) << "Lap day (%)"
        << std::right << std::setw(11) << "Trong (%)"
        << std::right << std::setw(12) << "Luot dat" << '\n';
    oss << "--------------------------------------------------------------------------------\n";

    double totalRevenue = 0.0;
    int totalCheckIns = 0;
    int totalBookings = 0;

    for (const auto& row : rows) {
        totalRevenue += row.revenue;
        totalCheckIns += row.checkedInBookings;
        totalBookings += row.bookingCount;

        std::ostringstream guestCol;
        guestCol << row.checkedInBookings << "/" << row.distinctCustomers;

        oss << std::left << std::setw(8) << ("T" + std::to_string(row.month))
            << std::right << std::setw(16) << std::fixed << std::setprecision(0) << row.revenue
            << std::right << std::setw(18) << guestCol.str()
            << std::right << std::setw(13) << std::fixed << std::setprecision(2) << row.occupancyPercent
            << std::right << std::setw(11) << std::fixed << std::setprecision(2) << row.vacancyPercent
            << std::right << std::setw(12) << row.bookingCount << '\n';
    }
    oss << "================================================================================\n";
    oss << "  Tong doanh thu nam " << year << ": " << std::fixed << std::setprecision(0)
        << totalRevenue << " VND | Tong check-in: " << totalCheckIns
        << " | Tong luot dat: " << totalBookings << '\n';
    os << oss.str();
}

void ReportManager::printFullReport(int year, std::ostream& os) const {
    validateYear(year);
    os << "================================================================================\n";
    os << "                    BAO CAO THONG KE KHACH SAN NAM " << year << '\n';
    os << "================================================================================\n\n";
    printSummaryTable(year, os);
    os << '\n';
    printBookingChart(year, os);
    os << '\n';
    printOccupancyReport(year, os);
    os << '\n';
    printGuestReport(year, os);
    os << '\n';
    printRevenueReport(year, os);
}

std::string ReportManager::exportReportToFile(int year, const std::string& reportsDir) const {
    validateYear(year);
    std::filesystem::path dirPath(reportsDir);
    if (!dirPath.empty()) {
        std::filesystem::create_directories(dirPath);
    }

    std::filesystem::path filePath = dirPath / ("baocao_" + std::to_string(year) + ".txt");
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Khong the tao file bao cao: " + filePath.string());
    }

    printFullReport(year, outFile);
    outFile.flush();
    if (!outFile.good()) {
        throw std::runtime_error("Loi khi ghi file bao cao: " + filePath.string());
    }
    return filePath.string();
}

