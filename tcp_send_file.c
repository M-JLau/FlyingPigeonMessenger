/*代码定义了一个用于通过TCP发送文件的函数。函数内部使用套接字在TCP连接上发送文件。当有客户端连接到服务器时，它会接收客户端发送的请求，
读取文件并通过TCP连接发送文件数据。当文件传输完成后，它会关闭文件和TCP连接。这个函数被设计为一个线程函数，可以在一个单独的线程中运行。*/

#include <stdio.h>                // 包含标准输入输出库
#include <stdlib.h>               // 包含标准库，包括内存分配、系统调用等
#include <string.h>               // 包含字符串操作函数库
#include <unistd.h>               // 包含Unix标准函数库，例如read, write等
#include <pthread.h>              // 包含线程库
#include <sys/socket.h>           // 包含套接字接口
#include <netinet/in.h>           // 包含Internet地址族
#include <arpa/inet.h>            // 包含IP地址转换函数
#include <sys/types.h>            // 包含系统数据类型定义
#include <sys/stat.h>             // 包含文件状态函数和结构
#include <fcntl.h>                // 包含文件控制函数，例如open
#include "IPMSG.H"                // 包含IPMSG相关的头文件
#include "pack_unpack.h"          // 包含pack_unpack相关的头文件
#include "userlist.h"             // 包含userlist相关的头文件
#include "filelist.h"             // 包含filelist相关的头文件
#include "public.h"               // 包含public相关的头文件

// 发送文件的函数
void *tcp_send_file()
{
    int ret=0;                      // 定义一个整型变量ret并初始化为0，用于存储返回值
    struct cmd cmd_obj;             // 定义一个cmd结构体变量cmd_obj
    char rcv[1024];                 // 定义一个字符数组rcv，大小为1024字节，用于存储接收的数据
    int con_sockt;                  // 定义一个整型变量con_sockt，用于存储连接套接字
    FILE *fd;                       // 定义一个文件指针fd
    int recvbytes;                  // 定义一个整型变量recvbytes，用于存储接收的字节数
    int len;                        // 定义一个整型变量len，用于存储长度
    struct sendfile sendfiles;      // 定义一个sendfile结构体变量sendfiles
    struct sendfile *send_files;    // 定义一个sendfile结构体指针send_files
    char recvbuf[BUFF_SIZE];        // 定义一个字符数组recvbuf，大小为BUFF_SIZE，用于存储接收的缓冲

    len=sizeof(tcp_sock_addr);      // 计算tcp_sock_addr的大小并赋值给len
    ret=listen(tcp_sock,30);        // 开始监听tcp_sock套接字，设置最大连接请求队列为30
    if(ret!=0)                      // 如果监听失败
    {
        perror("listen");           // 输出错误信息
        exit(-1);                   // 退出程序
    }
    while(1)                        // 无限循环，等待客户端连接
    {
        con_sockt=accept(tcp_sock,(struct sockaddr *)&tcp_sock_addr,&len); // 接受客户端连接，返回连接套接字
        printf("TCP连接成功\n");    // 输出TCP连接成功的消息
        recvbytes=recv(con_sockt,recvbuf,sizeof(recvbuf),0); // 接收数据，并保存接收到的字节数到recvbytes
        recvbuf[recvbytes] = '\0';  // 在接收到的数据后添加字符串结束标志
        memset(&cmd_obj, 0, sizeof(struct cmd)); // 将cmd_obj结构体清零
        sscanf(recvbuf, "%*[^:]:%[^:]:%[^:]:%[^:]:%d:%[^\n]",rcv, cmd_obj.name, cmd_obj.hostname, &cmd_obj.cmdid, cmd_obj.buf); // 从接收到的数据中解析出命令包
        if(GET_MODE(cmd_obj.cmdid) == IPMSG_GETFILEDATA) // 如果命令是获取文件数据
        {
            sscanf(cmd_obj.buf,"%lx:%d",&sendfiles.pkgnum,&sendfiles.num); // 从cmd_obj的buf中解析出包号和文件序号
            send_files=get_sendFile_num(sendfiles.num); // 根据文件序号获取要发送的文件信息
            fd=fopen(send_files->name,"rb");  // 以二进制只读方式打开文件
            if(fd == NULL)                    // 如果文件打开失败
            {
                printf("File:%s not found in current path\n",sendfiles.name); // 输出文件未找到的错误信息
                exit(-1);                      // 退出程序
            }
            char sendbuf[BUFFER_SIZE];        // 定义一个字符数组sendbuf，大小为BUFFER_SIZE，用于存储发送的缓冲
            bzero(sendbuf,BUFFER_SIZE);       // 把缓冲区清0

            int file_block_length=0;          // 定义一个整型变量file_block_length，用于存储文件块的长度
            // 每次读BUFFER_SIZE个字节，下一次读取时内部指针自动偏移到上一次读取到位置
            while((file_block_length =fread(sendbuf,sizeof(char),BUFFER_SIZE,fd))>0)
            {
                printf("%d\n",file_block_length); // 输出本次读取的字节数
                if(write(con_sockt,sendbuf,file_block_length)<0)  // 把文件块发送到连接套接字
                {
                    perror("send");              // 如果发送失败，输出错误信息
                    exit(1);                     // 退出程序
                }
                bzero(sendbuf,BUFFER_SIZE);     // 发送一次数据之后把缓冲区清零
            }
            del_sendFile_name(send_files->name); // 删除发送文件列表中的文件名
            fclose(fd);                        // 关闭文件
            printf("Transfer file finished !\n"); // 输出文件传输完成的信息
            close(con_sockt);                  // 关闭连接套接字
        }
    }
}




/*

代码用于通过TCP协议发送文件。该函数被设计为线程函数，因此可以在单独的线程中运行。

该函数的主要功能是接收客户端发送的命令包，解析出命令包中的命令，如果命令是获取文件数据，则根据文件序号获取要发送的文件信息，然后打开文件，
 读取文件内容并发送到连接套接字，直到文件读取完成，最后关闭连接套接字。

1. **包含头文件**: 包含必要的头文件以使用各种功能和数据类型。

2. **定义变量**: 函数内部定义了多个变量，包括套接字描述符、文件描述符、缓冲区等，以及自定义结构体。

3. **监听套接字**: 通过`listen`函数，代码将套接字`tcp_sock`设置为监听模式，准备接受客户端的连接请求。最大连接请求队列设置为30。如果监听失败，则输出错误信息并退出程序。

4. **等待客户端连接**: 进入无限循环，不断等待客户端的连接。一旦有客户端连接，`accept`函数接受连接并返回一个新的套接字描述符`con_sockt`。

5. **接收客户端数据**: 使用`recv`函数接收来自客户端的数据，并将其存储在`recvbuf`缓冲区中。使用`sscanf`函数解析接收到的数据，提取命令信息并存储在`cmd_obj`结构体中。

6. **检查命令类型**: 检查`cmd_obj.cmdid`是否是`IPMSG_GETFILEDATA`，这是一个标识符，表示客户端请求文件数据。

7. **打开文件并发送**: 如果客户端请求文件数据，代码从`cmd_obj.buf`中解析出包号和文件序号，并根据文件序号获取要发送的文件信息。然后，
以二进制只读方式打开文件。文件的内容被分块读取并发送到客户端。读取和发送过程在一个循环中完成，直到文件的所有内容发送完毕。

8. **清理**: 文件发送完毕后，代码从发送文件列表中删除文件名，关闭文件描述符和套接字描述符。

9. **循环继续**: 函数继续等待其他客户端的连接和请求。

*/



