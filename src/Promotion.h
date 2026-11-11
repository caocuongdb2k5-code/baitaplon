#ifndef PROMOTION_H
#define PROMOTION_H

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// Lop dai dien cho chuong trinh khuyen mai theo quy va loai phong
class Promotion {
private:
    int quarter;
    std::string roomType;
    double discountPercent;
    std::string description;

public:
    Promotion(int quarter,
              const std::string& roomType,
              double discountPercent,
              const std::string& description);

    // Getter
    int getQuarter() const;
    const std::string& getRoomType() const;
    double getDiscountPercent() const;
    const std::string& getDescription() const;

    // Setter co kiem tra du lieu hop le
    void setDiscountPercent(double discountPercent);
    void setDescription(const std::string& description);

    // Chuan hoa va kiem tra quy, loai phong, phan tram
    static void validateQuarter(int quarter);
    static std::string normalizeRoomType(const std::string& roomType);
    static void validateDiscountPercent(double discountPercent);

    std::string toString() const;
    nlohmann::json toJson() const;
    static Promotion fromJson(const nlohmann::json& j);

    bool operator==(const Promotion& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Promotion& promotion);
};

#endif // PROMOTION_H

