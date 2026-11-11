#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop dai dien cho khach hang
class Customer {
private:
    std::string cccd;
    std::string name;
    std::string phone;

public:
    Customer(const std::string& cccd, const std::string& name, const std::string& phone);

    // Getter
    const std::string& getCccd() const;
    const std::string& getName() const;
    const std::string& getPhone() const;

    // Setter (khong cho sua CCCD vi la khoa chinh)
    void setName(const std::string& name);
    void setPhone(const std::string& phone);

    // Cac ham static kiem tra tinh hop le cua du lieu
    static void validateCccd(const std::string& cccd);
    static void validatePhone(const std::string& phone);
    static void validateName(const std::string& name);

    std::string toString() const;
    nlohmann::json toJson() const;
    static Customer fromJson(const nlohmann::json& j);

    bool operator==(const Customer& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Customer& customer);
};

#endif // CUSTOMER_H

