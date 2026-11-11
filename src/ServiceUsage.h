#ifndef SERVICE_USAGE_H
#define SERVICE_USAGE_H

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop luu mot lan su dung dich vu cua booking (luu gia tai thoi diem dung)
class ServiceUsage {
private:
    std::string bookingId;
    std::string serviceId;
    std::string serviceName;
    int quantity;
    double unitPriceAtUse;
    std::string usedAt;

public:
    ServiceUsage(const std::string& bookingId,
                 const std::string& serviceId,
                 const std::string& serviceName,
                 int quantity,
                 double unitPriceAtUse,
                 const std::string& usedAt);

    // Getter
    const std::string& getBookingId() const;
    const std::string& getServiceId() const;
    const std::string& getServiceName() const;
    int getQuantity() const;
    double getUnitPriceAtUse() const;
    const std::string& getUsedAt() const;

    // Thanh tien = so luong * don gia tai luc dung
    double getTotal() const;

    std::string toString() const;
    nlohmann::json toJson() const;
    static ServiceUsage fromJson(const nlohmann::json& j);

    friend std::ostream& operator<<(std::ostream& os, const ServiceUsage& usage);
};

#endif // SERVICE_USAGE_H

