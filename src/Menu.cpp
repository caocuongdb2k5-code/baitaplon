#include "Menu.h"
#include "Booking.h"
#include "Customer.h"
#include "InputUtils.h"
#include "Invoice.h"
#include "KeywordChatEngine.h"
#include "Promotion.h"
#include "Room.h"
#include "Service.h"
#include "ServiceUsage.h"
#include "TimeUtils.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace {

std::string fitColumn(const std::string& text, std::size_t maxWidth) {
    if (text.size() <= maxWidth) {
        return text;
    }
    if (maxWidth <= 3) {
        return text.substr(0, maxWidth);
    }
    return text.substr(0, maxWidth - 3) + "...";
}

void printHeader(const std::string& title) {
    std::cout << "\n================================================================================\n";
    std::cout << "  " << title << '\n';
    std::cout << "================================================================================\n";
}

void printRoomsTable(const std::vector<const Room*>& rooms, const std::string& title) {
    printHeader(title);
    if (rooms.empty()) {
        std::cout << "  (Khong co phong nao)\n";
        return;
    }
    std::cout << std::left << std::setw(8) << "Ma"
              << std::setw(9) << "Loai"
              << std::right << std::setw(14) << "Gia/ngay"
              << "  " << std::left << std::setw(13) << "Trang thai"
              << "Tien ich\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    for (const Room* r : rooms) {
        oss.str("");
        oss.clear();
        oss << r->getPricePerDay() << " VND";
        std::cout << std::left << std::setw(8) << fitColumn(r->getId(), 7)
                  << std::setw(9) << r->getType()
                  << std::right << std::setw(14) << oss.str()
                  << "  " << std::left << std::setw(13) << roomStatusToDisplay(r->getStatus())
                  << fitColumn(r->getAmenities(), 34) << '\n';
    }
}

void printCustomersTable(const std::vector<const Customer*>& customers, const std::string& title) {
    printHeader(title);
    if (customers.empty()) {
        std::cout << "  (Khong co khach hang nao)\n";
        return;
    }
    std::cout << std::left << std::setw(15) << "CCCD"
              << std::setw(32) << "Ho ten"
              << std::setw(14) << "So dien thoai" << '\n';
    std::cout << "--------------------------------------------------------------------------------\n";
    for (const Customer* c : customers) {
        std::cout << std::left << std::setw(15) << c->getCccd()
                  << std::setw(32) << fitColumn(c->getName(), 30)
                  << std::setw(14) << c->getPhone() << '\n';
    }
}

void printBookingsTable(const std::vector<const Booking*>& bookings, const std::string& title) {
    printHeader(title);
    if (bookings.empty()) {
        std::cout << "  (Khong co booking nao)\n";
        return;
    }
    std::cout << std::left << std::setw(8) << "Ma BK"
              << std::setw(14) << "CCCD"
              << std::setw(7) << "Phong"
              << std::setw(18) << "Check-in (TT/DK)"
              << std::setw(18) << "Check-out (TT/DK)"
              << "Trang thai\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    for (const Booking* b : bookings) {
        const std::string& inStr = (b->getActualCheckIn() != "-")
                                       ? b->getActualCheckIn()
                                       : b->getExpectedCheckIn();
        const std::string& outStr = (b->getActualCheckOut() != "-")
                                        ? b->getActualCheckOut()
                                        : b->getExpectedCheckOut();
        std::cout << std::left << std::setw(8) << fitColumn(b->getBookingId(), 7)
                  << std::setw(14) << b->getCccd()
                  << std::setw(7) << fitColumn(b->getRoomId(), 6)
                  << std::setw(18) << inStr
                  << std::setw(18) << outStr
                  << bookingStatusToDisplay(b->getStatus()) << '\n';
    }
}

