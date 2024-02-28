#include "SocketFile.h"

SocketFile::SocketFile(){
    this->request_index = 0;
    this->bytes_completed_num = 0;
    this->last_change = 0;
    this->file_length = -1;
    this->bytes_completed = NULL;
}

SocketFile::SocketFile(std::string name){
    this->name = name;
    this->request_index = 0;
    this->bytes_completed_num = 0;
    this->last_change = 0;
    this->file_length = -1;
    this->bytes_completed = NULL;
}


SocketFile::~SocketFile(){
    if(bytes_completed){
        delete[] bytes_completed;
    }
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
    for(long i = 0 ; i < size ; i++){
        bytes_completed[i] = 0;
    }
}

bool SocketFile::isFinished(){
    long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
    return this->bytes_completed_num == size && this->file_length != -1;
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
    long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
    return (this->bytes_completed_num*100.0)/size;
}

void SocketFile::setPartCompleted(long start_index){
    this->bytes_completed[start_index / TRANSFER_SIZE] = true;
}

bool SocketFile::isThisPartFinished(long start_index){
    return this->bytes_completed[start_index / TRANSFER_SIZE];
}


void SocketFile::increaseBytesCompleted(){
    this->bytes_completed_num++;
}

void SocketFile::serialize(std::ofstream& ofs) const{
    ofs << this->name << '\n';
    ofs << this->file_length << '\n';
    if(this->file_length != -1){
        ofs << this->request_index << '\n';
        ofs << this->bytes_completed_num << '\n';
        long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
        for(int i = 0 ; i < size ; i++){
            ofs << this->bytes_completed[i] << ' ';
        }
        ofs << '\n';
    }
}

void SocketFile::deserialize(std::ifstream& ifs){
    ifs >> this->name;
    ifs >> this->file_length;
    if(this->file_length != -1){
        ifs >> this->request_index;
        ifs >> this->bytes_completed_num;
        long size = std::ceil(file_length*1.0/TRANSFER_SIZE);
        this->bytes_completed = new bool[size];
        for(int i = 0 ; i < size ; i++){
            ifs >> this->bytes_completed[i];
        }
    }
}

void SocketFile::setLastChange(long change){
    this->last_change = change;
}

long SocketFile::getLastChange(){
    return this->last_change;
}

long SocketFile::getBytesCompletedNum(){
    return this->bytes_completed_num;
}

void SocketFile::save(){
    std::string current_directory = fs::current_path().string(); // Get current dierctory
    std::string directory = current_directory + DETEAILS_DIR + "/" + this->name + ".txt"; // Goto DownloadDetails directory
    
    // Open file for writing
    std::ofstream ofs(directory);
    if (!ofs.is_open()) {
        perror("Failed to open file for writing.");
    } else {
        this->serialize(ofs);
        ofs.close();
    }
}