#include "epollweb.h"
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char const *argv[])
{
	if(argc < 3)
	{
		printf("eg:./a.out port path\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	int ret = chdir(argv[2]);

	//启动epoll模型
	epoll_run(port);

	return 0;
}