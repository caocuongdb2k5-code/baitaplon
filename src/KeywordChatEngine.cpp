#include "KeywordChatEngine.h"
#include "Booking.h"
#include "Customer.h"
#include "Enums.h"
#include "Invoice.h"
#include "Promotion.h"
#include "Room.h"
#include "Service.h"
#include "ServiceUsage.h"
#include "TimeUtils.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace {

char mapVietnameseCodepoint(char32_t cp) {
    switch (cp) {
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x0103:
        case 0x1EA1: case 0x1EA3: case 0x1EA5: case 0x1EA7: case 0x1EA9:
        case 0x1EAB: case 0x1EAD: case 0x1EAF: case 0x1EB1: case 0x1EB3:
        case 0x1EB5: case 0x1EB7:
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x0102:
        case 0x1EA0: case 0x1EA2: case 0x1EA4: case 0x1EA6: case 0x1EA8:
        case 0x1EAA: case 0x1EAC: case 0x1EAE: case 0x1EB0: case 0x1EB2:
        case 0x1EB4: case 0x1EB6:
            return 'a';
        case 0x0111: case 0x0110:
            return 'd';
        case 0x00E8: case 0x00E9: case 0x00EA:
        case 0x1EB9: case 0x1EBB: case 0x1EBD: case 0x1EBF: case 0x1EC1:
        case 0x1EC3: case 0x1EC5: case 0x1EC7:
        case 0x00C8: case 0x00C9: case 0x00CA:
        case 0x1EB8: case 0x1EBA: case 0x1EBC: case 0x1EBE: case 0x1EC0:
        case 0x1EC2: case 0x1EC4: case 0x1EC6:
            return 'e';
        case 0x00EC: case 0x00ED: case 0x0129: case 0x1EC9: case 0x1ECB:
        case 0x00CC: case 0x00CD: case 0x0128: case 0x1EC8: case 0x1ECA:
            return 'i';
        case 0x00F2: case 0x00F3: case 0x00F4: case 0x00F5: case 0x01A1:
        case 0x1ECD: case 0x1ECF: case 0x1ED1: case 0x1ED3: case 0x1ED5:
        case 0x1ED7: case 0x1ED9: case 0x1EDB: case 0x1EDD: case 0x1EDF:
        case 0x1EE1: case 0x1EE3:
        case 0x00D2: case 0x00D3: case 0x00D4: case 0x00D5: case 0x01A0:
        case 0x1ECC: case 0x1ECE: case 0x1ED0: case 0x1ED2: case 0x1ED4:
        case 0x1ED6: case 0x1ED8: case 0x1EDA: case 0x1EDC: case 0x1EDE:
        case 0x1EE0: case 0x1EE2:
            return 'o';
        case 0x00F9: case 0x00FA: case 0x0169: case 0x01B0:
        case 0x1EE5: case 0x1EE7: case 0x1EE9: case 0x1EEB: case 0x1EED:
        case 0x1EEF: case 0x1EF1:
        case 0x00D9: case 0x00DA: case 0x0168: case 0x01AF:
        case 0x1EE4: case 0x1EE6: case 0x1EE8: case 0x1EEA: case 0x1EEC:
        case 0x1EEE: case 0x1EF0:
            return 'u';
        case 0x00FD: case 0x1EF3: case 0x1EF5: case 0x1EF7: case 0x1EF9:
        case 0x00DD: case 0x1EF2: case 0x1EF4: case 0x1EF6: case 0x1EF8:
            return 'y';
        default:
            return '\0';
    }
}

bool isAllDigits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

std::string toUpperAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return s;
}

std::string formatMoney(double amount) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << amount;
    return oss.str();
}

} // namespace

KeywordChatEngine::KeywordChatEngine(const RoomManager& roomManager,
                                     const CustomerManager& customerManager,
                                     const BookingManager& bookingManager,
                                     const ServiceManager& serviceManager,
                                     const PromotionManager& promotionManager,
                                     const InvoiceManager& invoiceManager,
                                     const ReportManager& reportManager)
    : roomManager(roomManager),
      customerManager(customerManager),
      bookingManager(bookingManager),
      serviceManager(serviceManager),
      promotionManager(promotionManager),
      invoiceManager(invoiceManager),
      reportManager(reportManager) {
}

