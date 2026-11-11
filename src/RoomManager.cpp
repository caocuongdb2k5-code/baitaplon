#include "RoomManager.h"
#include "DoubleRoom.h"
#include "JsonUtils.h"
#include "SingleRoom.h"
#include "VipRoom.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

std::string RoomManager::normalizeType(const std::string& type) {
    std::string upper = type;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    if (upper != "SINGLE" && upper != "DOUBLE" && upper != "VIP") {
        throw std::invalid_argument("Loai phong khong hop le (chi ho tro SINGLE, DOUBLE, VIP): " + type);
    }
    return upper;
}

std::unique_ptr<Room> RoomManager::createRoom(const std::string& type,
                                              const std::string& id,
                                              double basePrice,
                                              RoomStatus status) {
    std::string normalized = normalizeType(type);
    if (normalized == "SINGLE") {
        return std::make_unique<SingleRoom>(id, basePrice, status);
    }
    if (normalized == "DOUBLE") {
        return std::make_unique<DoubleRoom>(id, basePrice, status);
    }
    return std::make_unique<VipRoom>(id, basePrice, status);
}

Room& RoomManager::requireRoomMutable(const std::string& id) {
    Room* room = findRoom(id);
    if (room == nullptr) {
        throw std::runtime_error("Khong tim thay phong: " + id);
    }
    return *room;
}

void RoomManager::addRoom(const std::string& type, const std::string& id, double basePrice) {
    if (findRoom(id) != nullptr) {
        throw std::runtime_error("Ma phong da ton tai: " + id);
    }
    // Phong moi luon khoi tao voi trang thai AVAILABLE
    rooms.push_back(createRoom(type, id, basePrice, RoomStatus::AVAILABLE));
}

void RoomManager::updateRoom(const std::string& id, double newBasePrice, const std::string& newType) {
    for (auto& room : rooms) {
        if (room->getId() == id) {
            RoomStatus currentStatus = room->getStatus();
            if (currentStatus == RoomStatus::BOOKED || currentStatus == RoomStatus::OCCUPIED) {
                throw std::runtime_error("Khong the sua phong dang dat hoac dang thue: " + id);
            }

            std::string normalizedNewType = normalizeType(newType);
            if (normalizedNewType == room->getType()) {
                room->setBasePrice(newBasePrice);
            } else {
                // Doi loai phong thi tao doi tuong lop con moi thay the, giu nguyen id va status
                room = createRoom(normalizedNewType, id, newBasePrice, currentStatus);
            }
            return;
        }
    }
    throw std::runtime_error("Khong tim thay phong: " + id);
}

void RoomManager::removeRoom(const std::string& id) {
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        if ((*it)->getId() == id) {
            RoomStatus status = (*it)->getStatus();
            if (status == RoomStatus::BOOKED || status == RoomStatus::OCCUPIED) {
                throw std::runtime_error("Khong the xoa phong dang dat hoac dang thue: " + id);
            }
            rooms.erase(it);
            return;
        }
    }
    throw std::runtime_error("Khong tim thay phong: " + id);
}

Room* RoomManager::findRoom(const std::string& id) {
    for (const auto& room : rooms) {
        if (room->getId() == id) {
            return room.get();
        }
    }
    return nullptr;
}

const Room* RoomManager::findRoom(const std::string& id) const {
    for (const auto& room : rooms) {
        if (room->getId() == id) {
            return room.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<Room>>& RoomManager::getAll() const {
    return rooms;
}

std::vector<const Room*> RoomManager::filterByType(const std::string& type) const {
    std::string normalized = normalizeType(type);
    std::vector<const Room*> result;
    for (const auto& room : rooms) {
        if (room->getType() == normalized) {
            result.push_back(room.get());
        }
    }
    return result;
}

std::vector<const Room*> RoomManager::filterByStatus(RoomStatus status) const {
    std::vector<const Room*> result;
    for (const auto& room : rooms) {
        if (room->getStatus() == status) {
            result.push_back(room.get());
        }
    }
    return result;
}

int RoomManager::countAvailable(const std::string& type) const {
    std::string normalized = normalizeType(type);
    int count = 0;
    for (const auto& room : rooms) {
        if (room->getType() == normalized && room->getStatus() == RoomStatus::AVAILABLE) {
            ++count;
        }
    }
    return count;
}

void RoomManager::markBooked(const std::string& id) {
    Room& room = requireRoomMutable(id);
    if (room.getStatus() != RoomStatus::AVAILABLE) {
        throw std::runtime_error("Phong " + id + " khong o trang thai Trong (AVAILABLE) de dat");
    }
    room.setStatus(RoomStatus::BOOKED);
}

void RoomManager::markOccupied(const std::string& id) {
    Room& room = requireRoomMutable(id);
    if (room.getStatus() != RoomStatus::BOOKED) {
        throw std::runtime_error("Phong " + id + " khong o trang thai Dang dat (BOOKED) de check-in");
    }
    room.setStatus(RoomStatus::OCCUPIED);
}

void RoomManager::markCleaning(const std::string& id) {
    Room& room = requireRoomMutable(id);
    if (room.getStatus() != RoomStatus::OCCUPIED) {
        throw std::runtime_error("Phong " + id + " khong o trang thai Dang thue (OCCUPIED) de check-out");
    }
    room.setStatus(RoomStatus::CLEANING);
}

void RoomManager::markAvailable(const std::string& id) {
    Room& room = requireRoomMutable(id);
    RoomStatus current = room.getStatus();
    if (current != RoomStatus::BOOKED && current != RoomStatus::CLEANING) {
        throw std::runtime_error(
            "Chi co the chuyen phong " + id + " ve Trong khi dang BOOKED (huy) hoac CLEANING (don xong)"
        );
    }
    room.setStatus(RoomStatus::AVAILABLE);
}

void RoomManager::loadFromFile(const std::string& path) {
    nlohmann::json arr = JsonUtils::readJsonArray(path);
    rooms.clear();

    for (const auto& item : arr) {
        try {
            if (!item.is_object()) {
                throw std::runtime_error("Phan tu khong phai JSON object");
            }
            std::string id = item.at("id").get<std::string>();
            std::string type = item.at("type").get<std::string>();
            double basePrice = item.at("basePrice").get<double>();
            RoomStatus status = stringToRoomStatus(item.at("status").get<std::string>());

            if (findRoom(id) != nullptr) {
                throw std::runtime_error("Trung ma phong trong file: " + id);
            }
            rooms.push_back(createRoom(type, id, basePrice, status));
        } catch (const nlohmann::json::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 phong loi JSON trong " + path + ": " + ex.what());
        } catch (const std::exception& ex) {
            JsonUtils::logWarning("Bo qua 1 phong khong hop le trong " + path + ": " + ex.what());
        }
    }
}

void RoomManager::saveToFile(const std::string& path) const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& room : rooms) {
        arr.push_back(room->toJson());
    }
    JsonUtils::writeJsonArray(path, arr);
}
