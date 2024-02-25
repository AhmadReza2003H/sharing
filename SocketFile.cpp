#include "SocketFile.h"

SocketFile::SocketFile(){
    this->request_index = 0;
    this->bytes_completed_num = 0;
    this->file_length = -1;
}

SocketFile::SocketFile(std::string name){
    this->name = name;
    this->request_index = 0;
    this->bytes_completed_num = 0;
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
    long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
    this->bytes_completed = new bool[size];
}

bool SocketFile::isFinished(){
    long size = std::ceil(file_length/TRANSFER_SIZE);
    return this->bytes_completed_num == size;
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


long SocketFile::getNextStartToRead(){
    long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
    long start;
    if(this->isFinished()){
        start = -1;
    } else {
        while (this->bytes_completed[this->request_index] != false){
            this->request_index = (this->request_index + 1)%size;
        }
        start = std::min((this->request_index)*TRANSFER_SIZE , this->file_length);
        this->request_index = (this->request_index + 1)%size;
    }
    return start;
}

void SocketFile::createFile(){
    FILE * file_ptr = fopen(this->name.c_str() , "ab+");
    fclose(file_ptr);
}

float SocketFile::getPercentCompleted(){
    return (this->bytes_completed_num*TRANSFER_SIZE*100.0)/this->file_length;
}

void SocketFile::setPartCompleted(long start_index){
    this->bytes_completed[start_index / TRANSFER_SIZE] = true;
}

void SocketFile::increaseBytesCompleted(){
    this->bytes_completed_num++;
}

