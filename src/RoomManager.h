#ifndef ROOM_MANAGER_H
#define ROOM_MANAGER_H

#include "Enums.h"
#include "Room.h"
#include <memory>
#include <string>
#include <vector>

// Lop quan ly danh sach phong va luat nghiep vu lien quan den phong
class RoomManager {
private:
    std::vector<std::unique_ptr<Room>> rooms;

    // Factory va ham ho tro noi bo
    static std::string normalizeType(const std::string& type);
    static std::unique_ptr<Room> createRoom(const std::string& type,
                                            const std::string& id,
                                            double basePrice,
                                            RoomStatus status = RoomStatus::AVAILABLE);
    Room& requireRoomMutable(const std::string& id);

public:
    RoomManager() = default;

    // CRUD phong
    void addRoom(const std::string& type, const std::string& id, double basePrice);
    void updateRoom(const std::string& id, double newBasePrice, const std::string& newType);
    void removeRoom(const std::string& id);

    // Tim kiem & loc
    Room* findRoom(const std::string& id);
    const Room* findRoom(const std::string& id) const;
    const std::vector<std::unique_ptr<Room>>& getAll() const;
    std::vector<const Room*> filterByType(const std::string& type) const;
    std::vector<const Room*> filterByStatus(RoomStatus status) const;
    int countAvailable(const std::string& type) const;

    // Chuyen trang thai phong an toan theo vong doi nghiep vu
    void markBooked(const std::string& id);
    void markOccupied(const std::string& id);
    void markCleaning(const std::string& id);
    void markAvailable(const std::string& id);

    // Doc va luu danh sach phong qua file JSON
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
};

#endif // ROOM_MANAGER_H
