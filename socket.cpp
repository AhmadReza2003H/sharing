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
void * renderNetwork(void *);
void sendMessageTCP(struct NetworkArgs *);
void handleNewConnection(NetworkArgs * , Connection *);
void downloadNewFile(NetworkArgs * , std::string);


int main(){
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    createFilesDirectory();
    NetworkArgs networkArgs;
    networkArgs.loadDownloadDetails();
    pthread_t recieve_udp_thread, send_udp_thread , accept_thread , render_thread;
    pthread_create(&accept_thread , NULL , acceptClients , (void *)&networkArgs);
    pthread_create(&send_udp_thread, NULL, sendBroadcastMessage, (void *)&networkArgs);
    pthread_create(&recieve_udp_thread, NULL,recieveBroadcastMessage ,(void *)&networkArgs);
    pthread_create(&render_thread , NULL , renderNetwork , (void *)&networkArgs);
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
    int socketFD = handler->connection->getSocket();

    while(true){
        int code = receive4Byte(socketFD);
        if(code == FILE_EXIST_CODE){ // Code for cheking file is exist
            if(!answerToCheckRequest(socketFD)){
                break;
            }
        } else if(code == ANSWER_TO_EXIST_CODE){ // Code for answer to file exist
            if(!responseToFileIsExist(socketFD , handler->networkArgs)){
                break;
            }
        } else if(code == SEND_FILE_CODE){ // Code for send some part of file
            if(!sendFileToSocket(socketFD , handler->networkArgs)){
                break;
            }
        } else if(code == RECEIVE_FILE_CODE){ // Code for recieve part of file
            if(!receiveFileFromSocket(socketFD , handler->networkArgs)){
                break;
            }
        } else if(code == PART_NOT_FIND_CODE){ // Code for asking for another part
            if(!askAnotherPartFromSocket(socketFD , handler->networkArgs)){
                break;
            }
        } else {
            printf("some thing went wrong in recieving code!!\n");
            break;
        }
    }

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
    // Send need message to new connection
    std::vector<SocketFile *> *socket_files = networkArgs->getSocketFiles();
    for(SocketFile * socket_file : *socket_files){
        if(!socket_file->isFinished()){
            sendNeedMessage(connection->getSocket() , socket_file->getName());
        }
    }
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
        networkArgs->lock();
        SocketFile * socket_file = new SocketFile(file_name);
        socket_file->save();
        networkArgs->addSocketFile(socket_file);
        networkArgs->unlock();
        // Send need message to all connections
        std::vector<Connection *> * connectios = networkArgs->getConnections();
        for(auto it = connectios->begin() ; it != connectios->end() ; it++){
            sendNeedMessage((*it)->getSocket() , file_name);
        }
    }
}

void * renderNetwork(void * arg){
    NetworkArgs * networkArgs = (NetworkArgs *) arg;
    while (true)
    {
        // Printing connections
        std::vector<Connection *> * connectios = networkArgs->getConnections();
        printf("num of connections : %ld\n",connectios->size());
        for(Connection * connection : *connectios){
            printf("conneted to IP : %s\n", inet_ntoa(connection->getSocketAddr().sin_addr));
        }

        // Printing downloads
        std::vector<SocketFile *> *socket_files = networkArgs->getSocketFiles();
        printf("num of downloads : %ld\n",socket_files->size());
        for(SocketFile * socket_file : *socket_files){
            long change = socket_file->getBytesCompletedNum() - socket_file->getLastChange();
            socket_file->setLastChange(socket_file->getBytesCompletedNum());
            printf("file : %s downloading with %ldb/s %f \n",socket_file->getName().c_str() , change*TRANSFER_SIZE , socket_file->getPercentCompleted());
        }
        usleep(1000000);
        system("clear");
    }
    
    return NULL;
}