std::string KeywordChatEngine::normalizeVietnamese(const std::string& text) {
    std::string out;
    out.reserve(text.size());

    std::size_t i = 0;
    while (i < text.size()) {
        unsigned char byte = static_cast<unsigned char>(text[i]);
        char32_t cp = 0;
        std::size_t bytes = 1;

        if (byte < 0x80) {
            cp = byte;
            bytes = 1;
        } else if ((byte & 0xE0) == 0xC0 && i + 1 < text.size()) {
            cp = ((byte & 0x1F) << 6) |
                 (static_cast<unsigned char>(text[i + 1]) & 0x3F);
            bytes = 2;
        } else if ((byte & 0xF0) == 0xE0 && i + 2 < text.size()) {
            cp = ((byte & 0x0F) << 12) |
                 ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(text[i + 2]) & 0x3F);
            bytes = 3;
        } else if ((byte & 0xF8) == 0xF0 && i + 3 < text.size()) {
            cp = ((byte & 0x07) << 18) |
                 ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(text[i + 3]) & 0x3F);
            bytes = 4;
        } else {
            ++i;
            continue;
        }
        i += bytes;

        // Bo qua dau thanh to hop (NFD combining diacritical marks)
        if (cp >= 0x0300 && cp <= 0x036F) {
            continue;
        }

        if (cp < 0x80) {
            char ch = static_cast<char>(std::tolower(static_cast<unsigned char>(cp)));
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == ' ' || ch == '/' || ch == '.') {
                out.push_back(ch);
            } else {
                out.push_back(' ');
            }
        } else {
            char mapped = mapVietnameseCodepoint(cp);
            if (mapped != '\0') {
                out.push_back(mapped);
            } else {
                out.push_back(' ');
            }
        }
    }

    // Gom nhieu khoang trang thanh 1
    std::string collapsed;
    bool prevSpace = true;
    for (char ch : out) {
        if (ch == ' ') {
            if (!prevSpace) {
                collapsed.push_back(' ');
                prevSpace = true;
            }
        } else {
            collapsed.push_back(ch);
            prevSpace = false;
        }
    }
    if (!collapsed.empty() && collapsed.back() == ' ') {
        collapsed.pop_back();
    }
    return collapsed;
}

std::vector<std::string> KeywordChatEngine::tokenize(const std::string& normalizedText) {
    std::vector<std::string> tokens;
    std::istringstream iss(normalizedText);
    std::string tok;
    while (iss >> tok) {
        while (!tok.empty() && (tok.back() == '.' || tok.back() == '/')) {
            tok.pop_back();
        }
        if (!tok.empty()) {
            tokens.push_back(tok);
        }
    }
    return tokens;
}

bool KeywordChatEngine::containsPhrase(const std::string& normalizedText, const std::string& phrase) {
    if (phrase.empty()) {
        return false;
    }
    std::string paddedText = " " + normalizedText + " ";
    std::string paddedPhrase = " " + phrase + " ";
    return paddedText.find(paddedPhrase) != std::string::npos;
}

bool KeywordChatEngine::containsAnyPhrase(const std::string& normalizedText,
                                          const std::vector<std::string>& phrases) {
    for (const auto& p : phrases) {
        if (containsPhrase(normalizedText, p)) {
            return true;
        }
    }
    return false;
}

std::string KeywordChatEngine::detectRoomType(const std::string& normalizedText,
                                              const std::vector<std::string>& tokens) {
    if (containsAnyPhrase(normalizedText, {"phong don", "loai don", "single"}) ||
        std::find(tokens.begin(), tokens.end(), "don") != tokens.end()) {
        return "SINGLE";
    }
    if (containsAnyPhrase(normalizedText, {"phong doi", "loai doi", "double"}) ||
        std::find(tokens.begin(), tokens.end(), "doi") != tokens.end()) {
        return "DOUBLE";
    }
    if (containsAnyPhrase(normalizedText, {"phong vip", "loai vip", "vip"}) ||
        std::find(tokens.begin(), tokens.end(), "vip") != tokens.end()) {
        return "VIP";
    }
    return "";
}

