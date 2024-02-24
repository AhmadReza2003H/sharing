#include "SocketFile.h"

SocketFile::SocketFile(){
    this->bytes_read = 0;
    this->file_length = -1;
}

SocketFile::SocketFile(std::string name){
    this->name = name;
    this->bytes_read = 0;
    this->file_length = -1;
}


SocketFile::~SocketFile(){
}

std::string SocketFile::getName(){
    return this->name;
}


void SocketFile::setName(std::string name){
    this->name = name;
}


long SocketFile::getFileLength(){
    return this->file_length;
}


void SocketFile::setFileLength(long file_length){
    this->file_length = file_length;
}

long SocketFile::getBytesRead(){
    return this->bytes_read;
}


void SocketFile::setBytesRead(long bytes_read){
    this->bytes_read = bytes_read;
}

bool SocketFile::isFinished(){
    return this->file_length == this->bytes_read;
}

void SocketFile::addSocket(int socket){
    this->sockets.insert(socket);
}

std::set<int> SocketFile::getSockets(){
    return this->sockets;
}

void SocketFile::removeSocket(int socket){
    this->sockets.erase(socket);
}

long SocketFile::getNextEndToRead(){
    return std::min(this->bytes_read + TRANSFER_SIZE , this->file_length);
}

void SocketFile::createFile(){
    FILE * file_ptr = fopen(this->name.c_str() , "ab+");
    fclose(file_ptr);
}

float SocketFile::getPercentCompleted(){
    return (this->bytes_read*100.0)/this->file_length;
}