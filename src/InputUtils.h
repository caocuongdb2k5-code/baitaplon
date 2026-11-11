#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <string>

// Tien ich nhap du lieu an toan tren terminal (khong lap vo han khi nhap sai)
namespace InputUtils {

int readInt(const std::string& prompt, int minVal, int maxVal);
double readDouble(const std::string& prompt, double minVal, double maxVal);
std::string readNonEmptyString(const std::string& prompt);
std::string readDate(const std::string& prompt);
std::string readDateTime(const std::string& prompt);
bool readYesNo(const std::string& prompt);
void waitForEnter(const std::string& message = "Nhan Enter de tiep tuc...");

} // namespace InputUtils

#endif // INPUT_UTILS_H

