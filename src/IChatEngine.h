#ifndef I_CHAT_ENGINE_H
#define I_CHAT_ENGINE_H

#include <string>

// Interface cho bo may tra loi chatbox noi bo (chi doc du lieu)
class IChatEngine {
public:
    virtual ~IChatEngine() = default;
    virtual std::string reply(const std::string& question) = 0;
};

#endif // I_CHAT_ENGINE_H

