#ifndef _SOCKETFILE_H_
#define _SOCKETFILE_H_
#include <string>
#include <set>
#include <algorithm>

class SocketFile
{
private:
    std::string name;
    long file_length;
    long bytes_read;
    std::set<int> sockets;
public:
    SocketFile();
    SocketFile(std::string);
    ~SocketFile();
    void setName(std::string);
    void setFileLength(long);
    void setBytesRead(long);
    void addSocket(int);
    void removeSocket(int);
    std::string getName();
    std::set<int> getSockets();
    long getFileLength();
    long getBytesRead();
    long getNextEndToRead();
    bool isFinished();
    void createFile();
};


#endif // _SOCKETFILE_H