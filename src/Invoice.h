#ifndef INVOICE_H
#define INVOICE_H

#include "ServiceUsage.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>
#include <vector>

// Lop dai dien cho hoa don thanh toan tra phong
class Invoice {
private:
    std::string invoiceId;
    std::string bookingId;
    std::string cccd;
    std::string customerName;
    std::string roomId;
    std::string roomType;
    double pricePerDay;
    std::string checkIn;
    std::string checkOut;
    int days;
    double roomCost;
    double serviceCost;
    double discountPercent;
    double discountAmount;
    double total;
    std::string issuedAt;

    // Chi tiet dich vu trong bo nho de in hoa don (khong ghi vao invoices.json)
    std::vector<ServiceUsage> serviceDetails;

public:
    Invoice(const std::string& invoiceId,
            const std::string& bookingId,
            const std::string& cccd,
            const std::string& customerName,
            const std::string& roomId,
            const std::string& roomType,
            double pricePerDay,
            const std::string& checkIn,
            const std::string& checkOut,
            int days,
            double roomCost,
            double serviceCost,
            double discountPercent,
            double discountAmount,
            double total,
            const std::string& issuedAt);

    // Getter
    const std::string& getInvoiceId() const;
    const std::string& getBookingId() const;
    const std::string& getCccd() const;
    const std::string& getCustomerName() const;
    const std::string& getRoomId() const;
    const std::string& getRoomType() const;
    double getPricePerDay() const;
    const std::string& getCheckIn() const;
    const std::string& getCheckOut() const;
    int getDays() const;
    double getRoomCost() const;
    double getServiceCost() const;
    double getDiscountPercent() const;
    double getDiscountAmount() const;
    double getTotal() const;
    const std::string& getIssuedAt() const;

    void setServiceDetails(const std::vector<ServiceUsage>& details);
    const std::vector<ServiceUsage>& getServiceDetails() const;

    // In hoa don can cot dep kem chi tiet dich vu
    void print(std::ostream& os) const;
    void print(std::ostream& os, const std::vector<const ServiceUsage*>& usages) const;

    nlohmann::json toJson() const;
    static Invoice fromJson(const nlohmann::json& j);

    bool operator==(const Invoice& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Invoice& invoice);
};

#endif // INVOICE_H

