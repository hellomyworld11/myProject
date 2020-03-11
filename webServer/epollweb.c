
#include "epollweb.h"

// 16进制数转化为10进制
int hexit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

/*
 *  这里的内容是处理%20之类的东西！是"解码"过程。
 *  %20 URL编码中的‘ ’(space)
 *  %21 '!' %22 '"' %23 '#' %24 '$'
 *  %25 '%' %26 '&' %27 ''' %28 '('......
 *  相关知识html中的‘ ’(space)是&nbsp
 */
void encode_str(char* to, int tosize, const char* from)
{
    int tolen;

    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) 
    {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) 
        {
            *to = *from;
            ++to;
            ++tolen;
        } 
        else 
        {
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }

    }
    *to = '\0';
}


void decode_str(char *to, char *from)
{
    for ( ; *from != '\0'; ++to, ++from  ) 
    {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) 
        { 

            *to = hexit(from[1])*16 + hexit(from[2]);

            from += 2;                      
        } 
        else
        {
            *to = *from;

        }

    }
    *to = '\0';

}
void epoll_run(int port)
{
    USUALSTRUCT st;
    USUALSTRUCT stread;
	//创建
	int epfd = epoll_create(MAXSIZE);
	if(epfd == -1)
	{
		perror("epoll_create error");
		exit(1);
	}
	//添加节点 监听节点Lfd
	int lfd = init_listenfd(port,epfd);

	//开始检测
	struct epoll_event all[MAXSIZE];
	while(1)
	{
		int ret = epoll_wait(epfd,all,MAXSIZE,-1);
		if(-1 == ret)
		{
			perror("epoll_wait error");
			exit(1);
		}
		for(int i = 0 ; i < ret ;i++)
		{
			//只处理读事件
			struct epoll_event *pev = &all[i];
			if(!(pev->events & EPOLLIN))
			{
				continue; // 不是读事件跳出循环
			}
			if(pev->data.fd == lfd)
			{
				//接收连接请求		
                st.fd = lfd;
                st.st_epfd = epfd;
				//do_accept(lfd,epfd);
                pthread_t tid;
                int ret = pthread_create(&tid, NULL, do_accept, (void *)(&st));
                if(-1 == ret)
                {
                    perror("create acceptthread error");
                    do_accept((void *)&st);
                }else  pthread_detach(tid);
			}else{
				//读数据
				stread.fd = pev->data.fd;
                stread.st_epfd = epfd;
                pthread_t tid;
                int ret = pthread_create(&tid,NULL,do_read,(void*)(&stread));
                if(-1 ==ret)
                {
                    perror("create readthread error");
                    do_read((void *)&stread);
                 
                }else  pthread_detach(tid);
			}

		}
	}
}
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                {
                    recv(sock, &c, 1, 0);
                }
                else
                {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        }
        else
        {
            c = '\n';
        }
    }
    buf[i] = '\0';
 //   if(n == -1)
 //   {
 //   	return -1;
 //   }
    return i;
}
void* do_read(void *arg)
{
    USUALSTRUCT *st = (USUALSTRUCT*)arg;
    int cfd = st->fd;
    int epfd = st->st_epfd;
	//将浏览器发过来的数据读到buf中
	char line[1024] = {0};
	//请求行
	int len = get_line(cfd,line,sizeof(line));
	if(len ==0)
	{
		printf("客户端断开连接\n");
		//关闭套接字
		//del cfd
		disconnect(cfd,epfd);
	}else if(len == -1)
	{
		perror("recv error");
		pthread_exit((void *)1);
	}else{
		printf("请求行：%s\n",line);
		while(len)
		{
			char buf[1024] = {0};
			len = get_line(cfd,buf,sizeof(buf));
			printf("请求头：%s\n",buf);
		}
	}
	//请求行
	if(strncasecmp("get",line,3) == 0)
	{
		//处理请求
		http_request(line,cfd);
		disconnect(cfd,epfd);
	}
    pthread_exit(NULL);
}

