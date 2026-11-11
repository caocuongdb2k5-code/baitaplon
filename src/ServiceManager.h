#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include "BookingManager.h"
#include "Enums.h"
#include "Service.h"
#include "ServiceUsage.h"
#include <string>
#include <vector>

// Lop quan ly danh muc dich vu va cac lan su dung dich vu theo booking
class ServiceManager {
private:
    const BookingManager& bookingManager;
    std::vector<Service> services;
    std::vector<ServiceUsage> usages;
    std::string servicesFilePath;
    std::string usagesFilePath;

    void seedDefaultServices();

public:
    explicit ServiceManager(const BookingManager& bookingManager,
                            const std::string& servicesFilePath = "data/services.json",
                            const std::string& usagesFilePath = "data/service_usage.json");

    // Quan ly danh muc dich vu
    void addService(const std::string& serviceId,
                    const std::string& name,
                    double unitPrice,
                    ServiceCategory category);
    void updateService(const std::string& serviceId,
                       const std::string& newName,
                       double newUnitPrice,
                       ServiceCategory newCategory);
    void removeService(const std::string& serviceId);
    Service* findService(const std::string& serviceId);
    const Service* findService(const std::string& serviceId) const;
    const std::vector<Service>& getAll() const;

    // Quan ly dich vu phat sinh theo booking
    const ServiceUsage& addUsage(const std::string& bookingId,
                                 const std::string& serviceId,
                                 int quantity);
    std::vector<const ServiceUsage*> getUsagesByBooking(const std::string& bookingId) const;
    double calcServiceTotal(const std::string& bookingId) const;

    // Doc va luu JSON
    void loadServicesFromFile(const std::string& path);
    void saveServicesToFile(const std::string& path) const;
    void loadUsagesFromFile(const std::string& path);
    void saveUsagesToFile(const std::string& path) const;
};

#endif // SERVICE_MANAGER_H