std::string KeywordChatEngine::detectRoomId(const std::vector<std::string>& tokens) const {
    for (const std::string& tok : tokens) {
        std::string upper = toUpperAscii(tok);
        if (roomManager.findRoom(upper) != nullptr) {
            return upper;
        }
    }
    for (const std::string& tok : tokens) {
        if (tok.size() >= 2 && tok[0] == 'p' && isAllDigits(tok.substr(1))) {
            return toUpperAscii(tok);
        }
    }
    return "";
}

std::string KeywordChatEngine::detectBookingId(const std::vector<std::string>& tokens) {
    for (const std::string& tok : tokens) {
        if (tok.size() >= 2 && tok[0] == 'b' && isAllDigits(tok.substr(1))) {
            return toUpperAscii(tok);
        }
    }
    return "";
}

double KeywordChatEngine::detectBudgetAmount(const std::string& normalizedText,
                                             const std::vector<std::string>& tokens) {
    (void)normalizedText;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        // Dang 500k hoac 450k
        if (tok.size() >= 2 && tok.back() == 'k') {
            std::string numPart = tok.substr(0, tok.size() - 1);
            try {
                std::size_t pos = 0;
                double val = std::stod(numPart, &pos);
                if (pos == numPart.size() && val > 0.0) {
                    return val * 1000.0;
                }
            } catch (...) {
            }
        }
        // Dang 1tr hoac 1.5tr
        if (tok.size() >= 3 && tok.substr(tok.size() - 2) == "tr") {
            std::string numPart = tok.substr(0, tok.size() - 2);
            try {
                std::size_t pos = 0;
                double val = std::stod(numPart, &pos);
                if (pos == numPart.size() && val > 0.0) {
                    return val * 1000000.0;
                }
            } catch (...) {
            }
        }
        // So nguyen >= 10000 (vd 500000) hoac so nho di kem "trieu" / "nghin"
        try {
            std::size_t pos = 0;
            double val = std::stod(tok, &pos);
            if (pos == tok.size() && val > 0.0) {
                if (i + 1 < tokens.size() && (tokens[i + 1] == "trieu" || tokens[i + 1] == "tr")) {
                    return val * 1000000.0;
                }
                if (i + 1 < tokens.size() && (tokens[i + 1] == "nghin" || tokens[i + 1] == "ngan")) {
                    return val * 1000.0;
                }
                if (val >= 10000.0) {
                    return val;
                }
            }
        } catch (...) {
        }
    }
    return 0.0;
}

int KeywordChatEngine::detectQuarter(const std::string& normalizedText,
                                     const std::vector<std::string>& tokens) {
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if ((tokens[i] == "quy" || tokens[i] == "q") && i + 1 < tokens.size()) {
            if (isAllDigits(tokens[i + 1])) {
                int q = std::stoi(tokens[i + 1]);
                if (q >= 1 && q <= 4) {
                    return q;
                }
            }
        }
        if (tokens[i].size() == 2 && tokens[i][0] == 'q' &&
            tokens[i][1] >= '1' && tokens[i][1] <= '4') {
            return tokens[i][1] - '0';
        }
    }
    (void)normalizedText;
    return TimeUtils::getQuarter(TimeUtils::getCurrentTimeString());
}

