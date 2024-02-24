#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <arpa/inet.h>

class Connection
{
private:
    int socket;
    bool is_accepted;
    struct sockaddr_in socket_addr;
public:
    Connection();
    Connection(int , bool , struct sockaddr_in);
    ~Connection();
    int getSocket();
    bool isAccepted();
    struct sockaddr_in getSocketAddr();
};


#endif // _CONNECTION_H