#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include "BookingManager.h"
#include "CustomerManager.h"
#include "InvoiceManager.h"
#include "RoomManager.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Thong ke luot khach check-in va so khach khac nhau trong thang
struct MonthlyGuestStats {
    int checkedInBookings = 0;
    int distinctCustomers = 0;
};

// Dong du lieu tong hop theo tung thang trong nam
struct MonthlySummaryRow {
    int month = 0;
    double revenue = 0.0;
    int checkedInBookings = 0;
    int distinctCustomers = 0;
    long long roomNights = 0;
    double occupancyPercent = 0.0;
    double vacancyPercent = 0.0;
    int bookingCount = 0;
};

// Tom tat trang thai phong trong hien tai
struct CurrentAvailabilityStats {
    int availableRooms = 0;
    int totalRooms = 0;
    double availablePercent = 0.0;
};

// Lop thong ke va xuat bao cao theo thang cua mot nam
class ReportManager {
private:
    const RoomManager& roomManager;
    const BookingManager& bookingManager;
    const InvoiceManager& invoiceManager;
    const CustomerManager& customerManager;

    static void validateYear(int year);
    static int getDaysInMonth(int year, int month);

public:
    ReportManager(const RoomManager& roomManager,
                  const BookingManager& bookingManager,
                  const InvoiceManager& invoiceManager,
                  const CustomerManager& customerManager);

    // Cac ham tinh toan tra du lieu (khong in truc tiep)
    std::map<int, double> getMonthlyRevenue(int year) const;
    std::map<int, MonthlyGuestStats> getMonthlyGuestStats(int year) const;
    std::map<int, long long> getMonthlyRoomNights(int year) const;
    std::map<int, double> getMonthlyOccupancyRate(int year) const;
    std::map<int, double> getMonthlyVacancyRate(int year) const;
    CurrentAvailabilityStats getCurrentAvailability() const;
    std::map<int, int> getMonthlyBookingCounts(int year) const;
    std::vector<MonthlySummaryRow> getAnnualSummary(int year) const;

    // Cac ham in bao cao ra stream
    void printRevenueReport(int year, std::ostream& os) const;
    void printGuestReport(int year, std::ostream& os) const;
    void printOccupancyReport(int year, std::ostream& os) const;
    void printBookingChart(int year, std::ostream& os) const;
    void printSummaryTable(int year, std::ostream& os) const;
    void printFullReport(int year, std::ostream& os) const;

    // Xuat bao cao day du ra reports/baocao_<nam>.txt
    std::string exportReportToFile(int year, const std::string& reportsDir = "reports") const;
};

#endif // REPORT_MANAGER_H

