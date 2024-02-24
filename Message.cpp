#include "Message.h"

Message::Message(){
    this->is_valid = false;
}

Message::Message(std::string message , bool is_valid , struct sockaddr_in sender_addr){
    this->message = message;
    this->is_valid = is_valid;
    this->sender_addr = sender_addr;
}

Message::~Message(){

}

std::string Message::getMessage(){
    return this->message;
}

bool Message::isValid(){
    return this->is_valid;
}

struct sockaddr_in Message::getSenderAddr(){
    return this->sender_addr;
}