void printServicesTable(const std::vector<Service>& services, const std::string& title) {
    printHeader(title);
    if (services.empty()) {
        std::cout << "  (Khong co dich vu nao)\n";
        return;
    }
    std::cout << std::left << std::setw(8) << "Ma DV"
              << std::setw(28) << "Ten dich vu"
              << std::setw(12) << "Nhom"
              << std::right << std::setw(15) << "Don gia" << '\n';
    std::cout << "--------------------------------------------------------------------------------\n";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    for (const auto& s : services) {
        oss.str("");
        oss.clear();
        oss << s.getUnitPrice() << " VND";
        std::cout << std::left << std::setw(8) << fitColumn(s.getServiceId(), 7)
                  << std::setw(28) << fitColumn(s.getName(), 26)
                  << std::setw(12) << serviceCategoryToDisplay(s.getCategory())
                  << std::right << std::setw(15) << oss.str() << '\n';
    }
}

void printInvoicesTable(const std::vector<Invoice>& invoices, const std::string& title) {
    printHeader(title);
    if (invoices.empty()) {
        std::cout << "  (Chua co hoa don nao)\n";
        return;
    }
    std::cout << std::left << std::setw(9) << "Ma HD"
              << std::setw(8) << "Ma BK"
              << std::setw(22) << "Khach hang"
              << std::setw(7) << "Phong"
              << std::right << std::setw(5) << "Ngay"
              << std::right << std::setw(15) << "Tong tien"
              << "  Ngay xuat\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    for (const auto& inv : invoices) {
        oss.str("");
        oss.clear();
        oss << inv.getTotal() << " VND";
        std::cout << std::left << std::setw(9) << fitColumn(inv.getInvoiceId(), 8)
                  << std::setw(8) << fitColumn(inv.getBookingId(), 7)
                  << std::setw(22) << fitColumn(inv.getCustomerName(), 20)
                  << std::setw(7) << fitColumn(inv.getRoomId(), 6)
                  << std::right << std::setw(5) << inv.getDays()
                  << std::right << std::setw(15) << oss.str()
                  << "  " << inv.getIssuedAt() << '\n';
    }
}

void printPromotionsTable(const std::vector<Promotion>& promotions, const std::string& title) {
    printHeader(title);
    if (promotions.empty()) {
        std::cout << "  (Chua co chuong trinh khuyen mai nao)\n";
        return;
    }
    std::cout << std::left << std::setw(8) << "Quy"
              << std::setw(12) << "Loai phong"
              << std::right << std::setw(10) << "Giam (%)"
              << "  Mo ta\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    for (const auto& p : promotions) {
        std::cout << std::left << std::setw(8) << ("Quy " + std::to_string(p.getQuarter()))
                  << std::setw(12) << p.getRoomType()
                  << std::right << std::setw(9) << std::fixed << std::setprecision(1) << p.getDiscountPercent() << "%"
                  << "  " << fitColumn(p.getDescription(), 45) << '\n';
    }
}

} // namespace

Menu::Menu(RoomManager& roomManager,
           CustomerManager& customerManager,
           BookingManager& bookingManager,
           ServiceManager& serviceManager,
           PromotionManager& promotionManager,
           InvoiceManager& invoiceManager,
           ReportManager& reportManager,
           const std::string& roomsPath,
           const std::string& customersPath,
           const std::string& bookingsPath,
           const std::string& servicesPath,
           const std::string& usagesPath,
           const std::string& promotionsPath,
           const std::string& invoicesPath)
    : roomManager(roomManager),
      customerManager(customerManager),
      bookingManager(bookingManager),
      serviceManager(serviceManager),
      promotionManager(promotionManager),
      invoiceManager(invoiceManager),
      reportManager(reportManager),
      chatEngine(std::make_unique<KeywordChatEngine>(
          roomManager,
          customerManager,
          bookingManager,
          serviceManager,
          promotionManager,
          invoiceManager,
          reportManager)),
      roomsPath(roomsPath),
      customersPath(customersPath),
      bookingsPath(bookingsPath),
      servicesPath(servicesPath),
      usagesPath(usagesPath),
      promotionsPath(promotionsPath),
      invoicesPath(invoicesPath) {
}

