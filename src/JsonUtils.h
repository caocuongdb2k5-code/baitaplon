#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "nlohmann/json.hpp"
#include <string>

// Tien ich doc/ghi file JSON an toan va ghi canh bao
namespace JsonUtils {

// Doc file JSON dang mang; neu file chua ton tai hoac JSON hong thi tra ve mang rong
nlohmann::json readJsonArray(const std::string& filePath);

// Ghi mang JSON ra file an toan (tu tao thu muc neu thieu, ghi file tam roi doi ten)
void writeJsonArray(const std::string& filePath, const nlohmann::json& arr);

// In thong bao canh bao khi doc du lieu loi (khong lam sap chuong trinh)
void logWarning(const std::string& message);

} // namespace JsonUtils

#endif // JSON_UTILS_H

