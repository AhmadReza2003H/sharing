#ifndef _SOCKETFILE_H_
#define _SOCKETFILE_H_
#include <string>
#include <set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#define TRANSFER_SIZE 5000

class SocketFile {
private:
    std::string name;
    long file_length;
    long request_index;
    long bytes_completed_num;
    long last_change;
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
    void setLastChange(long);
    std::string getName();
    std::set<int> getSockets();
    long getFileLength();
    long getNextStartToRead();
    long getLastChange();
    long getBytesCompletedNum();
    bool isFinished();
    void createFile();
    void increaseBytesCompleted();
    float getPercentCompleted();
    void serialize(std::ofstream&) const;
    void deserialize(std::ifstream&);
};


#endif // _SOCKETFILE_H_