void Menu::loadAllData() {
    roomManager.loadFromFile(roomsPath);
    customerManager.loadFromFile(customersPath);
    bookingManager.loadFromFile(bookingsPath);
    serviceManager.loadServicesFromFile(servicesPath);
    serviceManager.loadUsagesFromFile(usagesPath);
    promotionManager.loadFromFile(promotionsPath);
    invoiceManager.loadFromFile(invoicesPath);
}

void Menu::saveAllData() const {
    roomManager.saveToFile(roomsPath);
    customerManager.saveToFile(customersPath);
    bookingManager.saveToFile(bookingsPath);
    serviceManager.saveServicesToFile(servicesPath);
    serviceManager.saveUsagesToFile(usagesPath);
    promotionManager.saveToFile(promotionsPath);
    invoiceManager.saveToFile(invoicesPath);
}

std::string Menu::selectRoomType(bool allowAll) {
    std::cout << "  Chon loai phong: 1. SINGLE | 2. DOUBLE | 3. VIP";
    if (allowAll) {
        std::cout << " | 4. ALL";
    }
    std::cout << '\n';
    int choice = InputUtils::readInt("  Nhap lua chon: ", 1, allowAll ? 4 : 3);
    switch (choice) {
        case 1: return "SINGLE";
        case 2: return "DOUBLE";
        case 3: return "VIP";
        default: return "ALL";
    }
}

RoomStatus Menu::selectRoomStatus() {
    std::cout << "  Chon trang thai: 1. Trong | 2. Dang dat | 3. Dang thue | 4. Dang don\n";
    int choice = InputUtils::readInt("  Nhap lua chon: ", 1, 4);
    switch (choice) {
        case 1: return RoomStatus::AVAILABLE;
        case 2: return RoomStatus::BOOKED;
        case 3: return RoomStatus::OCCUPIED;
        default: return RoomStatus::CLEANING;
    }
}

ServiceCategory Menu::selectServiceCategory() {
    std::cout << "  Chon nhom dich vu: 1. An uong (FOOD) | 2. Giat ui (LAUNDRY) | 3. Khac (OTHER)\n";
    int choice = InputUtils::readInt("  Nhap lua chon: ", 1, 3);
    switch (choice) {
        case 1: return ServiceCategory::FOOD;
        case 2: return ServiceCategory::LAUNDRY;
        default: return ServiceCategory::OTHER;
    }
}

void Menu::run() {
    try {
        loadAllData();
    } catch (const std::exception& ex) {
        std::cout << "[CANH BAO] Loi khi nap du lieu khoi tao: " << ex.what() << '\n';
    }

    try {
        while (true) {
            printHeader("PHAN MEM QUAN LY KHACH SAN");
            std::cout << "  1. Quan ly phong\n";
            std::cout << "  2. Quan ly khach hang\n";
            std::cout << "  3. Dat phong / Check-in / Check-out / Huy dat phong\n";
            std::cout << "  4. Dich vu phat sinh\n";
            std::cout << "  5. Hoa don\n";
            std::cout << "  6. Bao cao thong ke\n";
            std::cout << "  7. Khuyen mai theo quy\n";
            std::cout << "  8. Chatbox\n";
            std::cout << "  0. Thoat\n";
            std::cout << "--------------------------------------------------------------------------------\n";

            int choice = InputUtils::readInt("Chon chuc nang (0-8): ", 0, 8);
            switch (choice) {
                case 1: handleRoomMenu(); break;
                case 2: handleCustomerMenu(); break;
                case 3: handleBookingMenu(); break;
                case 4: handleServiceMenu(); break;
                case 5: handleInvoiceMenu(); break;
                case 6: handleReportMenu(); break;
                case 7: handlePromotionMenu(); break;
                case 8: handleChatboxMenu(); break;
                case 0:
                    try {
                        saveAllData();
                        std::cout << "\nDa luu toan bo du lieu. Tam biet!\n";
                    } catch (const std::exception& ex) {
                        std::cout << "\n[LOI] Khong the luu du lieu khi thoat: " << ex.what() << '\n';
                    }
                    return;
            }
        }
    } catch (const std::exception& ex) {
        std::cout << "\nKet thuc chuong trinh: " << ex.what() << '\n';
        try {
            saveAllData();
        } catch (...) {
        }
    }
}

