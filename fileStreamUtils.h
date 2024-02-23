#ifndef _FILESTREAMUTILS_H_
#define _FILESTREAMUTILS_H_

#define CODE_SIZE sizeof(int)
#define FILE_EXIST_CODE 1
#define ANSWER_TO_EXIST_CODE 2
#define SEND_FILE_CODE 3
#define RECEIVE_FILE_CODE 4
#define BINARY_READER "rb" // Return NULL if not exist
#define BINARY_READ_AND_WRITE "rb+" // Return NULL if not exist
#define BINARY_READ_AND_WRITE_APPEND "ab+" // Create file if not exist

struct File
{
    char * name;
    int name_length;
};


#include "Connection.h"
#include "socketUtils.h"
#include "NetworkArgs.h"

long getFileSize(FILE *);
void sendFileName(int , int , char *);
struct File receiveFileName(int);
void sendNeedMessage(int , std::string);
void answerToCheckRequest(int);
void responseToFileIsExist(int , NetworkArgs *);
void sendFileIsExist(int , char * , int);
void sendFileToSocket(int);
void receiveFileFromSocket(int , NetworkArgs *);


#endif // _FILESTREAMUTILS_H_