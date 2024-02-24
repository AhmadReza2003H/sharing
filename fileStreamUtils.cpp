#include "fileStreamUtils.h"

void sendFileName(int socketFD, int file_name_length, char * file_name){
    send4Byte(socketFD , file_name_length); // Send file name length and file name
    sendNByte(socketFD , file_name , file_name_length);
}
struct File receiveFileName(int socketFD){
    File file;
    int file_name_length = receive4Byte(socketFD);
    char * name = receiveNByte(socketFD , file_name_length);
    file.name = name;
    file.name_length = file_name_length;
    return file;
}

long getFileSize(FILE * file_ptr){
    long file_length;
    if(file_ptr != NULL){
        // Get the file size
        fseek(file_ptr, 0, SEEK_END);
        file_length = ftell(file_ptr);
        rewind(file_ptr);
    } else {
        file_length = -1;
    }
    return file_length;
}

void sendNeedMessage(int socketFD , std::string name){
    send4Byte(socketFD , FILE_EXIST_CODE);
    send4Byte(socketFD , name.length());
    sendNByte(socketFD , name.c_str() , name.length());
}

void answerToCheckRequest(int socketFD){
    int file_name_length = receive4Byte(socketFD);  // Receive file name length
    char * name = receiveNByte(socketFD , file_name_length); // Receive file name
    sendFileIsExist(socketFD , name , file_name_length); // Response to it
    delete[] name;
}

void sendFileIsExist(int socketFD, char * file_name , int file_name_length){

    FILE * file_ptr;
    file_ptr = fopen(file_name , BINARY_READER);
    long file_length = getFileSize(file_ptr);  

    send4Byte(socketFD , ANSWER_TO_EXIST_CODE); // Sending ANSWER_TO_EXIST_CODE to socket
    send8Byte(socketFD , file_length); // Sending file length to socket(send -1 if file not exist)
    sendFileName(socketFD , file_name_length , file_name);
    if(file_ptr){
        fclose(file_ptr);
    }
}

void responseToFileIsExist(int socketFD , NetworkArgs * networkArgs){
    long file_length = receive8Byte(socketFD);
    File file = receiveFileName(socketFD);
    if(file_length != -1) { // sync for changing
        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);

        SocketFile * socketFile  = networkArgs->getSocketFile(file.name);

        if(socketFile == NULL){
        } else {
            socketFile->addSocket(socketFD);

            if(socketFile->getFileLength() == -1){
                socketFile->setFileLength(file_length);
                socketFile->createFile();
            }


            if(!socketFile->isFinished()){
                long start , end;
                start = socketFile->getBytesRead();
                end = socketFile->getNextEndToRead();

                socketFile->setBytesRead(end);
                send4Byte(socketFD , SEND_FILE_CODE);
                send8Byte(socketFD , start);
                send8Byte(socketFD , end);
                sendFileName(socketFD,  file.name_length , file.name);
            } else {

            }
        }

        pthread_mutex_unlock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    delete[] file.name;
}

void sendFileToSocket(int socketFD){
    long start = receive8Byte(socketFD);
    long end = receive8Byte(socketFD);
    long size_to_read = end - start;

    File file = receiveFileName(socketFD);

    FILE * file_ptr;
    file_ptr = fopen(file.name , BINARY_READER);

    if(file_ptr == NULL){
        // error from sending name todo
        return;
    }

    // Seek to the starting position
    if (fseek(file_ptr, start, SEEK_SET) != 0) {
        // not have this part todo
        return;
    }

    char * buffer = new char[size_to_read];

    // Read the data into buffer
    if (fread(buffer, 1, size_to_read, file_ptr) != size_to_read) {
        // perror("Error reading file");
        return;
    }
    send4Byte(socketFD , RECEIVE_FILE_CODE);
    send8Byte(socketFD , start);
    send8Byte(socketFD , end);
    sendFileName(socketFD , file.name_length , file.name);
    send8Byte(socketFD ,size_to_read);
    sendNByte(socketFD , buffer , size_to_read);
    delete[] buffer;
    delete[] file.name;
}

void receiveFileFromSocket(int socketFD, NetworkArgs * networkArgs){
    long start = receive8Byte(socketFD);
    long end = receive8Byte(socketFD);
    File file = receiveFileName(socketFD);
    long file_length = receive8Byte(socketFD);
    char * buffer = receiveNByte(socketFD , file_length);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);

    SocketFile * socketFile  = networkArgs->getSocketFile(file.name);
    FILE * file_ptr = fopen(file.name , "r+b");
    
    // Seek to the desired position in the file
    if (fseek(file_ptr, start, SEEK_SET) != 0) {
        perror("Error seeking in file");
        fclose(file_ptr);
    }
    long length = end - start;

    // Write the new data to the file
    if (fwrite(buffer, 1, length, file_ptr) != length) {
        perror("Error writing to file");
        fclose(
            file_ptr);
    }

    if(!socketFile->isFinished()){
        long start2 , end2;
        start2 = socketFile->getBytesRead();
        end2 = socketFile->getNextEndToRead();

        socketFile->setBytesRead(end2);
        printf("read from %ld to %ld for file %s\n",start2 , end2 , file.name);

        send4Byte(socketFD , SEND_FILE_CODE);
        send8Byte(socketFD , start2);
        send8Byte(socketFD , end2);
        sendFileName(socketFD,  file.name_length , file.name);  
    } else {
        printf("end of receiving file : %s\n",file.name);
    }


    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    delete[] buffer;
    delete[] file.name;
}