void Menu::handleRoomMenu() {
    while (true) {
        printHeader("QUAN LY PHONG");
        std::cout << "  1. Them phong moi\n";
        std::cout << "  2. Sua gia / loai phong\n";
        std::cout << "  3. Xoa phong\n";
        std::cout << "  4. Xem tat ca phong\n";
        std::cout << "  5. Loc phong theo loai\n";
        std::cout << "  6. Loc phong theo trang thai\n";
        std::cout << "  7. Danh dau don xong phong (CLEANING -> AVAILABLE)\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-7): ", 0, 7);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                std::string type = selectRoomType(false);
                std::string id = InputUtils::readNonEmptyString("  Nhap ma phong (vd P101): ");
                double basePrice = InputUtils::readDouble("  Nhap gia co ban (VND): ", 1.0, 1000000000.0);
                roomManager.addRoom(type, id, basePrice);
                roomManager.saveToFile(roomsPath);
                std::cout << "Da them phong thanh cong!\n";
            } else if (choice == 2) {
                std::string id = InputUtils::readNonEmptyString("  Nhap ma phong can sua: ");
                std::string newType = selectRoomType(false);
                double newPrice = InputUtils::readDouble("  Nhap gia co ban moi (VND): ", 1.0, 1000000000.0);
                roomManager.updateRoom(id, newPrice, newType);
                roomManager.saveToFile(roomsPath);
                std::cout << "Da cap nhat phong thanh cong!\n";
            } else if (choice == 3) {
                std::string id = InputUtils::readNonEmptyString("  Nhap ma phong can xoa: ");
                if (InputUtils::readYesNo("  Ban co chac chan muon xoa phong " + id + "? (y/n): ")) {
                    roomManager.removeRoom(id);
                    roomManager.saveToFile(roomsPath);
                    std::cout << "Da xoa phong thanh cong!\n";
                } else {
                    std::cout << "Da huy thao tac xoa.\n";
                }
            } else if (choice == 4) {
                std::vector<const Room*> list;
                for (const auto& r : roomManager.getAll()) {
                    list.push_back(r.get());
                }
                printRoomsTable(list, "DANH SACH TAT CA PHONG");
            } else if (choice == 5) {
                std::string type = selectRoomType(false);
                printRoomsTable(roomManager.filterByType(type), "DANH SACH PHONG LOAI " + type);
            } else if (choice == 6) {
                RoomStatus status = selectRoomStatus();
                printRoomsTable(
                    roomManager.filterByStatus(status),
                    "DANH SACH PHONG TRANG THAI: " + roomStatusToDisplay(status)
                );
            } else if (choice == 7) {
                printRoomsTable(
                    roomManager.filterByStatus(RoomStatus::CLEANING),
                    "PHONG DANG DON (CLEANING)"
                );
                std::string id = InputUtils::readNonEmptyString("  Nhap ma phong da don xong: ");
                bookingManager.finishCleaning(id);
                std::cout << "Phong " << id << " da san sang (AVAILABLE)!\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handleCustomerMenu() {
    while (true) {
        printHeader("QUAN LY KHACH HANG");
        std::cout << "  1. Them khach hang moi\n";
        std::cout << "  2. Sua thong tin khach hang\n";
        std::cout << "  3. Tra cuu theo CCCD\n";
        std::cout << "  4. Tra cuu theo SDT\n";
        std::cout << "  5. Tim kiem theo ten\n";
        std::cout << "  6. Xem tat ca khach hang\n";
        std::cout << "  7. Xem lich su luu tru cua khach\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-7): ", 0, 7);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                std::string cccd = InputUtils::readNonEmptyString("  Nhap CCCD (12 chu so): ");
                std::string name = InputUtils::readNonEmptyString("  Nhap ho ten: ");
                std::string phone = InputUtils::readNonEmptyString("  Nhap so dien thoai (10 chu so): ");
                customerManager.addCustomer(cccd, name, phone);
                customerManager.saveToFile(customersPath);
                std::cout << "Da them khach hang thanh cong!\n";
            } else if (choice == 2) {
                std::string cccd = InputUtils::readNonEmptyString("  Nhap CCCD khach can sua: ");
                std::string newName = InputUtils::readNonEmptyString("  Nhap ho ten moi: ");
                std::string newPhone = InputUtils::readNonEmptyString("  Nhap SDT moi (10 chu so): ");
                customerManager.updateCustomer(cccd, newName, newPhone);
                customerManager.saveToFile(customersPath);
                std::cout << "Da cap nhat khach hang thanh cong!\n";
            } else if (choice == 3) {
                std::string cccd = InputUtils::readNonEmptyString("  Nhap CCCD can tim: ");
                const Customer* c = customerManager.findByCccd(cccd);
                std::vector<const Customer*> list;
                if (c != nullptr) list.push_back(c);
                printCustomersTable(list, "KET QUA TRA CUU THEO CCCD");
            } else if (choice == 4) {
                std::string phone = InputUtils::readNonEmptyString("  Nhap SDT can tim: ");
                const Customer* c = customerManager.findByPhone(phone);
                std::vector<const Customer*> list;
                if (c != nullptr) list.push_back(c);
                printCustomersTable(list, "KET QUA TRA CUU THEO SDT");
            } else if (choice == 5) {
                std::string keyword = InputUtils::readNonEmptyString("  Nhap tu khoa ten: ");
                printCustomersTable(
                    customerManager.searchByName(keyword),
                    "KET QUA TIM KIEM TEN: " + keyword
                );
            } else if (choice == 6) {
                std::vector<const Customer*> list;
                for (const auto& c : customerManager.getAll()) {
                    list.push_back(&c);
                }
                printCustomersTable(list, "DANH SACH TAT CA KHACH HANG");
            } else if (choice == 7) {
                std::string cccd = InputUtils::readNonEmptyString("  Nhap CCCD khach hang: ");
                const Customer* c = customerManager.findByCccd(cccd);
                if (c == nullptr) {
                    throw std::runtime_error("Khong tim thay khach hang co CCCD: " + cccd);
                }
                std::cout << "  Khach hang: " << *c << '\n';
                printBookingsTable(
                    bookingManager.getByCustomer(cccd),
                    "LICH SU LUU TRU CUA KHACH " + c->getName()
                );
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handleBookingMenu() {
    while (true) {
        printHeader("DAT PHONG / CHECK-IN / CHECK-OUT / HUY");
        std::cout << "  1. Dat phong moi\n";
        std::cout << "  2. Check-in\n";
        std::cout << "  3. Check-out & Xuat hoa don\n";
        std::cout << "  4. Huy dat phong\n";
        std::cout << "  5. Xem booking dang hoat dong (RESERVED / CHECKED_IN)\n";
        std::cout << "  6. Xem tat ca booking\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-6): ", 0, 6);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                printRoomsTable(
                    roomManager.filterByStatus(RoomStatus::AVAILABLE),
                    "PHONG TRONG CO THE DAT"
                );
                std::string cccd = InputUtils::readNonEmptyString("  Nhap CCCD khach hang: ");
                std::string roomId = InputUtils::readNonEmptyString("  Nhap ma phong muon dat: ");
                std::string expIn = InputUtils::readDateTime("  Nhap gio du kien nhan (YYYY-MM-DD HH:MM): ");
                std::string expOut = InputUtils::readDateTime("  Nhap gio du kien tra (YYYY-MM-DD HH:MM): ");

                const Booking& created = bookingManager.createBooking(cccd, roomId, expIn, expOut);
                std::cout << "\nDat phong thanh cong! Ma booking: " << created.getBookingId() << '\n';
                std::cout << "  " << created << '\n';
            } else if (choice == 2) {
                std::vector<const Booking*> reserved;
                for (const Booking* b : bookingManager.getActiveBookings()) {
                    if (b->getStatus() == BookingStatus::RESERVED) {
                        reserved.push_back(b);
                    }
                }
                printBookingsTable(reserved, "BOOKING CHO CHECK-IN (RESERVED)");
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking can check-in: ");
                bookingManager.checkIn(bookingId);
                std::cout << "Check-in thanh cong cho booking " << bookingId << "!\n";
            } else if (choice == 3) {
                std::vector<const Booking*> occupied;
                for (const Booking* b : bookingManager.getActiveBookings()) {
                    if (b->getStatus() == BookingStatus::CHECKED_IN) {
                        occupied.push_back(b);
                    }
                }
                printBookingsTable(occupied, "BOOKING DANG O (CHECKED_IN)");
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking can check-out: ");

                bookingManager.checkOut(bookingId);
                invoiceManager.createInvoice(bookingId);
                std::cout << "\nCheck-out thanh cong! Hoa don thanh toan:\n\n";
                invoiceManager.printInvoice(bookingId, std::cout);
                std::string txtPath = invoiceManager.saveInvoiceToFile(bookingId);
                std::cout << "\nDa luu hoa don ra file: " << txtPath << '\n';
            } else if (choice == 4) {
                std::vector<const Booking*> reserved;
                for (const Booking* b : bookingManager.getActiveBookings()) {
                    if (b->getStatus() == BookingStatus::RESERVED) {
                        reserved.push_back(b);
                    }
                }
                printBookingsTable(reserved, "BOOKING CO THE HUY (RESERVED)");
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking muon huy: ");
                if (InputUtils::readYesNo("  Xac nhan huy booking " + bookingId + "? (y/n): ")) {
                    bookingManager.cancelBooking(bookingId);
                    std::cout << "Da huy booking " << bookingId << " va tra phong ve trang thai Trong!\n";
                } else {
                    std::cout << "Da giu nguyen booking.\n";
                }
            } else if (choice == 5) {
                printBookingsTable(
                    bookingManager.getActiveBookings(),
                    "DANH SACH BOOKING DANG HOAT DONG"
                );
            } else if (choice == 6) {
                std::vector<const Booking*> list;
                for (const auto& b : bookingManager.getAll()) {
                    list.push_back(&b);
                }
                printBookingsTable(list, "DANH SACH TAT CA BOOKING");
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handleServiceMenu() {
    while (true) {
        printHeader("DICH VU PHAT SINH");
        std::cout << "  1. Xem danh muc dich vu\n";
        std::cout << "  2. Them dich vu cho booking dang o\n";
        std::cout << "  3. Xem dich vu da dung theo booking\n";
        std::cout << "  4. Them dich vu moi vao danh muc\n";
        std::cout << "  5. Sua dich vu trong danh muc\n";
        std::cout << "  6. Xoa dich vu khoi danh muc\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-6): ", 0, 6);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                printServicesTable(serviceManager.getAll(), "DANH MUC DICH VU");
            } else if (choice == 2) {
                std::vector<const Booking*> staying;
                for (const Booking* b : bookingManager.getActiveBookings()) {
                    if (b->getStatus() == BookingStatus::CHECKED_IN) {
                        staying.push_back(b);
                    }
                }
                printBookingsTable(staying, "BOOKING DANG O (CHECKED_IN)");
                printServicesTable(serviceManager.getAll(), "DANH MUC DICH VU");

                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking: ");
                std::string serviceId = InputUtils::readNonEmptyString("  Nhap ma dich vu: ");
                int qty = InputUtils::readInt("  Nhap so luong: ", 1, 10000);
                const ServiceUsage& usage = serviceManager.addUsage(bookingId, serviceId, qty);
                std::cout << "Da ghi nhan dich vu: " << usage << '\n';
            } else if (choice == 3) {
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking can xem: ");
                auto usages = serviceManager.getUsagesByBooking(bookingId);
                printHeader("DICH VU DA DUNG CUA BOOKING " + bookingId);
                if (usages.empty()) {
                    std::cout << "  (Chua su dung dich vu nao)\n";
                } else {
                    for (const ServiceUsage* u : usages) {
                        std::cout << "  " << *u << '\n';
                    }
                    std::cout << "--------------------------------------------------------------------------------\n";
                    std::cout << "  Tong tien dich vu: " << std::fixed << std::setprecision(0)
                              << serviceManager.calcServiceTotal(bookingId) << " VND\n";
                }
            } else if (choice == 4) {
                std::string serviceId = InputUtils::readNonEmptyString("  Nhap ma dich vu (vd S05): ");
                std::string name = InputUtils::readNonEmptyString("  Nhap ten dich vu: ");
                double price = InputUtils::readDouble("  Nhap don gia (VND): ", 1.0, 1000000000.0);
                ServiceCategory category = selectServiceCategory();
                serviceManager.addService(serviceId, name, price, category);
                std::cout << "Da them dich vu vao danh muc!\n";
            } else if (choice == 5) {
                std::string serviceId = InputUtils::readNonEmptyString("  Nhap ma dich vu can sua: ");
                std::string newName = InputUtils::readNonEmptyString("  Nhap ten moi: ");
                double newPrice = InputUtils::readDouble("  Nhap don gia moi (VND): ", 1.0, 1000000000.0);
                ServiceCategory newCategory = selectServiceCategory();
                serviceManager.updateService(serviceId, newName, newPrice, newCategory);
                std::cout << "Da cap nhat dich vu thanh cong (cac dong da dung truoc do khong doi)!\n";
            } else if (choice == 6) {
                std::string serviceId = InputUtils::readNonEmptyString("  Nhap ma dich vu can xoa: ");
                if (InputUtils::readYesNo("  Xac nhan xoa dich vu " + serviceId + "? (y/n): ")) {
                    serviceManager.removeService(serviceId);
                    std::cout << "Da xoa dich vu khoi danh muc!\n";
                } else {
                    std::cout << "Da huy thao tac xoa.\n";
                }
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handleInvoiceMenu() {
    while (true) {
        printHeader("QUAN LY HOA DON");
        std::cout << "  1. Xem / in lai hoa don theo ma booking\n";
        std::cout << "  2. Xuat hoa don ra file .txt (invoices/hoadon_<ma>.txt)\n";
        std::cout << "  3. Xem danh sach tat ca hoa don\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-3): ", 0, 3);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking: ");
                std::cout << '\n';
                invoiceManager.printInvoice(bookingId, std::cout);
            } else if (choice == 2) {
                std::string bookingId = InputUtils::readNonEmptyString("  Nhap ma booking: ");
                std::string path = invoiceManager.saveInvoiceToFile(bookingId);
                std::cout << "Da xuat hoa don ra file: " << path << '\n';
            } else if (choice == 3) {
                printInvoicesTable(invoiceManager.getAll(), "DANH SACH TAT CA HOA DON");
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handleReportMenu() {
    while (true) {
        printHeader("BAO CAO THONG KE");
        std::cout << "  1. Doanh thu theo thang\n";
        std::cout << "  2. So khach thue theo thang\n";
        std::cout << "  3. Ty le lap day & phong trong\n";
        std::cout << "  4. Bieu do so sanh luot dat phong\n";
        std::cout << "  5. Bang tong hop 12 thang\n";
        std::cout << "  6. Xem day du & xuat file bao cao (reports/baocao_<nam>.txt)\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-6): ", 0, 6);
        if (choice == 0) return;

        try {
            int year = InputUtils::readInt("  Nhap nam thong ke (1970-9999): ", 1970, 9999);
            std::cout << '\n';
            if (choice == 1) {
                reportManager.printRevenueReport(year, std::cout);
            } else if (choice == 2) {
                reportManager.printGuestReport(year, std::cout);
            } else if (choice == 3) {
                reportManager.printOccupancyReport(year, std::cout);
            } else if (choice == 4) {
                reportManager.printBookingChart(year, std::cout);
            } else if (choice == 5) {
                reportManager.printSummaryTable(year, std::cout);
            } else if (choice == 6) {
                reportManager.printFullReport(year, std::cout);
                std::string path = reportManager.exportReportToFile(year, "reports");
                std::cout << "\nDa xuat bao cao ra file: " << path << '\n';
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::handlePromotionMenu() {
    while (true) {
        printHeader("KHUYEN MAI THEO QUY");
        std::cout << "  1. Xem danh sach khuyen mai\n";
        std::cout << "  2. Them / cap nhat khuyen mai\n";
        std::cout << "  3. Xoa khuyen mai\n";
        std::cout << "  0. Quay lai\n";

        int choice = InputUtils::readInt("Chon chuc nang (0-3): ", 0, 3);
        if (choice == 0) return;

        try {
            if (choice == 1) {
                printPromotionsTable(promotionManager.getAll(), "DANH SACH KHUYEN MAI THEO QUY");
            } else if (choice == 2) {
                int quarter = InputUtils::readInt("  Nhap quy (1-4): ", 1, 4);
                std::string roomType = selectRoomType(true);
                double percent = InputUtils::readDouble("  Nhap phan tram giam (0-100): ", 0.0, 100.0);
                std::string desc = InputUtils::readNonEmptyString("  Nhap mo ta khuyen mai: ");
                promotionManager.setPromotion(quarter, roomType, percent, desc);
                std::cout << "Da luu chuong trinh khuyen mai!\n";
            } else if (choice == 3) {
                printPromotionsTable(promotionManager.getAll(), "DANH SACH KHUYEN MAI HIEN TAI");
                int quarter = InputUtils::readInt("  Nhap quy can xoa (1-4): ", 1, 4);
                std::string roomType = selectRoomType(true);
                if (InputUtils::readYesNo("  Xac nhan xoa khuyen mai Quy " + std::to_string(quarter) +
                                          " - " + roomType + "? (y/n): ")) {
                    promotionManager.removePromotion(quarter, roomType);
                    std::cout << "Da xoa khuyen mai!\n";
                } else {
                    std::cout << "Da huy thao tac xoa.\n";
                }
            }
        } catch (const std::exception& ex) {
            std::cout << "\n[LOI] " << ex.what() << '\n';
        }
        InputUtils::waitForEnter();
    }
}

void Menu::appendChatLog(const std::string& question, const std::string& answer) {
    try {
        std::error_code ec;
        std::filesystem::create_directories("data", ec);
        std::ofstream logFile("data/chat_log.txt", std::ios::app);
        if (!logFile.is_open()) {
            return;
        }
        std::string nowStr = TimeUtils::getCurrentTimeString();
        logFile << "[" << nowStr << "] Ban: " << question << '\n';
        logFile << "[" << nowStr << "] Bot: " << answer << "\n\n";
    } catch (...) {
    }
}

void Menu::handleChatboxMenu() {
    printHeader("CHATBOX TRO LY NOI BO (go 'help' de xem goi y, go 'thoat' de ve)");
    while (true) {
        std::cout << "Ban: ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            if (std::cin.eof()) {
                throw std::runtime_error("Ket thuc luong nhap (EOF)");
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::size_t start = 0;
        while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start])) != 0) {
            ++start;
        }
        std::size_t end = line.size();
        while (end > start && std::isspace(static_cast<unsigned char>(line[end - 1])) != 0) {
            --end;
        }
        std::string question = line.substr(start, end - start);
        if (question.empty()) {
            continue;
        }

        std::string lowered = question;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (lowered == "thoat" || lowered == "exit" || lowered == "quit" || lowered == "0") {
            return;
        }

        try {
            std::string answer = chatEngine->reply(question);
            std::cout << "Bot: " << answer << "\n\n";
            appendChatLog(question, answer);
        } catch (const std::exception& ex) {
            std::cout << "Bot: [LOI] " << ex.what() << "\n\n";
        }
    }
}

