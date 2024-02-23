#ifndef _NETWORKARGS_H_
#define _NETWORKARGS_H_

#include "Connection.h"
#include "Message.h"
#include "fileStreamUtils.h"
#include "SocketFile.h"

class NetworkArgs{
private:
    int udp_socket;
    int tcp_socket;
    int ID;
    int connected;
    int accepted;
    struct sockaddr_in my_addr;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in udp_socket_addr;
    struct sockaddr_in tcp_socket_addr;
    std::vector<Connection *> * connections;
    std::vector<SocketFile *> * socket_files;

    void enableAddressReuse();
    void enableBroadcastOption();
    void setUpAddresses();
    void bindSockets();
    void listenSockets();
    void closeSockets();
    
public:
    NetworkArgs();
    ~NetworkArgs();
    int getUdpSocket();
    int getTCPSocket();
    int getID();
    int getConnected();
    int getAccepted();
    struct sockaddr_in getBroadcastAddr();
    struct sockaddr_in getUDPAddr();
    struct sockaddr_in getTCPAddr();
    std::vector<Connection *> * getConnections();
    std::vector<SocketFile *> * getSocketFiles();
    void addConnection(Connection *);
    void addConnection(int , bool ,struct sockaddr_in);
    void addSocketFile(SocketFile *);
    void deleteConnection(Connection *);
    void deleteConnection(struct sockaddr_in);
    void sendTCPMessage(std::string);
    bool isConnected(struct sockaddr_in);
    int sendBroadcastMessage();
    std::string getMessage();
    Message getBroadcatMessage();
    Connection * cheackBroadcastMessage(Message);
    Connection * makeConnection(Message);
    Connection * connectToAddress(struct sockaddr_in);
    SocketFile * getSocketFile(std::string);
};



#endif // _NETWORKARGS_H_