#include "InputUtils.h"
#include "TimeUtils.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

std::string trim(const std::string& str) {
    std::size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])) != 0) {
        ++start;
    }
    std::size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])) != 0) {
        --end;
    }
    return str.substr(start, end - start);
}

std::string readLineSafe(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        if (std::cin.eof()) {
            throw std::runtime_error("Ket thuc luong nhap (EOF)");
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return "";
    }
    return line;
}

} // namespace

namespace InputUtils {

int readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        if (line.empty()) {
            std::cout << "Loi: Vui long nhap so nguyen tu " << minVal << " den " << maxVal << ".\n";
            continue;
        }
        try {
            std::size_t pos = 0;
            int value = std::stoi(line, &pos);
            if (pos != line.size()) {
                std::cout << "Loi: Vui long chi nhap so nguyen hop le.\n";
                continue;
            }
            if (value < minVal || value > maxVal) {
                std::cout << "Loi: Gia tri phai nam trong khoang ["
                          << minVal << " .. " << maxVal << "].\n";
                continue;
            }
            return value;
        } catch (const std::exception&) {
            std::cout << "Loi: Gia tri khong phai so nguyen hop le.\n";
        }
    }
}

double readDouble(const std::string& prompt, double minVal, double maxVal) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        if (line.empty()) {
            std::cout << "Loi: Vui long nhap so hop le.\n";
            continue;
        }
        try {
            std::size_t pos = 0;
            double value = std::stod(line, &pos);
            if (pos != line.size()) {
                std::cout << "Loi: Vui long chi nhap so hop le.\n";
                continue;
            }
            if (value < minVal || value > maxVal) {
                std::cout << "Loi: Gia tri phai nam trong khoang ["
                          << minVal << " .. " << maxVal << "].\n";
                continue;
            }
            return value;
        } catch (const std::exception&) {
            std::cout << "Loi: Gia tri khong phai so hop le.\n";
        }
    }
}

std::string readNonEmptyString(const std::string& prompt) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        if (!line.empty()) {
            return line;
        }
        std::cout << "Loi: Khong duoc de trong. Vui long nhap lai.\n";
    }
}

std::string readDate(const std::string& prompt) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        if (line.size() == 10 && TimeUtils::isValidTimeString(line + " 00:00")) {
            return line;
        }
        std::cout << "Loi: Ngay phai theo dinh dang YYYY-MM-DD (vd 2026-11-11).\n";
    }
}

std::string readDateTime(const std::string& prompt) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        if (TimeUtils::isValidTimeString(line)) {
            return line;
        }
        std::cout << "Loi: Thoi gian phai theo dinh dang YYYY-MM-DD HH:MM (vd 2026-11-11 14:00).\n";
    }
}

bool readYesNo(const std::string& prompt) {
    while (true) {
        std::string line = trim(readLineSafe(prompt));
        std::transform(line.begin(), line.end(), line.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (line == "y" || line == "yes" || line == "c" || line == "co") {
            return true;
        }
        if (line == "n" || line == "no" || line == "k" || line == "khong") {
            return false;
        }
        std::cout << "Loi: Vui long nhap 'y' (co) hoac 'n' (khong).\n";
    }
}

void waitForEnter(const std::string& message) {
    std::cout << '\n' << message;
    std::string dummy;
    if (!std::getline(std::cin, dummy) && !std::cin.eof()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

} // namespace InputUtils

