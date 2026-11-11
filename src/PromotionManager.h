#ifndef PROMOTION_MANAGER_H
#define PROMOTION_MANAGER_H

#include "Promotion.h"
#include <string>
#include <vector>

// Lop quan ly khuyen mai theo quy va loai phong
class PromotionManager {
private:
    std::vector<Promotion> promotions;
    std::string promotionsFilePath;

    void seedDefaultPromotions();

public:
    explicit PromotionManager(const std::string& promotionsFilePath = "data/promotions.json");

    // Them moi hoac ghi de khuyen mai cung (quy, loai phong)
    void setPromotion(int quarter,
                      const std::string& roomType,
                      double percent,
                      const std::string& description);
    void removePromotion(int quarter, const std::string& roomType);
    const std::vector<Promotion>& getAll() const;

    // Tra cuu muc giam gia (%): uu tien dung loai phong -> ALL -> 0
    double getDiscount(int quarter, const std::string& roomType) const;

    // Doc va luu JSON
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
};

#endif // PROMOTION_MANAGER_H

