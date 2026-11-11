#include "PromotionManager.h"
#include "JsonUtils.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <stdexcept>
#include <system_error>

PromotionManager::PromotionManager(const std::string& promotionsFilePath)
    : promotionsFilePath(promotionsFilePath) {
}

void PromotionManager::seedDefaultPromotions() {
    promotions.clear();
    promotions.emplace_back(4, "VIP", 10.0, "Uu dai phong VIP quy 4");
}

void PromotionManager::setPromotion(int quarter,
                                    const std::string& roomType,
                                    double percent,
                                    const std::string& description) {
    Promotion newPromo(quarter, roomType, percent, description);
    for (auto& promo : promotions) {
        if (promo == newPromo) {
            promo.setDiscountPercent(newPromo.getDiscountPercent());
            promo.setDescription(newPromo.getDescription());
            saveToFile(promotionsFilePath);
            return;
        }
    }
    promotions.push_back(std::move(newPromo));
    saveToFile(promotionsFilePath);
}

void PromotionManager::removePromotion(int quarter, const std::string& roomType) {
    Promotion::validateQuarter(quarter);
    std::string normalizedType = Promotion::normalizeRoomType(roomType);

    for (auto it = promotions.begin(); it != promotions.end(); ++it) {
        if (it->getQuarter() == quarter && it->getRoomType() == normalizedType) {
            promotions.erase(it);
            saveToFile(promotionsFilePath);
            return;
        }
    }
    throw std::runtime_error("Khong tim thay khuyen mai cho quy va loai phong da chon");
}

const std::vector<Promotion>& PromotionManager::getAll() const {
    return promotions;
}

double PromotionManager::getDiscount(int quarter, const std::string& roomType) const {
    Promotion::validateQuarter(quarter);
    std::string normalizedType = Promotion::normalizeRoomType(roomType);

    double allDiscount = 0.0;
    bool hasAll = false;
    for (const auto& promo : promotions) {
        if (promo.getQuarter() != quarter) {
            continue;
        }
        if (promo.getRoomType() == normalizedType) {
            return promo.getDiscountPercent();
        }
        if (promo.getRoomType() == "ALL") {
            allDiscount = promo.getDiscountPercent();
            hasAll = true;
        }
    }
    return hasAll ? allDiscount : 0.0;
}

void PromotionManager::loadFromFile(const std::string& path) {
    promotionsFilePath = path;
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        seedDefaultPromotions();
        saveToFile(path);
        return;
    }

    nlohmann::json arr = JsonUtils::readJsonArray(path);
    promotions.clear();
    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            Promotion loaded = Promotion::fromJson(item);
            bool replaced = false;
            for (auto& existing : promotions) {
                if (existing == loaded) {
                    existing = loaded;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                promotions.push_back(std::move(loaded));
            }
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 khuyen mai loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 khuyen mai khong hop le trong " + path + ": " + ex.what());
        }
    }
}

void PromotionManager::saveToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& promo : promotions) {
        arr.push_back(promo.toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}

