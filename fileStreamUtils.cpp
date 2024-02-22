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

void responseToFileIsExist(int socketFD){
    long file_length = receive8Byte(socketFD);
    File file = receiveFileName(socketFD);
    if(file_length != -1) {
        send4Byte(socketFD , SEND_FILE_CODE); 
        sendFileName(socketFD,  file.name_length , file.name);
        // todo send specefic part of file 
    }
    delete[] file.name;
}

void sendFileToSocket(int sockedFD){
    File file = receiveFileName(sockedFD);

    FILE * file_ptr;
    file_ptr = fopen(file.name , BINARY_READER);
    long file_length = getFileSize(file_ptr);  

    send4Byte(sockedFD , RECEIVE_FILE_CODE);
    sendFileName(sockedFD , file.name_length , file.name);
    send8Byte(sockedFD , file_length);

    if(file_ptr){
        char *buffer = new char[file_length];
        fread(buffer , file_length , 1 , file_ptr);
        sendNByte(sockedFD , buffer , file_length);
    }

    delete[] file.name;
}

void receiveFileFromSocket(int socketFD){
    File file = receiveFileName(socketFD);
    long file_length = receive8Byte(socketFD);

    if(file_length != -1){
        char * buffer = receiveNByte(socketFD , file_length);
        FILE * file_ptr = fopen(file.name , BINARY_READ_AND_WRITE_APPEND);

        // Write the buffer to the file
        fwrite(buffer, sizeof(char), file_length, file_ptr);

        // Close the file and free the buffer
        fclose(file_ptr);
        delete[] buffer;

        printf("new file : %s downloaded\n", file.name);
    }


    delete[] file.name;
}