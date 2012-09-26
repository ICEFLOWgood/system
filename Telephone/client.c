#include"my.h"


#define FALSE 0
#define TRUE  1

int sockfd;//连接套接字描述符；
char ip[256] = {0};
int port = 0;

typedef void* (*fun_SubThread)(void*); 


int Error_Print(char* sError)
{
#ifndef _MINGW_
//	perror(sError);
#else
 	printf(sError);
	printf("ErrorCore == %d\n",GetLastError());
#endif
	return FALSE;
}
int Thread_WaitQuit(int nId)
{
#ifndef _MINGW_
	 pthread_join(nId, NULL);
#else

	HANDLE hThread=(HANDLE)nId;
	if(WaitForSingleObject(hThread,INFINITE)) return Error_Print("WaitQuit");

	CloseHandle(hThread);
#endif
	return TRUE;
}
int Thread_Create(fun_SubThread ThreadName) 
{
#ifndef _MINGW_
	pthread_t tId;
	if(pthread_create(&tId,NULL,ThreadName,NULL)) return FALSE;
	return tId;
#else
	HANDLE hThread=NULL;
	int tId=0;
	hThread=(HANDLE)_beginthreadex(NULL,0,ThreadName,NULL,0,&tId);
	return (int)hThread;
#endif
}

void read_ip()
{
	FILE*fp = NULL;
	fp = fopen("ip.txt", "r");
	if(fp == NULL)
	{
		printf("read ip error!\n");
		exit(0);
	}
	fscanf(fp,"%s%d",ip,&port);
}
int Socket_Create(int af,int type,int protocol)
{
	int sock=0;

#ifdef _MINGW_
	WSADATA wsaData ={0};
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) return Error_Print("socket error\n");
	
	sock=socket(af,type,protocol);
    if (sock == INVALID_SOCKET) return Error_Print("socket error\n");
#else

	sock=socket(af,type,protocol);
	if (sock == -1) return Error_Print("socket error\n");
#endif

	return sock;
}

int ReadLine(char* pBuf,int nSize)
{
	int nLen=0;
	char c=0;
	
	do
	{
		c=getchar();
		pBuf[nLen++]=c;
	}while(c!='\n');
	
	return nLen;
}




int Socket_Read(int nSock,char* pBuf,int nSize)
{
	int ret;
#ifndef _MINGW_        
	ret = read(nSock,pBuf,nSize);
#else
	if(nSock==0)
	{
		ret = ReadLine(pBuf,nSize);	
	}
	else
	{
		ret = recv(nSock,pBuf,nSize,0);
	}
#endif
	return ret;
}
int Socket_Write(int nSock,char* pBuf,int nSize)
{
	int ret;
#ifndef _MINGW_        
	ret = write(nSock,pBuf,nSize);
#else
	ret = send(nSock,pBuf,nSize,0);
#endif 
	return TRUE;
}

int main()
{
    int ret = -1;
	setlocale(LC_CTYPE,"zh_CN.UTF-8");
	printf("与服务器正在连接...\n");
	read_ip();
    ret = function();//客户端功能展开函数；
    if(ret != 0)
    {
        Error_Print("function error\n");
        return -1;
    }
    return 0;
}

int function()
{
    int ret = -1;
    pthread_t tid1 = -1;//定义线程ID；
    pthread_t tid2 = -1;
    
    //IPv4地址族结构；
    struct sockaddr_in seraddr;
    memset (&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);
	seraddr.sin_addr.s_addr = inet_addr(ip);
	
	sockfd=Socket_Create(AF_INET,SOCK_STREAM,SOCKET_TCP);
	
    ret = connect(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr));//连接；
    if(ret != 0)
    {
        Error_Print("connect error\n");
        return -1;
    }
    printf("与服务器连接成功！\n");
	
	tid1=Thread_Create(write_ser);
	if(!tid1)return Error_Print("create_thread Write Error");
	tid2=Thread_Create(read_ser); 
	if(!tid2)return Error_Print("create_thread Read  Error");

    Thread_WaitQuit(tid1);//等待线程结束；
    Thread_WaitQuit(tid2);
	printf("线程退出...");
    return 0;
}

void *write_ser()//写数据的线程函数；
{   
    QR_HEAD package_head;//包头；
    memset(&package_head, 0, sizeof(QR_HEAD));
    package_head.package_len = 72;//定长包；
    package_head.package_id = 10;//协议类型；
    char data[64];//包的数据段；
    
    char *buf = malloc(sizeof(QR_HEAD)+64);    
    int ret = -1;
    //循环往套接字里写入数据；
    fflush(stdin);
    printf("查找支持：中文名、简拼、全拼、公司手机号、私人号码、分机号、邮箱！\n退出系统请输入：quit\n");
    printf("input message:");
    while(1)
    {
        fflush(stdout);
        fflush(stdin);
        memset(buf, 0, sizeof(QR_HEAD)+64);
        memcpy(buf, &package_head, sizeof(QR_HEAD));
        memset(data, 0, 64);
        ret = Socket_Read(0, data, sizeof(data)-1);//从标准输入中读取数据；
        if(ret == -1)
        {   
            Error_Print("write_ser read error\n");
            exit(0);
        }
        if(strncmp(data, "quit", sizeof("quit")-1)==0)
        {
            exit(0);
        }
        fflush(stdout);
        fflush(stdin);
        memcpy(buf+sizeof(QR_HEAD), data, 64);
        ret =Socket_Write(sockfd, buf, sizeof(QR_HEAD)+64);//把标准输入的数据写入到套接字里面；
        if(ret == -1)
        {
            Error_Print("write_ser write error\n");
            exit(0);
        }
    }
}

void *read_ser()//读数据的线程函数；
{
    int ret = -1;
    char buf[5096];
    //循环从socket套接字里读取数据，直至读取完毕；
    while(1)
    {
        memset(buf, 0, sizeof(buf)); 
		ret=Socket_Read(sockfd,buf,sizeof(buf)-1);

        if(ret < 0)
        {
            Error_Print("read_ser read error\n");
            exit(0);
        }
        else if(ret == 0)
        {
            printf("\nser end\n");
            fflush(stdout);
            exit(0);
        }
        unpackage(buf);//解包；
    }
}

void unpackage(char*buf)//解析查询结果数据包；
{
    QA_HEAD* change_buf;
    int number;//传回的信息的条数；
    change_buf = (QA_HEAD*)buf;
 // printf("\n******len %d\n******id %d\n", change_buf->package_len, change_buf->package_id);
    if(change_buf->package_id != 11)
    {
        return;
    }
    if(change_buf->package_len == 8)
    {
        printf("你要查询的信息不存在！\n");
    }
    else
    {
        number = (change_buf->package_len-sizeof(QA_HEAD))/224;
        
        while(number--)
        {
            printf("你要查询的信息如下:\n姓名：%s\n简拼：%s\n全拼：%s\n公司电话：%s\n私人电话：%s\n分机号：%s\nEmail：%s\n",
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->myname,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->abbreviation,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->full,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->company,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->privation,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->extension,
                                ((INFOR*)(buf+sizeof(QA_HEAD)+number*224))->emall
                                );
        }
        
    }
    printf("查找支持：中文名、简拼、全拼、公司手机号、私人号码、分机号、邮箱！\n退出系统请输入：quit\n");
    printf("input message:");//提示输入数据；
    fflush(stdout);
}
