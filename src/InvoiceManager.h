#ifndef INVOICE_MANAGER_H
#define INVOICE_MANAGER_H

#include "BookingManager.h"
#include "CustomerManager.h"
#include "Invoice.h"
#include "PromotionManager.h"
#include "RoomManager.h"
#include "ServiceManager.h"
#include <iostream>
#include <string>
#include <vector>

// Lop quan ly tinh tien, xuat hoa don va luu hoa don ra JSON / TXT
class InvoiceManager {
private:
    const BookingManager& bookingManager;
    const RoomManager& roomManager;
    const CustomerManager& customerManager;
    const ServiceManager& serviceManager;
    const PromotionManager& promotionManager;

    std::vector<Invoice> invoices;
    std::string invoicesFilePath;
    std::string invoicesDir;
    int nextInvoiceNumber;

    static int extractInvoiceNumber(const std::string& invoiceId);
    const Invoice* findByInvoiceId(const std::string& invoiceId) const;
    std::string generateNextInvoiceId();

public:
    InvoiceManager(const BookingManager& bookingManager,
                   const RoomManager& roomManager,
                   const CustomerManager& customerManager,
                   const ServiceManager& serviceManager,
                   const PromotionManager& promotionManager,
                   const std::string& invoicesFilePath = "data/invoices.json",
                   const std::string& invoicesDir = "invoices");

    // Tao hoa don cho booking da CHECKED_OUT va chua co hoa don
    const Invoice& createInvoice(const std::string& bookingId);

    // In hoa don ra stream (man hinh) va xuat ra file invoices/hoadon_<bookingId>.txt
    void printInvoice(const std::string& bookingId, std::ostream& os) const;
    std::string saveInvoiceToFile(const std::string& bookingId) const;

    // Tim kiem & truy van
    Invoice* findInvoice(const std::string& bookingId);
    const Invoice* findInvoice(const std::string& bookingId) const;
    const std::vector<Invoice>& getAll() const;

    // Doc va luu JSON
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
};

#endif // INVOICE_MANAGER_H

