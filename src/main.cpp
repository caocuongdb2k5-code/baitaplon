#include "BookingManager.h"
#include "CustomerManager.h"
#include "InvoiceManager.h"
#include "Menu.h"
#include "PromotionManager.h"
#include "ReportManager.h"
#include "RoomManager.h"
#include "ServiceManager.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    const std::string roomsPath = "data/rooms.json";
    const std::string customersPath = "data/customers.json";
    const std::string bookingsPath = "data/bookings.json";
    const std::string servicesPath = "data/services.json";
    const std::string usagesPath = "data/service_usage.json";
    const std::string promotionsPath = "data/promotions.json";
    const std::string invoicesPath = "data/invoices.json";

    try {
        RoomManager roomManager;
        CustomerManager customerManager;
        BookingManager bookingManager(roomManager, customerManager, roomsPath, bookingsPath);
        ServiceManager serviceManager(bookingManager, servicesPath, usagesPath);
        PromotionManager promotionManager(promotionsPath);
        InvoiceManager invoiceManager(
            bookingManager, roomManager, customerManager, serviceManager, promotionManager, invoicesPath, "invoices"
        );
        ReportManager reportManager(roomManager, bookingManager, invoiceManager, customerManager);

        Menu menu(
            roomManager,
            customerManager,
            bookingManager,
            serviceManager,
            promotionManager,
            invoiceManager,
            reportManager,
            roomsPath,
            customersPath,
            bookingsPath,
            servicesPath,
            usagesPath,
            promotionsPath,
            invoicesPath
        );
        menu.run();
    } catch (const std::exception& ex) {
        std::cerr << "Loi nghiem trong: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
