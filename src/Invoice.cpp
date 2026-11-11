#include "Invoice.h"
#include "TimeUtils.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

bool isBlank(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

void formatInvoiceBody(std::ostringstream& oss,
                       const Invoice& inv,
                       const std::vector<const ServiceUsage*>& usages) {
    oss << std::fixed << std::setprecision(0);
    oss << "====================================================================\n";
    oss << "                        HOA DON THANH TOAN                          \n";
    oss << "====================================================================\n";
    oss << std::left << std::setw(20) << "Ma hoa don:" << inv.getInvoiceId() << '\n';
    oss << std::left << std::setw(20) << "Ma booking:" << inv.getBookingId() << '\n';
    oss << std::left << std::setw(20) << "Ngay xuat:" << inv.getIssuedAt() << '\n';
    oss << "--------------------------------------------------------------------\n";
    oss << std::left << std::setw(20) << "Khach hang:" << inv.getCustomerName() << '\n';
    oss << std::left << std::setw(20) << "CCCD:" << inv.getCccd() << '\n';
    oss << std::left << std::setw(20) << "Phong:"
        << inv.getRoomId() << " (" << inv.getRoomType() << ")\n";
    oss << std::left << std::setw(20) << "Check-in:" << inv.getCheckIn() << '\n';
    oss << std::left << std::setw(20) << "Check-out:" << inv.getCheckOut() << '\n';
    oss << "--------------------------------------------------------------------\n";

    oss << std::left << std::setw(28) << "Hang muc"
        << std::right << std::setw(8) << "SL"
        << std::right << std::setw(15) << "Don gia"
        << std::right << std::setw(17) << "Thanh tien\n";
    oss << "--------------------------------------------------------------------\n";

    std::string roomLabel = "Tien phong (" + std::to_string(inv.getDays()) + " ngay)";
    oss << std::left << std::setw(28) << roomLabel
        << std::right << std::setw(8) << inv.getDays()
        << std::right << std::setw(15) << inv.getPricePerDay()
        << std::right << std::setw(17) << inv.getRoomCost() << '\n';

    if (usages.empty()) {
        if (inv.getServiceCost() > 0.0) {
            oss << std::left << std::setw(28) << "Dich vu phat sinh"
                << std::right << std::setw(8) << "-"
                << std::right << std::setw(15) << "-"
                << std::right << std::setw(17) << inv.getServiceCost() << '\n';
        } else {
            oss << std::left << std::setw(28) << "(Khong su dung dich vu)" << '\n';
        }
    } else {
        for (const ServiceUsage* usage : usages) {
            std::string name = usage->getServiceName();
            if (name.size() > 26) {
                name = name.substr(0, 23) + "...";
            }
            oss << std::left << std::setw(28) << name
                << std::right << std::setw(8) << usage->getQuantity()
                << std::right << std::setw(15) << usage->getUnitPriceAtUse()
                << std::right << std::setw(17) << usage->getTotal() << '\n';
        }
    }

    oss << "--------------------------------------------------------------------\n";
    oss << std::left << std::setw(42) << "Tong tien phong:"
        << std::right << std::setw(22) << inv.getRoomCost() << " VND\n";
    oss << std::left << std::setw(42) << "Tong tien dich vu:"
        << std::right << std::setw(22) << inv.getServiceCost() << " VND\n";

    std::ostringstream discountLabel;
    discountLabel << "Giam gia khuyen mai (" << std::fixed << std::setprecision(1)
                  << inv.getDiscountPercent() << "%):";
    oss << std::left << std::setw(42) << discountLabel.str()
        << std::right << std::setw(22) << (-inv.getDiscountAmount()) << " VND\n";
    oss << "====================================================================\n";
    oss << std::left << std::setw(42) << "TONG THANH TOAN:"
        << std::right << std::setw(22) << inv.getTotal() << " VND\n";
    oss << "====================================================================\n";
}

} // namespace