void KeywordChatEngine::detectMonthYear(const std::string& normalizedText,
                                        const std::vector<std::string>& tokens,
                                        int& outMonth,
                                        int& outYear,
                                        bool& hasExplicitMonth) {
    std::string nowStr = TimeUtils::getCurrentTimeString();
    outMonth = TimeUtils::getMonth(nowStr);
    outYear = TimeUtils::getYear(nowStr);
    hasExplicitMonth = false;

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        // Dang MM/YYYY (vd 11/2026)
        auto slashPos = tok.find('/');
        if (slashPos != std::string::npos) {
            std::string left = tok.substr(0, slashPos);
            std::string right = tok.substr(slashPos + 1);
            if (isAllDigits(left) && isAllDigits(right)) {
                int m = std::stoi(left);
                int y = std::stoi(right);
                if (m >= 1 && m <= 12 && y >= 1970 && y <= 9999) {
                    outMonth = m;
                    outYear = y;
                    hasExplicitMonth = true;
                    return;
                }
            }
        }
        if ((tok == "thang" || tok == "t") && i + 1 < tokens.size() && isAllDigits(tokens[i + 1])) {
            int m = std::stoi(tokens[i + 1]);
            if (m >= 1 && m <= 12) {
                outMonth = m;
                hasExplicitMonth = true;
            }
        }
        if (tok == "nam" && i + 1 < tokens.size() && isAllDigits(tokens[i + 1])) {
            int y = std::stoi(tokens[i + 1]);
            if (y >= 1970 && y <= 9999) {
                outYear = y;
            }
        }
        if (tok.size() == 4 && isAllDigits(tok)) {
            int y = std::stoi(tok);
            if (y >= 1970 && y <= 9999) {
                outYear = y;
            }
        }
    }
    (void)normalizedText;
}

std::string KeywordChatEngine::handleHelp() const {
    return "Toi la tro ly noi bo (chi doc du lieu). Ban co the hoi:\n"
           "  - Con phong trong nao / Con phong don / doi / vip khong?\n"
           "  - Gia phong don / doi / vip bao nhieu?\n"
           "  - Phong P201 tinh trang sao?\n"
           "  - Phong duoi 500k / duoi 700000\n"
           "  - Danh sach phong dang thue / dang don / dang dat\n"
           "  - Khach Nguyen Van An (hoac CCCD / SDT) dang o phong nao?\n"
           "  - Bang gia dich vu / Tong dich vu booking B001\n"
           "  - Khuyen mai quy 4\n"
           "  - Doanh thu thang 11/2026\n"
           "  - Ty le lap day thang 11/2026";
}

std::string KeywordChatEngine::handleGreeting() const {
    return "Xin chao! Toi la tro ly quan ly khach san. "
           "Ban can tra cuu phong, khach hang, dich vu, khuyen mai hay bao cao? "
           "(Go 'help' de xem cac cau hoi mau)";
}

