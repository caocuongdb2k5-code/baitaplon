#ifndef CUSTOMER_MANAGER_H
#define CUSTOMER_MANAGER_H

#include "Customer.h"
#include <string>
#include <vector>

// Lop quan ly danh sach khach hang va luat nghiep vu khach hang
class CustomerManager {
private:
    std::vector<Customer> customers;

    static std::string toLower(const std::string& str);

public:
    CustomerManager() = default;

    // Them va cap nhat khach hang (khong cho sua CCCD)
    void addCustomer(const std::string& cccd, const std::string& name, const std::string& phone);
    void updateCustomer(const std::string& cccd, const std::string& newName, const std::string& newPhone);

    // Tim kiem & truy van
    Customer* findByCccd(const std::string& cccd);
    const Customer* findByCccd(const std::string& cccd) const;
    const Customer* findByPhone(const std::string& phone) const;
    std::vector<const Customer*> searchByName(const std::string& keyword) const;
    const std::vector<Customer>& getAll() const;

    // Doc va luu danh sach khach hang qua file JSON
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
};

#endif // CUSTOMER_MANAGER_H

