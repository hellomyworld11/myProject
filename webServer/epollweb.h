#ifndef _EPOLL_WEB_H
#define _EPOLL_WEB_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#define MAXSIZE 2000
typedef struct{
    int fd;  //待操作的文件描述符
    int st_epfd;  //epoll根节点

}USUALSTRUCT;
void epoll_run(int port);
int init_listenfd(int port,int epfd);
void* do_accept(void *arg);
int get_line(int sock, char *buf, int size);
void send_dir(int cfd,const char* dirname);
void send_file(int cfd,const char *filename);
void* do_read(void *arg);
void disconnect(int cfd,int epfd);
void http_request(const char * request,int cfd);
void send_respnd_head(int cfd ,int no,const char *desp,const char *type,long len);
void encode_str(char* to, int tosize, const char* from);
void decode_str(char *to, char *from);
const char *get_file_type(const char *name);
int hexit(char c);
#endif