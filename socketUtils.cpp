#include "socketUtils.h"

void throwReceivingException(){
    throw std::runtime_error("Error receiving data from socket");
}

void throwIncompleteReceivingException(){
    throw std::runtime_error("Error receiving incomplete data from socket");
}


struct AcceptedSocket acceptIncomingConnection(int serverSocketFD){
    // Accept new client
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocketFD = accept(serverSocketFD ,(struct sockaddr*) &clientAddress , &clientLen);


    struct AcceptedSocket acceptedSocket;
    acceptedSocket.address = clientAddress;
    acceptedSocket.acceptedSocketFD = clientSocketFD;
    acceptedSocket.isAccepet = clientSocketFD > 0;

    if(!acceptedSocket.isAccepet){
        acceptedSocket.error = clientSocketFD;
    }
    return acceptedSocket;
}

int sendBroadCastMessage(std::string message, int udp_socket, struct sockaddr_in broadcast_addr){
    return sendto(udp_socket, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
}

struct sockaddr_in getMyAddress(){
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    struct sockaddr_in my_ip_addr;
    
    char *addr;

    if (getifaddrs(&ifap) == -1) {
        std::cerr << "Error: Couldn't get network interface information.\n";
        EXIT_FAILURE;
    }

    for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != nullptr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            // Exclude loopback interface and get the first non-loopback IP address
            if (std::strcmp(ifa->ifa_name, "lo") != 0) {
                my_ip_addr = *sa;
                freeifaddrs(ifap);
                return my_ip_addr;
            }
        }
    }
    return my_ip_addr;
}

int createIPV4TCPSocket(){
    return socket(AF_INET, SOCK_STREAM, 0);
}
int createIPV4UDPSocket(){
    return socket(AF_INET, SOCK_DGRAM, 0);
}


int generateRandomID(){
    int ID = (std::rand() % 9) + 1;
    for(int i = 0 ; i < ID_LEN - 1 ; i++){
        ID = ID*10 + (std::rand() & 10);
    }
    return ID;
}

struct MessageDetails getMessageDetails(std::string message){
    MessageDetails result;
   // Create a string stream from the input string
    std::stringstream ss(message);

    // Vector to store parsed integers
    std::vector<int> numbers;

    // Temporary string to store each token
    std::string token;

    // Parse the string separated by commas
    while (std::getline(ss, token, ',')) {
        // Convert the token to an integer and add it to the vector
        numbers.push_back(std::stoi(token));
    }

    result.accepted = numbers.at(0);
    result.connected = numbers.at(1);
    result.ID = numbers.at(2);

    
    return result;
}

bool operator==(const struct sockaddr_in & first, const struct sockaddr_in & seccond){
    return first.sin_addr.s_addr == seccond.sin_addr.s_addr;
}

void send4Byte(int socketFD, int number){
    int32_t number_network_order = htonl(number);
    if(send(socketFD , &number_network_order , sizeof(int) , 0) == -1){
        throw std::runtime_error("Error send data to socket");
    }
}

void send8Byte(int socketFD, long number){
    int64_t number_network_order =  static_cast<int64_t>(htonl(static_cast<uint32_t>(number >> 32))) << 32 |
                     htonl(static_cast<uint32_t>(number & 0xFFFFFFFF));
    if(send(socketFD , &number_network_order , sizeof(long) , 0) == -1){
        throw std::runtime_error("Error send data to socket");
    }
}

void sendNByte(int socketFD, char * text , long text_length){
    if(send(socketFD , text , text_length , 0) == -1){
        throw std::runtime_error("Error send data to socket");
    }
}

int receive4Byte(int socketFD){

    int32_t receive_number_network_order;
    ssize_t amountReceive = recv(socketFD, &receive_number_network_order , sizeof(int) , 0);

    if(amountReceive <= 0){
        throwReceivingException();
    }
    return ntohl(receive_number_network_order);

}
long receive8Byte(int socketFD){

    int64_t receive_number_network_order;
    ssize_t amountReceive = recv(socketFD, &receive_number_network_order , sizeof(long) , 0);

    if(amountReceive <= 0){
        throwReceivingException();
    }

    // Convert received int64_t value from network byte order to host byte order
    uint32_t high_part = ntohl(static_cast<uint32_t>(receive_number_network_order >> 32));
    uint32_t low_part = ntohl(static_cast<uint32_t>(receive_number_network_order & 0xFFFFFFFF));
    int64_t receive_number_host_order = (static_cast<int64_t>(high_part) << 32) | low_part;

    // Cast the 64-bit integer to long and return
    return static_cast<long>(receive_number_host_order);    
}

char * receiveNByte(int socketFD, long length){
    char * name = new char[length];
    size_t amountReceive = recv(socketFD , name , length , 0);
    if(amountReceive > 0){
        if(amountReceive != length){
            delete[] name;
            throwIncompleteReceivingException();
        }
    } else {
        delete[] name;
        throwReceivingException();
    }
    return name;
}
