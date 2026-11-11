#ifndef MENU_H
#define MENU_H

#include "BookingManager.h"
#include "CustomerManager.h"
#include "Enums.h"
#include "IChatEngine.h"
#include "InvoiceManager.h"
#include "PromotionManager.h"
#include "ReportManager.h"
#include "RoomManager.h"
#include "ServiceManager.h"
#include <memory>
#include <string>

// Tang giao dien console: chi nhap, xuat, goi Manager va boc try/catch
class Menu {
private:
    RoomManager& roomManager;
    CustomerManager& customerManager;
    BookingManager& bookingManager;
    ServiceManager& serviceManager;
    PromotionManager& promotionManager;
    InvoiceManager& invoiceManager;
    ReportManager& reportManager;
    std::unique_ptr<IChatEngine> chatEngine;

    std::string roomsPath;
    std::string customersPath;
    std::string bookingsPath;
    std::string servicesPath;
    std::string usagesPath;
    std::string promotionsPath;
    std::string invoicesPath;

    void loadAllData();
    void saveAllData() const;

    // Cac menu con
    void handleRoomMenu();
    void handleCustomerMenu();
    void handleBookingMenu();
    void handleServiceMenu();
    void handleInvoiceMenu();
    void handleReportMenu();
    void handlePromotionMenu();
    void handleChatboxMenu();
    static void appendChatLog(const std::string& question, const std::string& answer);

    // Ham ho tro nhap lua chon
    static std::string selectRoomType(bool allowAll = false);
    static RoomStatus selectRoomStatus();
    static ServiceCategory selectServiceCategory();

public:
    Menu(RoomManager& roomManager,
         CustomerManager& customerManager,
         BookingManager& bookingManager,
         ServiceManager& serviceManager,
         PromotionManager& promotionManager,
         InvoiceManager& invoiceManager,
         ReportManager& reportManager,
         const std::string& roomsPath = "data/rooms.json",
         const std::string& customersPath = "data/customers.json",
         const std::string& bookingsPath = "data/bookings.json",
         const std::string& servicesPath = "data/services.json",
         const std::string& usagesPath = "data/service_usage.json",
         const std::string& promotionsPath = "data/promotions.json",
         const std::string& invoicesPath = "data/invoices.json");

    void run();
};

#endif // MENU_H