void do_readold(int cfd,int epfd)
{
	//将浏览器发过来的数据读到buf中
	char line[1024] = {0};
	//请求行
	int len = get_line(cfd,line,sizeof(line));
	if(len ==0)
	{
		printf("客户端断开连接\n");
		//关闭套接字
		//del cfd
		disconnect(cfd,epfd);
	}else if(len == -1)
	{
		perror("recv error");
		exit(1);
	}else{
		printf("请求行：%s\n",line);
		while(len)
		{
			char buf[1024] = {0};
			len = get_line(cfd,buf,sizeof(buf));
			printf("请求头：%s\n",buf);
		}
	}
	//请求行
	if(strncasecmp("get",line,3) == 0)
	{
		//处理请求
		http_request(line,cfd);
		disconnect(cfd,epfd);
	}

}
void disconnect(int cfd,int epfd)
{
	int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,NULL);
	if(ret == -1)
	{
		//show 404		
		perror("epoll_ctl del error");
	}
	close(cfd);
}
//http请求处理
void http_request(const char * request,int cfd)
{
	//拆分
	char method[12],path[1024],protocol[12];
	sscanf(request,"%[^ ] %[^ ] %[^ ]",method,path,protocol);
	printf("method:%s , path:%s,protocol:%s\n",method,path,protocol);
	//处理path
	decode_str(path,path);
	char *file = path+1;

	if(strcmp(path,"/")==0)
	{
		file = "./";
	}
	//获取文件属性
	struct stat st;
	int ret = stat(file,&st);
	if(ret == -1)
	{
		send_respnd_head(cfd,404,"FILE NOT FOUND",".html",-1);
		send_file(cfd,"404.htm");
	}
	//判断是目录还是文件
	if(S_ISDIR(st.st_mode))
	{
		//发送头
		send_respnd_head(cfd,200,"OK",get_file_type(".html"),-1);
		//发送目录
		send_dir(cfd,file);


	}else if(S_ISREG(st.st_mode)){
		//文件
		//发送消息包头
		send_respnd_head(cfd,200,"OK",get_file_type(file),-1);
		//发送文件内容
		send_file(cfd,file);
	}

}
void send_dir(int cfd,const char* dirname)
{
	//html页面
	char buf[4096] = {0};
	sprintf(buf,"<html><head><title>目录名:%s</title></head>",dirname);
	sprintf(buf+strlen(buf),"<body><h1>当前目录:%s</h1><table>",dirname);
#if 0
	//打开目录
	DIR *dir = opendir(dirname);
	if(dir == NULL)
	{
		perror("opendir error");
		exit(1);
	}
	//都目录
	struct dirent *ptr = NULL;

	while((ptr = readdir(dir))!=NULL)
	{
		sprintf(buf,"<tr><td></td>");
		char *name = ptr->d_name;

	}
	closedir(dir);
#endif
	//存放目录项
	char enstr[1024]={0};
	char path[1024] = {0}; 
	struct dirent **ptr;
	int num = scandir(dirname,&ptr,NULL,alphasort);
	//遍历
	for(int i = 0 ;i <num;i++)
	{
		char *name = ptr[i]->d_name;
		//字符串拼接
		sprintf(path,"%s/%s",dirname,name);
		struct stat st;
		stat(path,&st);
		encode_str(enstr,sizeof(enstr),name);
		//如果文件
		if(S_ISREG(st.st_mode))
		{
			sprintf(buf+strlen(buf),"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				enstr,name,st.st_size);
		}else if(S_ISDIR(st.st_mode))
		{
			sprintf(buf+strlen(buf),"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				enstr,name,st.st_size);
		}
		send(cfd,buf,strlen(buf),0);
		memset(buf,0,sizeof(buf));
		//如果目录
	}
	sprintf(buf,"</table></body></html>");
	send(cfd,buf,strlen(buf),0);
	printf("send message ok\n");
}
//发送文件内容
void send_file(int cfd,const char *filename)
{
	//打开文件
	int fd = open(filename , O_RDONLY);
	if(-1 == fd)
	{
		//show 404
		return;
	}
	//循环读文件
	char buf[4096] = {0};
	int len = 0;
    printf("----\n");
	while((len = read(fd,buf,sizeof(buf)))>0)
	{
		int ret = send(cfd,buf,len,0);
        printf("send : %d\n",ret);
         memset(buf,0,sizeof(buf));
        if(-1 ==ret)
        {
         //   printf("errno = %d\n",errno);
            if(errno == EAGAIN)
            {
                continue;
            }else if(errno == EINTR)
            {
                continue;
            }else{
                perror("send error");
                pthread_exit((void *)1);
             }
        }
       
	}
//	if(len == -1)
//	{
//		perror("read file error");
//		pthread_exit((void*)1);
//	}
	close(fd);
}
//发送响应消息头
void send_respnd_head(int cfd ,int no,const char *desp,const char *type,long len)
{
	char buf[1024] = {0};
	//状态行
	sprintf(buf,"http/1.1 %d %s\r\n",no,desp);
	send(cfd,buf,strlen(buf),0);
	//消息包头
	sprintf(buf,"Content-Type:%s;charset=utf-8\r\n",type);
	send(cfd,buf,strlen(buf),0);
	sprintf(buf,"Content-Length:%ld\r\n",len);
	send(cfd,buf,strlen(buf),0);
	//空行
	send(cfd,"\r\n",2,0);
}
// 通过文件名获取文件的类型
const char *get_file_type(const char *name)
{
    char* dot;

    // 自右向左查找‘.’字符, 如不存在返回NULL
    dot = strrchr(name, '.');   
    if (dot == NULL)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}
void* do_accept(void *arg)
{
    USUALSTRUCT *st = (USUALSTRUCT *)arg;
    int lfd = st->fd;
    int epfd = st->st_epfd;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	int cfd = accept(lfd ,(struct sockaddr*)&client,&len);
	if(cfd == -1)
	{
		perror("accept error");		
        pthread_exit((void *)1);
	}
	char ip[64] = {0};
	printf("New CLient ip:%s,Port:%d,cfd = %d\n",inet_ntop(AF_INET,&client.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client.sin_port),cfd);
	//挂到epoll树上	
	struct epoll_event ev;
	ev.data.fd = cfd;
	//设置cfd非阻塞
	int flag = fcntl(cfd,F_GETFL);
	flag|=O_NONBLOCK;
	fcntl(cfd,F_SETFL,flag);
	//边沿非阻塞
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
    pthread_exit(NULL);
}

void do_acceptold(int lfd,int epfd)
{
	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	int cfd = accept(lfd ,(struct sockaddr*)&client,&len);
	if(cfd == -1)
	{
		perror("accept error");
		exit(1);
	}
	char ip[64] = {0};
	printf("New CLient ip:%s,Port:%d,cfd = %d\n",inet_ntop(AF_INET,&client.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client.sin_port),cfd);
	//挂到epoll树上	
	struct epoll_event ev;
	ev.data.fd = cfd;
	//设置cfd非阻塞
	int flag = fcntl(cfd,F_GETFL);
	flag|=O_NONBLOCK;
	fcntl(cfd,F_SETFL,flag);
	//边沿非阻塞
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
}

int init_listenfd(int port,int epfd)
{
	//创建套接字
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("socket cretae error");
		exit(1);
	}
	//绑定
	struct sockaddr_in serv;
	memset(&serv,0,sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	int flag =1;//端口复用
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
	int ret = bind(sockfd,(struct sockaddr*)&serv,sizeof(serv));
	if(ret == -1)
	{
		perror("bind error");
		exit(1);
	}
	ret = listen(sockfd,64);
	if(ret ==-1)
	{
		perror("listen error");
		exit(1);
	}
	//sockfd添加到树上
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	ret = epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
	if(-1 == ret)
	{
		perror("epoll_ctl error");
		exit(1);
	}
	return sockfd;
}