Invoice::Invoice(const std::string& invoiceId,
                 const std::string& bookingId,
                 const std::string& cccd,
                 const std::string& customerName,
                 const std::string& roomId,
                 const std::string& roomType,
                 double pricePerDay,
                 const std::string& checkIn,
                 const std::string& checkOut,
                 int days,
                 double roomCost,
                 double serviceCost,
                 double discountPercent,
                 double discountAmount,
                 double total,
                 const std::string& issuedAt)
    : invoiceId(invoiceId),
      bookingId(bookingId),
      cccd(cccd),
      customerName(customerName),
      roomId(roomId),
      roomType(roomType),
      pricePerDay(pricePerDay),
      checkIn(checkIn),
      checkOut(checkOut),
      days(days),
      roomCost(roomCost),
      serviceCost(serviceCost),
      discountPercent(discountPercent),
      discountAmount(discountAmount),
      total(total),
      issuedAt(issuedAt) {
    if (invoiceId.empty() || isBlank(invoiceId)) {
        throw std::invalid_argument("Ma hoa don khong duoc de trong");
    }
    if (bookingId.empty() || isBlank(bookingId)) {
        throw std::invalid_argument("Ma booking trong hoa don khong duoc de trong");
    }
    if (days < 1) {
        throw std::invalid_argument("So ngay thue trong hoa don phai >= 1");
    }
    if (pricePerDay <= 0.0 || roomCost < 0.0 || serviceCost < 0.0 ||
        discountPercent < 0.0 || discountPercent > 100.0 ||
        discountAmount < 0.0 || total < 0.0) {
        throw std::invalid_argument("Gia tri tien trong hoa don khong hop le");
    }
    if (!TimeUtils::isValidTimeString(checkIn) ||
        !TimeUtils::isValidTimeString(checkOut) ||
        !TimeUtils::isValidTimeString(issuedAt)) {
        throw std::invalid_argument("Moc thoi gian trong hoa don khong hop le");
    }
}

const std::string& Invoice::getInvoiceId() const { return invoiceId; }
const std::string& Invoice::getBookingId() const { return bookingId; }
const std::string& Invoice::getCccd() const { return cccd; }
const std::string& Invoice::getCustomerName() const { return customerName; }
const std::string& Invoice::getRoomId() const { return roomId; }
const std::string& Invoice::getRoomType() const { return roomType; }
double Invoice::getPricePerDay() const { return pricePerDay; }
const std::string& Invoice::getCheckIn() const { return checkIn; }
const std::string& Invoice::getCheckOut() const { return checkOut; }
int Invoice::getDays() const { return days; }
double Invoice::getRoomCost() const { return roomCost; }
double Invoice::getServiceCost() const { return serviceCost; }
double Invoice::getDiscountPercent() const { return discountPercent; }
double Invoice::getDiscountAmount() const { return discountAmount; }
double Invoice::getTotal() const { return total; }
const std::string& Invoice::getIssuedAt() const { return issuedAt; }

void Invoice::setServiceDetails(const std::vector<ServiceUsage>& details) {
    serviceDetails = details;
}

const std::vector<ServiceUsage>& Invoice::getServiceDetails() const {
    return serviceDetails;
}

void Invoice::print(std::ostream& os) const {
    std::vector<const ServiceUsage*> ptrs;
    ptrs.reserve(serviceDetails.size());
    for (const auto& item : serviceDetails) {
        ptrs.push_back(&item);
    }
    print(os, ptrs);
}

void Invoice::print(std::ostream& os, const std::vector<const ServiceUsage*>& usages) const {
    std::ostringstream oss;
    formatInvoiceBody(oss, *this, usages);
    os << oss.str();
}

nlohmann::json Invoice::toJson() const {
    return nlohmann::json{
        {"invoiceId", invoiceId},
        {"bookingId", bookingId},
        {"cccd", cccd},
        {"customerName", customerName},
        {"roomId", roomId},
        {"roomType", roomType},
        {"pricePerDay", pricePerDay},
        {"checkIn", checkIn},
        {"checkOut", checkOut},
        {"days", days},
        {"roomCost", roomCost},
        {"serviceCost", serviceCost},
        {"discountPercent", discountPercent},
        {"discountAmount", discountAmount},
        {"total", total},
        {"issuedAt", issuedAt}
    };
}

Invoice Invoice::fromJson(const nlohmann::json& j) {
    return Invoice(
        j.at("invoiceId").get<std::string>(),
        j.at("bookingId").get<std::string>(),
        j.at("cccd").get<std::string>(),
        j.at("customerName").get<std::string>(),
        j.at("roomId").get<std::string>(),
        j.at("roomType").get<std::string>(),
        j.at("pricePerDay").get<double>(),
        j.at("checkIn").get<std::string>(),
        j.at("checkOut").get<std::string>(),
        j.at("days").get<int>(),
        j.at("roomCost").get<double>(),
        j.at("serviceCost").get<double>(),
        j.at("discountPercent").get<double>(),
        j.at("discountAmount").get<double>(),
        j.at("total").get<double>(),
        j.at("issuedAt").get<std::string>()
    );
}

bool Invoice::operator==(const Invoice& other) const {
    return invoiceId == other.invoiceId;
}

std::ostream& operator<<(std::ostream& os, const Invoice& invoice) {
    invoice.print(os);
    return os;
}

