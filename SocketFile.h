#ifndef _SOCKETFILE_H_
#define _SOCKETFILE_H_
#include <string>
#include <set>
#include <algorithm>
#include <cmath>


#define TRANSFER_SIZE 1000

class SocketFile {
private:
    std::string name;
    long file_length;
    long request_index;
    long bytes_completed_num;
    bool * bytes_completed;
    std::set<int> sockets;
public:
    SocketFile();
    SocketFile(std::string);
    ~SocketFile();
    void setName(std::string);
    void setFileLength(long);
    void addSocket(int);
    void removeSocket(int);
    void setPartCompleted(long);
    std::string getName();
    std::set<int> getSockets();
    long getFileLength();
    long getNextStartToRead();
    bool isFinished();
    void createFile();
    void increaseBytesCompleted();
    float getPercentCompleted();
};


#endif // _SOCKETFILE_H_