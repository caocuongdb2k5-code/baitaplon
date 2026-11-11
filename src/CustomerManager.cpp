#include "CustomerManager.h"
#include "JsonUtils.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

std::string CustomerManager::toLower(const std::string& str) {
    std::string lowered = str;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

void CustomerManager::addCustomer(const std::string& cccd,
                                  const std::string& name,
                                  const std::string& phone) {
    // Validate truoc khi kiem tra trung lap
    Customer newCustomer(cccd, name, phone);
    if (findByCccd(cccd) != nullptr) {
        throw std::runtime_error("CCCD da ton tai trong he thong: " + cccd);
    }
    customers.push_back(std::move(newCustomer));
}

void CustomerManager::updateCustomer(const std::string& cccd,
                                     const std::string& newName,
                                     const std::string& newPhone) {
    Customer* existing = findByCccd(cccd);
    if (existing == nullptr) {
        throw std::runtime_error("Khong tim thay khach hang co CCCD: " + cccd);
    }
    // Validate ca ten va SDT truoc khi ghi de de tranh cap nhat do dang
    Customer::validateName(newName);
    Customer::validatePhone(newPhone);

    existing->setName(newName);
    existing->setPhone(newPhone);
}

Customer* CustomerManager::findByCccd(const std::string& cccd) {
    for (auto& customer : customers) {
        if (customer.getCccd() == cccd) {
            return &customer;
        }
    }
    return nullptr;
}

const Customer* CustomerManager::findByCccd(const std::string& cccd) const {
    for (const auto& customer : customers) {
        if (customer.getCccd() == cccd) {
            return &customer;
        }
    }
    return nullptr;
}

const Customer* CustomerManager::findByPhone(const std::string& phone) const {
    for (const auto& customer : customers) {
        if (customer.getPhone() == phone) {
            return &customer;
        }
    }
    return nullptr;
}

std::vector<const Customer*> CustomerManager::searchByName(const std::string& keyword) const {
    std::vector<const Customer*> result;
    std::string lowerKeyword = toLower(keyword);
    if (lowerKeyword.empty()) {
        return result;
    }

    for (const auto& customer : customers) {
        if (toLower(customer.getName()).find(lowerKeyword) != std::string::npos) {
            result.push_back(&customer);
        }
    }
    return result;
}

const std::vector<Customer>& CustomerManager::getAll() const {
    return customers;
}

void CustomerManager::loadFromFile(const std::string& path) {
    nlohmann::json arr = JsonUtils::readJsonArray(path);
    customers.clear();

    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            Customer customer = Customer::fromJson(item);
            if (findByCccd(customer.getCccd()) != nullptr) {
                throw std::runtime_error("Trung CCCD trong file: " + customer.getCccd());
            }
            customers.push_back(std::move(customer));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 khach hang loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 khach hang khong hop le trong " + path + ": " + ex.what());
        }
    }
}

void CustomerManager::saveToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& customer : customers) {
        arr.push_back(customer.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

