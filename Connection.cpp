#include "Connection.h"

Connection::Connection(){

}

Connection::Connection(int socket, bool is_accepted, struct sockaddr_in addr){
    this->socket = socket ;
    this->is_accepted = is_accepted;
    this->socket_addr = addr;
}

Connection::~Connection(){

}

int Connection::getSocket(){
    return this->socket;
}

bool Connection::isAccepted(){
    return this->is_accepted;
}

struct sockaddr_in Connection::getSocketAddr(){
    return this->socket_addr;
}
