#include "fileStreamUtils.h"

bool sendFileName(int socketFD, int file_name_length, char * file_name){
    if(!send4Byte(socketFD , file_name_length)){
        return false;
    }
    if(!sendNByte(socketFD , file_name , file_name_length)){
        return false;
    }
    return true;
}
struct File receiveFileName(int socketFD){
    File file;
    int file_name_length = receive4Byte(socketFD);
    char * name = receiveNByte(socketFD , file_name_length);
    file.name = name;
    file.name_length = file_name_length;
    return file;
}

void createFilesDirectory(){
    std::string current_directory = fs::current_path().string(); // Get current dierctory
    std::string files_directory = current_directory + FILES_DIR ; // Goto files directory directory
    
    if (!fs::exists(files_directory)){ // Create this folder (when code run for the first time)
        fs::create_directory(files_directory);
    }
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

bool sendNeedMessage(int socketFD , std::string name){
    if(!send4Byte(socketFD , FILE_EXIST_CODE)){
        return false;
    }
    if(!send4Byte(socketFD , name.length())){
        return false;
    }
    if(!sendNByte(socketFD , name.c_str() , name.length())){
        return false;
    }
    return true;
}

bool answerToCheckRequest(int socketFD){
    int file_name_length;
    char * name;
    try {
        file_name_length = receive4Byte(socketFD);  // Receive file name length
        name = receiveNByte(socketFD , file_name_length); // Receive file name
    } catch(...){
        return false;
    }
    return sendFileIsExist(socketFD , name , file_name_length); // Response to it
}

bool sendFileIsExist(int socketFD, char * file_name , int file_name_length){

    FILE * file_ptr;
    std::string file_path = fs::current_path().string() + FILES_DIR + '/' + std::string(file_name);
    file_ptr = fopen(file_path.c_str() , BINARY_READER);
    long file_length = getFileSize(file_ptr);

    if(!send4Byte(socketFD , ANSWER_TO_EXIST_CODE)){
        delete[] file_name;
        return false;
    }
    if(!send8Byte(socketFD , file_length)){
        delete[] file_name;
        return false;
    }
    if(!sendFileName(socketFD , file_name_length , file_name)){
        delete[] file_name;
        return false;
    }

    if(file_ptr){
        fclose(file_ptr);
    }

    delete[] file_name;
    return true;
}

bool responseToFileIsExist(int socketFD , NetworkArgs * networkArgs){
    long file_length;
    File file;
    try{
        file_length = receive8Byte(socketFD);
        file = receiveFileName(socketFD);
    } catch(...){
        return false;
    }
    bool result = true;
    if(file_length != -1) {
        networkArgs->lock();
        SocketFile * socketFile  = networkArgs->getSocketFile(file.name);

        if(socketFile != NULL){

            if(socketFile->getFileLength() == -1){
                socketFile->setFileLength(file_length);
                socketFile->save();
            }


            if(!socketFile->isFinished()){
                long start , end;
                start = socketFile->getNextStartToRead();
                end = std::min(start + TRANSFER_SIZE , socketFile->getFileLength());
                result = send4Byte(socketFD , SEND_FILE_CODE);
                result &= send8Byte(socketFD , start);
                result &= send8Byte(socketFD , end);
                result &= sendFileName(socketFD , file.name_length , file.name);
                socketFile->save();
            } else {
                networkArgs->deleteSocketFile(socketFile);
                result = true;
            }
        }
        networkArgs->unlock();
    }
    delete[] file.name;
    return result;
}

bool askPartFromSocket(int socketFD, SocketFile * socketFile , File file){
    bool result;
    long start , end;
    start = socketFile->getNextStartToRead();
    end = std::min(start + TRANSFER_SIZE , socketFile->getFileLength());
    result = send4Byte(socketFD , SEND_FILE_CODE);
    result &= send8Byte(socketFD , start);
    result &= send8Byte(socketFD , end);
    result &= sendFileName(socketFD , file.name_length , file.name);
    socketFile->save();
    return result;
}

bool sendFilePart(int socketFD, File file, long start, long end, FILE * file_ptr){
    long size_to_read = end - start;
    // Seek to the starting position
    if (fseek(file_ptr, start, SEEK_SET) != 0) {
        fclose(file_ptr);
        delete[] file.name;
        return false;
    }

    char * buffer = new char[size_to_read];

    // Read the data into buffer
    if (fread(buffer, 1, size_to_read, file_ptr) != size_to_read) {
        fclose(file_ptr);
        delete[] buffer;
        delete[] file.name;
        return false;
    }
    bool result;
    result = send4Byte(socketFD , RECEIVE_FILE_CODE);
    result &= send8Byte(socketFD , start);
    result &= send8Byte(socketFD , end);
    result &= sendFileName(socketFD , file.name_length , file.name);
    result &= sendNByte(socketFD , buffer , size_to_read);
    delete[] buffer;
    return result;
}

bool sendFileToSocket(int socketFD , NetworkArgs * networkArgs){
    long start , end;
    File file;


    try{
        start = receive8Byte(socketFD);
        end = receive8Byte(socketFD);
        file = receiveFileName(socketFD);
    } catch(...){
        return false;
    }

    std::string file_path = fs::current_path().string() + FILES_DIR + '/' + std::string(file.name);
    FILE * file_ptr;
    file_ptr = fopen(file_path.c_str() , BINARY_READER);

    if(file_ptr == NULL){
        delete[] file.name;
        return true;
    }

    SocketFile * socket_file = networkArgs->getSocketFile(file.name);
    bool result = true;
    if(socket_file != NULL){ // File is in dowloading process
        if(socket_file->isThisPartFinished(start)){
            result &= sendFilePart(socketFD , file , start , end , file_ptr);
        } else {
            result &= send4Byte(socketFD , PART_NOT_FIND_CODE);
            result &= sendFileName(socketFD , file.name_length , file.name);
        }
    } else {
        result &= sendFilePart(socketFD , file , start , end , file_ptr);
    }
    fclose(file_ptr);
    delete[] file.name;
    return result;
}

bool askAnotherPartFromSocket(int socketFD, NetworkArgs * networkArgs){
    File file;
    try{
        file = receiveFileName(socketFD);
    } catch(...){
        return false;
    }
    SocketFile * socket_file = networkArgs->getSocketFile(file.name);
    bool result = true;
    if(socket_file != NULL){
        networkArgs->lock();
        if(!socket_file->isFinished()){
            long start , end;
            start = socket_file->getNextStartToRead();
            end = std::min(start + TRANSFER_SIZE , socket_file->getFileLength());
            result = send4Byte(socketFD , SEND_FILE_CODE);
            result &= send8Byte(socketFD , start);
            result &= send8Byte(socketFD , end);
            result &= sendFileName(socketFD , file.name_length , file.name);
            socket_file->save();
        } else {
            networkArgs->deleteSocketFile(socket_file);
        }
        networkArgs->unlock();
    }
    delete[] file.name;
    return result;
}


bool receiveFileFromSocket(int socketFD, NetworkArgs * networkArgs){
    long start , end  , length;
    File file;
    char * buffer;
    try {
        start = receive8Byte(socketFD);
        end = receive8Byte(socketFD);
        file = receiveFileName(socketFD);
        length = end - start;
    } catch(...){
        return false;
    }
    try{
        buffer = receiveNByte(socketFD , length);
    } catch(...){
        delete[] file.name;
        return false;
    }

    networkArgs->lock();

    SocketFile * socketFile  = networkArgs->getSocketFile(file.name);
    std::string file_path = fs::current_path().string() + FILES_DIR + '/' + std::string(file.name);
    FILE * file_ptr = fopen(file_path.c_str() , BINARY_READ_AND_WRITE_APPEND);
    
    // Seek to the desired position in the file
    if (fseek(file_ptr, start, SEEK_SET) != 0) {
        fclose(file_ptr);
        delete[] buffer;
        delete[] file.name;
        networkArgs->unlock();
        return false;
    }

    // Write the new data to the file
    if (fwrite(buffer, 1, length, file_ptr) != length) {
        fclose(file_ptr);
        delete[] buffer;
        delete[] file.name;
        networkArgs->unlock();
        return false;    
    }

    socketFile->increaseBytesCompleted();
    socketFile->setPartCompleted(start);
    socketFile->save();
    bool result = true;
    if(!socketFile->isFinished()){
        start = socketFile->getNextStartToRead();
        end = std::min(start + TRANSFER_SIZE , socketFile->getFileLength());
        result = send4Byte(socketFD , SEND_FILE_CODE);
        result &= send8Byte(socketFD , start);
        result &= send8Byte(socketFD , end);
        result &= sendFileName(socketFD , file.name_length , file.name);
        socketFile->save();
    } else {
        networkArgs->deleteSocketFile(socketFile);
    }

    fclose(file_ptr);
    networkArgs->unlock();

    delete[] buffer;
    delete[] file.name;
    return result;
}