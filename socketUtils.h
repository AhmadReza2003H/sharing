#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <cstring>
#include <ifaddrs.h>
#include <sstream>
#include <stdexcept>

#define BROADCAST_IP "255.255.255.255" // Broadcast IP address
#define PORT 8080    // port number
#define BUFFER_SIZE 1024
#define ID_LEN 5
#define BACKLOG_LEN 10

struct MessageDetails{
    int accepted;
    int connected;
    int ID;
};

struct AcceptedSocket {
    int acceptedSocketFD;
    struct sockaddr_in address;
    bool isAccepet;
    int error;
};



void throwReceivingException();
void throwIncompleteReceivingException();
struct AcceptedSocket acceptIncomingConnection(int);
struct MessageDetails getMessageDetails(std::string);
int sendBroadCastMessage(std::string , int , struct sockaddr_in);
struct sockaddr_in getMyAddress();
int createIPV4TCPSocket();
int createIPV4UDPSocket();
int generateRandomID();
void send4Byte(int , int);
void send8Byte(int , long);
void sendNByte(int , char * , long);
int receive4Byte(int);
long receive8Byte(int);
char * receiveNByte(int , long);
bool operator==(const struct sockaddr_in & , const struct sockaddr_in &);


#endif // _STRUCTURES_H_