std::string KeywordChatEngine::handleBudgetSuggestion(const std::string& normalizedText,
                                                      const std::vector<std::string>& tokens) const {
    double maxBudget = detectBudgetAmount(normalizedText, tokens);
    std::string typeFilter = detectRoomType(normalizedText, tokens);

    std::vector<const Room*> matches;
    for (const auto& r : roomManager.getAll()) {
        if (r->getStatus() != RoomStatus::AVAILABLE) {
            continue;
        }
        if (!typeFilter.empty() && r->getType() != typeFilter) {
            continue;
        }
        if (r->getPricePerDay() <= maxBudget) {
            matches.push_back(r.get());
        }
    }

    if (matches.empty()) {
        return "Hien khong co phong trong nao co gia <= " + formatMoney(maxBudget) + " VND/ngay.";
    }

    std::ostringstream oss;
    oss << "Goi y " << matches.size() << " phong trong co gia <= "
        << formatMoney(maxBudget) << " VND/ngay:\n";
    for (const Room* r : matches) {
        oss << "  - " << r->getId() << " (" << r->getType() << "): "
            << formatMoney(r->getPricePerDay()) << " VND/ngay\n";
    }
    std::string res = oss.str();
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

std::string KeywordChatEngine::handleSpecificRoom(const std::string& roomId) const {
    const Room* room = roomManager.findRoom(roomId);
    if (room == nullptr) {
        return "Khong tim thay phong " + roomId + " trong he thong.";
    }

    std::ostringstream oss;
    oss << "Thong tin phong " << room->getId() << ":\n"
        << "  - Loai: " << room->getType() << " | Gia/ngay: "
        << formatMoney(room->getPricePerDay()) << " VND\n"
        << "  - Trang thai: " << roomStatusToDisplay(room->getStatus()) << '\n'
        << "  - Tien ich: " << room->getAmenities();

    if (room->getStatus() == RoomStatus::BOOKED || room->getStatus() == RoomStatus::OCCUPIED) {
        for (const Booking* b : bookingManager.getActiveBookings()) {
            if (b->getRoomId() == roomId) {
                const Customer* c = customerManager.findByCccd(b->getCccd());
                std::string custName = c ? c->getName() : "(khong ro)";
                oss << "\n  - Booking: " << b->getBookingId()
                    << " (" << bookingStatusToDisplay(b->getStatus()) << ")"
                    << " | Khach: " << custName << " (CCCD: " << b->getCccd() << ")";
                break;
            }
        }
    }
    return oss.str();
}

std::string KeywordChatEngine::handleAvailableRooms(const std::string& roomType) const {
    if (!roomType.empty()) {
        std::vector<const Room*> avail;
        for (const Room* r : roomManager.filterByType(roomType)) {
            if (r->getStatus() == RoomStatus::AVAILABLE) {
                avail.push_back(r);
            }
        }
        if (avail.empty()) {
            return "Hien het phong trong loai " + roomType + ".";
        }
        std::ostringstream oss;
        oss << "Hien con " << avail.size() << " phong trong loai " << roomType << ": ";
        for (std::size_t i = 0; i < avail.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << avail[i]->getId() << " (" << formatMoney(avail[i]->getPricePerDay()) << " VND)";
        }
        return oss.str();
    }

    std::vector<const Room*> allAvail = roomManager.filterByStatus(RoomStatus::AVAILABLE);
    if (allAvail.empty()) {
        return "Hien tai khong con phong trong nao.";
    }

    std::ostringstream oss;
    oss << "Hien co tong cong " << allAvail.size() << " phong trong:\n";
    const char* types[3] = {"SINGLE", "DOUBLE", "VIP"};
    for (const char* t : types) {
        std::vector<std::string> ids;
        for (const Room* r : allAvail) {
            if (r->getType() == t) {
                ids.push_back(r->getId());
            }
        }
        oss << "  - " << t << " (" << ids.size() << "): ";
        if (ids.empty()) {
            oss << "(het)";
        } else {
            for (std::size_t i = 0; i < ids.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << ids[i];
            }
        }
        oss << '\n';
    }
    std::string res = oss.str();
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

std::string KeywordChatEngine::handleOccupiedOrCleaningRooms(const std::string& normalizedText) const {
    if (containsAnyPhrase(normalizedText, {"dang don", "can don", "cho don", "cleaning"})) {
        auto list = roomManager.filterByStatus(RoomStatus::CLEANING);
        if (list.empty()) {
            return "Hien khong co phong nao dang don dep (CLEANING).";
        }
        std::ostringstream oss;
        oss << "Co " << list.size() << " phong dang don (CLEANING): ";
        for (std::size_t i = 0; i < list.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << list[i]->getId();
        }
        return oss.str();
    }

    if (containsAnyPhrase(normalizedText, {"dang dat", "dat truoc", "booked"})) {
        auto list = roomManager.filterByStatus(RoomStatus::BOOKED);
        if (list.empty()) {
            return "Hien khong co phong nao o trang thai Dang dat (BOOKED).";
        }
        std::ostringstream oss;
        oss << "Co " << list.size() << " phong dang dat (BOOKED): ";
        for (std::size_t i = 0; i < list.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << list[i]->getId();
        }
        return oss.str();
    }

    auto list = roomManager.filterByStatus(RoomStatus::OCCUPIED);
    if (list.empty()) {
        return "Hien khong co phong nao dang co khach thue (OCCUPIED).";
    }
    std::ostringstream oss;
    oss << "Co " << list.size() << " phong dang thue (OCCUPIED):\n";
    for (const Room* r : list) {
        oss << "  - " << r->getId() << " (" << r->getType() << ")";
        for (const Booking* b : bookingManager.getActiveBookings()) {
            if (b->getRoomId() == r->getId() && b->getStatus() == BookingStatus::CHECKED_IN) {
                const Customer* c = customerManager.findByCccd(b->getCccd());
                oss << " | Booking: " << b->getBookingId()
                    << " | Khach: " << (c ? c->getName() : b->getCccd());
                break;
            }
        }
        oss << '\n';
    }
    std::string res = oss.str();
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

std::string KeywordChatEngine::handleRoomPrices(const std::string& roomType) const {
    const auto& allRooms = roomManager.getAll();
    if (allRooms.empty()) {
        return "He thong hien chua co phong nao de tra cuu gia.";
    }

    std::vector<std::string> typesToShow;
    if (!roomType.empty()) {
        typesToShow.push_back(roomType);
    } else {
        typesToShow = {"SINGLE", "DOUBLE", "VIP"};
    }

    std::ostringstream oss;
    oss << "Bang gia phong hien tai (VND/ngay):\n";
    for (const std::string& t : typesToShow) {
        auto rooms = roomManager.filterByType(t);
        oss << "  - Loai " << t << ": ";
        if (rooms.empty()) {
            oss << "(chua co phong)\n";
        } else {
            double minP = rooms[0]->getPricePerDay();
            double maxP = rooms[0]->getPricePerDay();
            for (const Room* r : rooms) {
                minP = std::min(minP, r->getPricePerDay());
                maxP = std::max(maxP, r->getPricePerDay());
            }
            if (minP == maxP) {
                oss << formatMoney(minP) << " VND/ngay";
            } else {
                oss << formatMoney(minP) << " - " << formatMoney(maxP) << " VND/ngay";
            }
            oss << " (" << rooms[0]->getAmenities() << ")\n";
        }
    }
    std::string res = oss.str();
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

std::string KeywordChatEngine::handleCustomerLookup(const std::string& normalizedText,
                                                    const std::vector<std::string>& tokens) const {
    std::vector<const Customer*> found;

    // 1. Tim theo CCCD (12 chu so) hoac SDT (10 chu so)
    for (const std::string& tok : tokens) {
        if (isAllDigits(tok) && tok.size() == 12) {
            if (const Customer* c = customerManager.findByCccd(tok)) {
                found.push_back(c);
            }
        } else if (isAllDigits(tok) && tok.size() == 10) {
            if (const Customer* c = customerManager.findByPhone(tok)) {
                found.push_back(c);
            }
        }
    }

    // 2. Neu chua thay, thu trich tu khoa ten khach
    if (found.empty()) {
        const std::vector<std::string> stopWords = {
            "khach", "hang", "ten", "tra", "cuu", "tim", "kiem", "cccd", "sdt",
            "so", "dien", "thoai", "dang", "o", "phong", "nao", "booking", "gi",
            "khong", "co", "cua", "la", "ai", "thue"
        };
        std::string nameQuery;
        for (const std::string& tok : tokens) {
            if (std::find(stopWords.begin(), stopWords.end(), tok) == stopWords.end()) {
                if (!nameQuery.empty()) nameQuery += " ";
                nameQuery += tok;
            }
        }
        if (!nameQuery.empty()) {
            // Tim theo ten (khong phan biet dau / hoa thuong)
            for (const Customer& c : customerManager.getAll()) {
                std::string normName = normalizeVietnamese(c.getName());
                if (normName.find(nameQuery) != std::string::npos) {
                    found.push_back(&c);
                }
            }
        }
    }

    if (found.empty()) {
        (void)normalizedText;
        return "Khong tim thay khach hang phu hop (vui long cung cap ten, CCCD 12 so hoac SDT 10 so).";
    }

    std::ostringstream oss;
    for (std::size_t idx = 0; idx < found.size(); ++idx) {
        const Customer* c = found[idx];
        if (idx > 0) oss << '\n';
        oss << "Khach: " << c->getName() << " | CCCD: " << c->getCccd()
            << " | SDT: " << c->getPhone();

        auto bookings = bookingManager.getByCustomer(c->getCccd());
        if (bookings.empty()) {
            oss << "\n  -> Chua co booking nao.";
        } else {
            for (const Booking* b : bookings) {
                oss << "\n  -> Booking " << b->getBookingId()
                    << " | Phong " << b->getRoomId()
                    << " | Trang thai: " << bookingStatusToDisplay(b->getStatus());
            }
        }
    }
    return oss.str();
}

std::string KeywordChatEngine::handlePromotions(const std::string& normalizedText,
                                                const std::vector<std::string>& tokens) const {
    int quarter = detectQuarter(normalizedText, tokens);
    double pSingle = promotionManager.getDiscount(quarter, "SINGLE");
    double pDouble = promotionManager.getDiscount(quarter, "DOUBLE");
    double pVip = promotionManager.getDiscount(quarter, "VIP");

    std::ostringstream oss;
    oss << "Khuyen mai ap dung cho Quy " << quarter << ":\n"
        << "  - Phong SINGLE: giam " << pSingle << "%\n"
        << "  - Phong DOUBLE: giam " << pDouble << "%\n"
        << "  - Phong VIP   : giam " << pVip << "%";
    return oss.str();
}

std::string KeywordChatEngine::handleRevenue(const std::string& normalizedText,
                                             const std::vector<std::string>& tokens) const {
    int month = 0;
    int year = 0;
    bool hasExplicitMonth = false;
    detectMonthYear(normalizedText, tokens, month, year, hasExplicitMonth);

    auto revenueMap = reportManager.getMonthlyRevenue(year);
    double yearTotal = 0.0;
    for (const auto& kv : revenueMap) {
        yearTotal += kv.second;
    }

    std::ostringstream oss;
    if (!hasExplicitMonth && containsPhrase(normalizedText, "nam")) {
        oss << "Tong doanh thu ca nam " << year << ": " << formatMoney(yearTotal)
            << " VND (tong so hoa don he thong: " << invoiceManager.getAll().size() << ").";
    } else {
        oss << "Doanh thu thang " << month << "/" << year << ": "
            << formatMoney(revenueMap[month]) << " VND "
            << "(Tong ca nam " << year << ": " << formatMoney(yearTotal) << " VND).";
    }
    return oss.str();
}

std::string KeywordChatEngine::handleOccupancy(const std::string& normalizedText,
                                               const std::vector<std::string>& tokens) const {
    int month = 0;
    int year = 0;
    bool hasExplicitMonth = false;
    detectMonthYear(normalizedText, tokens, month, year, hasExplicitMonth);

    auto occ = reportManager.getMonthlyOccupancyRate(year);
    auto vac = reportManager.getMonthlyVacancyRate(year);
    auto nights = reportManager.getMonthlyRoomNights(year);
    CurrentAvailabilityStats cur = reportManager.getCurrentAvailability();

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Thang " << month << "/" << year << ": "
        << "Ty le lap day = " << occ[month] << "% (" << nights[month] << " phong-dem), "
        << "Ty le phong trong = " << vac[month] << "%.\n"
        << "Hien tai dang trong (AVAILABLE): " << cur.availableRooms << "/" << cur.totalRooms
        << " phong (" << cur.availablePercent << "%).";
    return oss.str();
}

std::string KeywordChatEngine::handleServices(const std::string& normalizedText,
                                              const std::vector<std::string>& tokens) const {
    std::string bookingId = detectBookingId(tokens);
    if (!bookingId.empty()) {
        const Booking* b = bookingManager.findBooking(bookingId);
        if (b == nullptr) {
            return "Khong tim thay booking " + bookingId + " de tra cuu dich vu.";
        }
        auto usages = serviceManager.getUsagesByBooking(bookingId);
        if (usages.empty()) {
            return "Booking " + bookingId + " (phong " + b->getRoomId() + ") chua su dung dich vu nao (tong: 0 VND).";
        }
        std::ostringstream oss;
        oss << "Dich vu da dung cua booking " << bookingId << ":\n";
        for (const ServiceUsage* u : usages) {
            oss << "  - " << u->getServiceName() << " x" << u->getQuantity()
                << " (" << formatMoney(u->getUnitPriceAtUse()) << ") = "
                << formatMoney(u->getTotal()) << " VND\n";
        }
        oss << "Tong tien dich vu: " << formatMoney(serviceManager.calcServiceTotal(bookingId)) << " VND";
        return oss.str();
    }

    (void)normalizedText;
    const auto& services = serviceManager.getAll();
    if (services.empty()) {
        return "Danh muc dich vu hien dang trong.";
    }
    std::ostringstream oss;
    oss << "Bang gia dich vu hien hanh:\n";
    for (const Service& s : services) {
        oss << "  - " << s.getServiceId() << ": " << s.getName()
            << " (" << serviceCategoryToDisplay(s.getCategory()) << ") - "
            << formatMoney(s.getUnitPrice()) << " VND\n";
    }
    std::string res = oss.str();
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

std::string KeywordChatEngine::handleFallback() const {
    return "Xin loi, toi chua hieu ro cau hoi nay. Ban co the thu:\n"
           "  - 'con phong trong khong?'\n"
           "  - 'phong P201'\n"
           "  - 'doanh thu thang 11/2026'\n"
           "  - 'help' (de xem tat ca cau hoi ho tro)";
}

std::string KeywordChatEngine::reply(const std::string& question) {
    std::string normalized = normalizeVietnamese(question);
    if (normalized.empty()) {
        return handleFallback();
    }

    std::vector<std::string> tokens = tokenize(normalized);

    // 1. Help
    if (containsAnyPhrase(normalized, {"help", "huong dan", "tro giup", "cu phap", "hoi duoc gi"})) {
        return handleHelp();
    }

    // 2. Goi y phong theo muc gia ("phong duoi 500k")
    double budget = detectBudgetAmount(normalized, tokens);
    if (budget > 0.0 &&
        containsAnyPhrase(normalized, {"duoi", "toi da", "tam", "khoang", "re hon", "goi y", "tu"})) {
        return handleBudgetSuggestion(normalized, tokens);
    }

    // 3. Hoi cu the mot ma phong ("phong P201")
    std::string roomId = detectRoomId(tokens);
    if (!roomId.empty()) {
        return handleSpecificRoom(roomId);
    }

    // 4. Dich vu (bang gia hoac tong dich vu cua 1 booking)
    std::string bookingId = detectBookingId(tokens);
    if (containsPhrase(normalized, "dich vu") ||
        (!bookingId.empty() && containsAnyPhrase(normalized, {"dich vu", "tong", "tien", "su dung"}))) {
        return handleServices(normalized, tokens);
    }

    // 5. Doanh thu theo thang / nam
    if (containsAnyPhrase(normalized, {"doanh thu", "thu nhap", "tien thu"})) {
        return handleRevenue(normalized, tokens);
    }

    // 6. Ty le lap day / phong trong theo thang
    if (containsAnyPhrase(normalized, {"lap day", "cong suat", "ty le"})) {
        return handleOccupancy(normalized, tokens);
    }

    // 7. Khuyen mai theo quy
    if (containsAnyPhrase(normalized, {"khuyen mai", "uu dai", "giam gia", "voucher"})) {
        return handlePromotions(normalized, tokens);
    }

    // 8. Danh sach phong dang thue / dang don / dang dat
    if (containsAnyPhrase(normalized, {"dang thue", "co khach", "dang o", "occupied",
                                       "dang don", "can don", "cho don", "cleaning",
                                       "dang dat", "dat truoc", "booked"})) {
        return handleOccupiedOrCleaningRooms(normalized);
    }

    // 9. Gia phong theo loai
    std::string roomType = detectRoomType(normalized, tokens);
    if (containsPhrase(normalized, "gia") &&
        (containsPhrase(normalized, "phong") || !roomType.empty())) {
        return handleRoomPrices(roomType);
    }

    // 10. Phong trong (tong hoac theo loai)
    if (containsAnyPhrase(normalized, {"con phong", "phong trong", "trong", "available"})) {
        return handleAvailableRooms(roomType);
    }

    // 11. Tra cuu khach hang theo ten / CCCD / SDT
    bool hasIdToken = false;
    for (const std::string& tok : tokens) {
        if (isAllDigits(tok) && (tok.size() == 10 || tok.size() == 12)) {
            hasIdToken = true;
            break;
        }
    }
    if (hasIdToken || containsAnyPhrase(normalized, {"khach", "cccd", "sdt", "so dien thoai", "tra cuu", "tim"})) {
        return handleCustomerLookup(normalized, tokens);
    }

    // 12. Chao hoi don gian
    if (containsAnyPhrase(normalized, {"xin chao", "chao", "hello", "hi"})) {
        return handleGreeting();
    }

    return handleFallback();
}

