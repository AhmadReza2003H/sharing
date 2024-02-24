#include "socketUtils.h"
#include "NetworkArgs.h"
#include "fileStreamUtils.h"
#include <ctime>

#define EXIT "exit"

struct Handler{
    Connection * connection;
    NetworkArgs * networkArgs;
};

void * sendBroadcastMessage(void *);
void * recieveBroadcastMessage(void *);
void * recieveTCPMessage(void *);
void * acceptClients(void *);
void sendMessageTCP(struct NetworkArgs *);
void handleNewConnection(NetworkArgs * , Connection *);
void downloadNewFile(NetworkArgs * , std::string);

int main(){
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    NetworkArgs networkArgs;
    pthread_t recieve_udp_thread, send_udp_thread , accept_thread;
    pthread_mutex_t mutex;
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_create(&accept_thread , NULL , acceptClients , (void *)&networkArgs);
    pthread_create(&send_udp_thread, NULL, sendBroadcastMessage, (void *)&networkArgs);
    pthread_create(&recieve_udp_thread, NULL,recieveBroadcastMessage ,(void *)&networkArgs);
    sendMessageTCP(&networkArgs);

    return 0;
}

void * sendBroadcastMessage(void * arg){
    NetworkArgs* networkArgs = (( NetworkArgs *)arg);

    while (true) {
        // Create broadcast message (num of accepted , num of connected , ID) and send it
        networkArgs->sendBroadcastMessage();
        usleep(500000);

    }

    return NULL;
}

void * recieveBroadcastMessage(void * arg){
    NetworkArgs * networkArgs = ((NetworkArgs *)arg);
    while (true) {
        // Get broadcast message
        Message message = networkArgs->getBroadcatMessage();

        // Check message
        Connection * connection = networkArgs->cheackBroadcastMessage(message);
        if(connection){
            handleNewConnection(networkArgs , connection);
        }
    }

    return NULL;
}

void * acceptClients(void * arg){
    NetworkArgs * networkArgs = ((NetworkArgs *)arg);
    while (true){
        AcceptedSocket acceptedClient = acceptIncomingConnection(networkArgs->getTCPSocket());
        if(acceptedClient.isAccepet){
            Connection *connection = new Connection(acceptedClient.acceptedSocketFD , true , acceptedClient.address);
            networkArgs->addConnection(connection);
            handleNewConnection(networkArgs , connection);
        }
    }
    
    return NULL;
}

void * recieveTCPMessage(void * arg){
    struct Handler * handler = ((struct Handler *)arg);

    printf("user with IP:%s joined the network\n",inet_ntoa(handler->connection->getSocketAddr().sin_addr));
    int socketFD = handler->connection->getSocket();
    while(true){
        try{
            int code = receive4Byte(socketFD);
            if(code == FILE_EXIST_CODE){ // Code for cheking file is exist
                answerToCheckRequest(socketFD);
            } else if(code == ANSWER_TO_EXIST_CODE){ // Code for answer to file exist
                responseToFileIsExist(socketFD , handler->networkArgs);
            } else if(code == SEND_FILE_CODE){ // Code for send some part of file
                sendFileToSocket(socketFD);
            } else if(code == RECEIVE_FILE_CODE){ // Code for recieve part of file
                receiveFileFromSocket(socketFD , handler->networkArgs);
            } else if(code == 5){ // Code for

            } else {
                printf("some thing went wrong in recieving code!!\n");
                break;
            }
    
        } catch(...){
            break;
        }
    }

    printf("user with IP:%s exit from network\n",inet_ntoa(handler->connection->getSocketAddr().sin_addr));

    // Erase drop connection
    handler->networkArgs->deleteConnection(handler->connection);
    delete handler;

    return NULL;
}

void sendMessageTCP(NetworkArgs * netWorkArgs) {
    // Create a loop for sending 
    std::string line;
    while (true){
        getline(std::cin , line);
        if(line == EXIT){
            break;
        }
        downloadNewFile(netWorkArgs , line);
    }
}


void handleNewConnection(NetworkArgs * networkArgs, Connection * connection){
    // Create new thread for handling new connection
    struct Handler * handler = new Handler();
    handler->connection = connection;
    handler->networkArgs = networkArgs;
    pthread_t th;
    pthread_create(&th , NULL , recieveTCPMessage , (void *)handler);    
}

void downloadNewFile(NetworkArgs * networkArgs, std::string file_name){
    // Checking file is not downloaded or in a download phaze
    if(networkArgs->getSocketFile(file_name) == NULL){
        // Create new socket file
        SocketFile * socket_file = new SocketFile(file_name);
         networkArgs->addSocketFile(socket_file);
        // Send need message to all connections
        std::vector<Connection *> * connectios = networkArgs->getConnections();
        for(auto it = connectios->begin() ; it != connectios->end() ; it++){
            sendNeedMessage((*it)->getSocket() , file_name);
        }
    }
}
