#ifndef _FILESTREAMUTILS_H_
#define _FILESTREAMUTILS_H_

#define CODE_SIZE sizeof(int)
#define FILE_EXIST_CODE 1
#define ANSWER_TO_EXIST_CODE 2
#define SEND_FILE_CODE 3
#define RECEIVE_FILE_CODE 4
#define PART_NOT_FIND_CODE 5
#define BINARY_READER "rb" // Return NULL if not exist
#define BINARY_READ_AND_WRITE "rb+" // Return NULL if not exist
#define BINARY_READ_AND_WRITE_APPEND "ab+" // Create file if not exist
#define FILES_DIR "/Downloads"

struct File
{
    char * name;
    int name_length;
};


#include "Connection.h"
#include "socketUtils.h"
#include "NetworkArgs.h"

class NetworkArgs;
class SocketFile;

long getFileSize(FILE *);
bool sendFileName(int , int , char *);
struct File receiveFileName(int);
bool sendNeedMessage(int , std::string);
bool answerToCheckRequest(int);
bool responseToFileIsExist(int , NetworkArgs *);
bool sendFileIsExist(int , char * , int);
bool sendFileToSocket(int , NetworkArgs *);
bool sendFilePart(int , File , long , long , FILE *);
bool askPartFromSocket(int , SocketFile * , File);
bool receiveFileFromSocket(int , NetworkArgs *);
bool askAnotherPartFromSocket(int , NetworkArgs *);
void createFilesDirectory();


#endif // _FILESTREAMUTILS_H_