#ifndef SERVICE_H
#define SERVICE_H

#include "Enums.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop dai dien cho mot dich vu trong bang gia khach san
class Service {
private:
    std::string serviceId;
    std::string name;
    double unitPrice;
    ServiceCategory category;

public:
    Service(const std::string& serviceId,
            const std::string& name,
            double unitPrice,
            ServiceCategory category);

    // Getter
    const std::string& getServiceId() const;
    const std::string& getName() const;
    double getUnitPrice() const;
    ServiceCategory getCategory() const;

    // Setter co kiem tra du lieu hop le
    void setName(const std::string& name);
    void setUnitPrice(double unitPrice);
    void setCategory(ServiceCategory category);

    std::string toString() const;
    nlohmann::json toJson() const;
    static Service fromJson(const nlohmann::json& j);

    bool operator==(const Service& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Service& service);
};

#endif // SERVICE_H

