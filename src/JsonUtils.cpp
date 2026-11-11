#include "JsonUtils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace JsonUtils {

void logWarning(const std::string& message) {
    std::cerr << "[CANH BAO] " << message << '\n';
}

nlohmann::json readJsonArray(const std::string& filePath) {
    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec)) {
        return nlohmann::json::array();
    }

    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        logWarning("Khong the mo file de doc: " + filePath);
        return nlohmann::json::array();
    }

    // Neu file hoan toan rong thi xem nhu mang rong
    if (inFile.peek() == std::ifstream::traits_type::eof()) {
        return nlohmann::json::array();
    }

    try {
        nlohmann::json data;
        inFile >> data;
        if (!data.is_array()) {
            logWarning("Noi dung file khong phai mang JSON: " + filePath);
            return nlohmann::json::array();
        }
        return data;
    } catch (const nlohmann::json::exception& ex) {
        logWarning("File JSON bi hong (" + filePath + "): " + ex.what());
        return nlohmann::json::array();
    }
}

void writeJsonArray(const std::string& filePath, const nlohmann::json& arr) {
    if (!arr.is_array()) {
        throw std::invalid_argument("Du lieu ghi ra file JSON phai la mot mang");
    }

    std::filesystem::path targetPath(filePath);
    if (targetPath.has_parent_path()) {
        std::filesystem::create_directories(targetPath.parent_path());
    }

    std::string tempPath = filePath + ".tmp";
    {
        std::ofstream outFile(tempPath, std::ios::trunc);
        if (!outFile.is_open()) {
            throw std::runtime_error("Khong the mo file tam de ghi: " + tempPath);
        }

        outFile << arr.dump(4) << '\n';
        outFile.flush();
        if (!outFile.good()) {
            outFile.close();
            std::error_code rmEc;
            std::filesystem::remove(tempPath, rmEc);
            throw std::runtime_error("Loi khi ghi du lieu ra file tam: " + tempPath);
        }
    }

    std::error_code ec;
    std::filesystem::rename(tempPath, targetPath, ec);
    if (ec) {
        // Mot so moi truong Windows khong cho rename de len file da ton tai
        std::filesystem::remove(targetPath, ec);
        std::filesystem::rename(tempPath, targetPath);
    }
}

} // namespace JsonUtils

