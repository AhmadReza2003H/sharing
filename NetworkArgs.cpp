#include "NetworkArgs.h"

NetworkArgs::NetworkArgs(){
    this->tcp_socket = createIPV4TCPSocket();
    this->udp_socket = createIPV4UDPSocket();
    this->connected = 0;
    this->accepted = 0;
    this->ID = generateRandomID();
    this->enableAddressReuse();
    this->enableBroadcastOption();
    this->setUpAddresses();
    this->bindSockets();
    this->listenSockets();
    this->connections = new std::vector<Connection *>();
}

NetworkArgs::~NetworkArgs(){
    // Delete connections
    for(auto it = this->connections->begin(); it != this->connections->end();it++){
        delete *it;
    }
    // Delete connections vector
    delete this->connections;
    // Close socket
    this->closeSockets();
}

void NetworkArgs::enableAddressReuse(){
    // Enable address reuse
    int reuse_addr = 1;
    if (setsockopt(this->udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        perror("setsockopt");
        closeSockets();
        exit(EXIT_FAILURE);
    }
    if (setsockopt(this->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        perror("setsockopt");
        closeSockets();
        exit(EXIT_FAILURE);
    }
}

void NetworkArgs::enableBroadcastOption(){
    // Enable broadcast option
    int broadcast_enable = 1;
    if (setsockopt(this->udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
        perror("setsockopt");
        closeSockets();
        exit(EXIT_FAILURE);
    }
}

void NetworkArgs::setUpAddresses(){
    // Get my address
    this->my_addr = getMyAddress();

    // Set up the UDP address structure
    memset(&this->udp_socket_addr, 0, sizeof(this->udp_socket_addr));
    this->udp_socket_addr.sin_family = AF_INET;
    this->udp_socket_addr.sin_port = htons(PORT);
    this->udp_socket_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // Set up the broadcast address structure
    memset(&this->broadcast_addr, 0, sizeof(this->broadcast_addr));
    this->broadcast_addr.sin_family = AF_INET;
    this->broadcast_addr.sin_port = htons(PORT);
    this->broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    // Set up the tcp server address structure
    memset(&this->tcp_socket_addr , 0 , sizeof(this->tcp_socket_addr));
    this->tcp_socket_addr.sin_family = AF_INET;
    this->tcp_socket_addr.sin_port = htons(PORT);
    this->tcp_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

void NetworkArgs::bindSockets(){
    // Bind the udp socket to the broadcast address
    if (bind(this->udp_socket, (struct sockaddr *)&this->udp_socket_addr, sizeof(this->udp_socket_addr)) == -1) {
        perror("udp bind");
        this->closeSockets();
        exit(EXIT_FAILURE);
    }

    // Bind the tcp socket to all available network
    if(bind(this->tcp_socket , (struct sockaddr *)&this->tcp_socket_addr , sizeof(this->tcp_socket_addr)) == -1){
        perror("tcp bind");
        this->closeSockets();
        exit(EXIT_FAILURE);
    }


}

void NetworkArgs::listenSockets(){
    // Allowing TCP socket to accept incoming connections
    if(listen(this->tcp_socket , BACKLOG_LEN) == -1 ){
        perror("listen");
        this->closeSockets();
        exit(EXIT_FAILURE);
    }
}

void NetworkArgs::closeSockets(){
    close(this->tcp_socket);
    close(this->udp_socket);
}

int NetworkArgs::getUdpSocket(){
    return this->udp_socket;
}

int NetworkArgs::getTCPSocket(){
    return this->tcp_socket;
}

int NetworkArgs::getID(){
    return this->ID;
}

int NetworkArgs::getConnected(){
    return this->connected;
}

int NetworkArgs::getAccepted(){
    return this->accepted;
}

struct sockaddr_in NetworkArgs::getBroadcastAddr(){
    return this->broadcast_addr;
}

struct sockaddr_in NetworkArgs::getUDPAddr(){
    return this->udp_socket_addr;
}

struct sockaddr_in NetworkArgs::getTCPAddr(){
    return this->tcp_socket_addr;
}

std::vector<Connection * > * NetworkArgs::getConnections(){
    return this->connections;
}

void NetworkArgs::addConnection(Connection * connection){
    this->connections->push_back(connection);
    if(connection->isAccepted()) {
        this->accepted++;
    } else {
        this->connected++;
    }
}

void NetworkArgs::addConnection(int socket , bool isAccepted ,  struct sockaddr_in address){
    Connection * connection = new Connection(socket , isAccepted , address);
    this->connections->push_back(connection);
}


void NetworkArgs::deleteConnection(Connection * connection){
    for(auto it = this->connections->begin() ; it != this->connections->end() ; it++){
        if(*it == connection){
            this->connections->erase(it);
            break;
        }
    }
    if(connection->isAccepted()) {
        this->accepted--;
    } else {
        this->connected--;
    }
    delete connection;
}

void NetworkArgs::deleteConnection(struct sockaddr_in addr){
    Connection * connection = nullptr;
    for(auto it = this->connections->begin() ; it != this->connections->end() ; it++){
        if((*it)->getSocketAddr().sin_addr.s_addr == addr.sin_addr.s_addr){
            this->connections->erase(it);
            connection = *it;
            break;
        }
    }

    if(connection){
        if(connection->isAccepted()) {
            this->accepted--;
        } else {
            this->connected--;
        }
        delete connection;
    }
}

bool NetworkArgs::isConnected(struct sockaddr_in addr){
    for(auto it = this->connections->begin() ; it != this->connections->end() ; it++){
        if((*it)->getSocketAddr().sin_addr.s_addr == addr.sin_addr.s_addr){
            return true;
        }
    }
    return false;
}

int NetworkArgs::sendBroadcastMessage(){
    std::string message = this->getMessage();
    return sendto(this->udp_socket, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr *)&this->broadcast_addr, sizeof(this->broadcast_addr));
}

std::string NetworkArgs::getMessage(){
    std::string message = "";
    message += std::to_string(this->accepted) + ",";
    message += std::to_string(this->connected) + ",";
    message += std::to_string(this->ID);
    return message;
}

Message NetworkArgs::getBroadcatMessage(){
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    char buffer[BUFFER_SIZE];
    int recv_len = recvfrom(this->udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);

    if (recv_len == -1) {
        return Message();
    }

    buffer[recv_len] = 0 ; // NULL termination
    Message result(std::string(buffer) , true , sender_addr);
    return result;
}

Connection * NetworkArgs::cheackBroadcastMessage(Message message){

    if(message.isValid()){
        if(!(this->my_addr == message.getSenderAddr())){
            if(!this->isConnected(message.getSenderAddr())){
                return this->makeConnection(message);
            }
        }
    }
    return nullptr;
}

Connection * NetworkArgs::makeConnection(Message message){
    MessageDetails message_details = getMessageDetails(message.getMessage());
    // Hand shaking
    bool help = (this->ID < message_details.ID) || (this->ID == message_details.ID && this->my_addr.sin_addr.s_addr < message.getSenderAddr().sin_addr.s_addr);
    if(message_details.accepted <= message_details.connected && this->connected < this->accepted){
        return connectToAddress(message.getSenderAddr());
    }

    else if(message_details.connected == message_details.accepted && this->accepted == this->connected && help){
        return connectToAddress(message.getSenderAddr());
    }

    else if(message_details.accepted < message_details.connected && this->accepted == this->connected){
        return connectToAddress(message.getSenderAddr());
    }

    else if(message_details.accepted < message_details.connected && this->accepted < this->connected){
        int sender_ac_delta = message_details.connected - message_details.accepted;
        int my_ac_delta = this->connected - this->accepted;
        if(my_ac_delta < sender_ac_delta || (my_ac_delta == sender_ac_delta && help)){
            return connectToAddress(message.getSenderAddr());
        }
    }

    else if(message_details.connected < message_details.accepted && this->connected < this->accepted){
        int sender_ac_delta = message_details.accepted - message_details.connected;
        int my_ac_delta = this->accepted - this->connected;
        if(sender_ac_delta < my_ac_delta || (sender_ac_delta < my_ac_delta && help)){
            return connectToAddress(message.getSenderAddr());
        }
    }
     return nullptr;
}

Connection * NetworkArgs::connectToAddress(struct sockaddr_in addr){
    int socket = createIPV4TCPSocket();
    connect(socket , (struct sockaddr *)&addr , sizeof(addr));
    Connection * connection = new Connection(socket , false , addr);
    this->connections->push_back(connection);    
    return connection;
}

void NetworkArgs::sendTCPMessage(std::string message){
    for(auto it = this->connections->begin() ; it != this->connections->end() ; it++){
        int socket = (*it)->getSocket();
        send4Byte(socket , FILE_EXIST_CODE);
        send4Byte(socket , message.length());
        send(socket , message.c_str() , message.length() , 0);
    }
}