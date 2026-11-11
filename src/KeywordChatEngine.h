#ifndef KEYWORD_CHAT_ENGINE_H
#define KEYWORD_CHAT_ENGINE_H

#include "BookingManager.h"
#include "CustomerManager.h"
#include "IChatEngine.h"
#include "InvoiceManager.h"
#include "PromotionManager.h"
#include "ReportManager.h"
#include "RoomManager.h"
#include "ServiceManager.h"
#include <string>
#include <vector>

// Bo may chatbox dua tren luat va tu khoa tieng Viet (co dau hoac khong dau)
class KeywordChatEngine : public IChatEngine {
private:
    const RoomManager& roomManager;
    const CustomerManager& customerManager;
    const BookingManager& bookingManager;
    const ServiceManager& serviceManager;
    const PromotionManager& promotionManager;
    const InvoiceManager& invoiceManager;
    const ReportManager& reportManager;

    // Tien xu ly van ban: bo dau UTF-8, chuyen chu thuong, tach tu
    static std::string normalizeVietnamese(const std::string& text);
    static std::vector<std::string> tokenize(const std::string& normalizedText);
    static bool containsPhrase(const std::string& normalizedText, const std::string& phrase);
    static bool containsAnyPhrase(const std::string& normalizedText,
                                  const std::vector<std::string>& phrases);

    // Cac ham trich xuat tham so tu cau hoi
    static std::string detectRoomType(const std::string& normalizedText,
                                      const std::vector<std::string>& tokens);
    std::string detectRoomId(const std::vector<std::string>& tokens) const;
    static std::string detectBookingId(const std::vector<std::string>& tokens);
    static double detectBudgetAmount(const std::string& normalizedText,
                                     const std::vector<std::string>& tokens);
    static int detectQuarter(const std::string& normalizedText,
                             const std::vector<std::string>& tokens);
    static void detectMonthYear(const std::string& normalizedText,
                                const std::vector<std::string>& tokens,
                                int& outMonth,
                                int& outYear,
                                bool& hasExplicitMonth);

    // Cac ham xu ly tung y dinh (chi doc du lieu tu Manager)
    std::string handleHelp() const;
    std::string handleGreeting() const;
    std::string handleBudgetSuggestion(const std::string& normalizedText,
                                       const std::vector<std::string>& tokens) const;
    std::string handleSpecificRoom(const std::string& roomId) const;
    std::string handleAvailableRooms(const std::string& roomType) const;
    std::string handleOccupiedOrCleaningRooms(const std::string& normalizedText) const;
    std::string handleRoomPrices(const std::string& roomType) const;
    std::string handleCustomerLookup(const std::string& normalizedText,
                                     const std::vector<std::string>& tokens) const;
    std::string handlePromotions(const std::string& normalizedText,
                                 const std::vector<std::string>& tokens) const;
    std::string handleRevenue(const std::string& normalizedText,
                              const std::vector<std::string>& tokens) const;
    std::string handleOccupancy(const std::string& normalizedText,
                                const std::vector<std::string>& tokens) const;
    std::string handleServices(const std::string& normalizedText,
                               const std::vector<std::string>& tokens) const;
    std::string handleFallback() const;

public:
    KeywordChatEngine(const RoomManager& roomManager,
                      const CustomerManager& customerManager,
                      const BookingManager& bookingManager,
                      const ServiceManager& serviceManager,
                      const PromotionManager& promotionManager,
                      const InvoiceManager& invoiceManager,
                      const ReportManager& reportManager);

    std::string reply(const std::string& question) override;
};

#endif // KEYWORD_CHAT_ENGINE_H

