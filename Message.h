#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include "string"
#include <arpa/inet.h>


class Message{
private:
    std::string message;
    bool is_valid;
    struct sockaddr_in sender_addr;
public:
    Message();
    Message(std::string , bool , struct sockaddr_in);
    ~Message();
    std::string getMessage();
    bool isValid();
    struct sockaddr_in getSenderAddr();
};


#endif // _MESSAGE_H_