/*
允许用户在局域网内发送和接收消息和文件。它基于UDP和TCP协议。
*/



#include <stdio.h>          // 包含标准输入输出库
#include <unistd.h>         // 包含Unix标准库, 用于read()函数
#include <sys/socket.h>     // 包含socket编程的相关库
#include <stdlib.h>         // 包含标准库函数, 如malloc()
#include <netinet/in.h>     // 包含Internet地址族定义
#include <arpa/inet.h>      // 包含IP地址转换函数
#include <string.h>         // 包含字符串操作函数
#include <pthread.h>        // 包含线程库
#include <sys/types.h>      // 包含系统数据类型定义
#include <sys/stat.h>       // 包含文件状态定义
#include <time.h>           // 包含时间处理函数
#include <fcntl.h>          // 包含文件控制函数
#include "IPMSG.H"          // 包含自定义IPMSG相关头文件
#include "userlist.h"       // 包含自定义用户列表相关头文件
#include "pack_unpack.h"    // 包含自定义打包和解包相关头文件
#include "filelist.h"       // 包含自定义文件列表相关头文件
#include "public.h"         // 包含自定义公共头文件

// 声明函数原型
void help_cmd(void);                    // 显示帮助信息
void ls_cmd(void);                      // 查看在线用户
void sendto_cmd(char *input_name);      // 发送消息给指定用户
void send_BR_EXIT(void);                // 发送退出广播
void ls_sendfile(void);                 // 查看发送文件列表
void ls_rcvfile(void);                  // 查看接收文件列表
void sendfile_cmd(char *input_name);    // 向指定用户发送文件
void getfile_cmd(char *input_name);     // 从指定用户接收文件

// 扫描键盘
void *key_scan_process()                // 定义一个名为 key_scan_process 的函数，用于扫描键盘输入，没有参数，返回指针类型
{
    char buf[BUFF_SIZE];                 // 定义一个字符数组 buf，大小为 BUFF_SIZE，用于存储键盘输入的内容
    int recvBytes;                       // 定义一个整数变量 recvBytes，用于存储读取到的字节数
    while(1)                             // 开始一个无限循环，持续扫描键盘输入
    {
        printf("ipmsg> ");               // 在屏幕上打印提示符 "ipmsg> "
        memset(&buf, 0, sizeof(buf));    // 将 buf 数组的内容清零，以准备存储新的输入

        // 读取键盘输入
        if ((recvBytes = read(1, buf, sizeof(buf))) != -1 ) // 如果成功读取键盘输入，则将读取到的字节数存储在 recvBytes 变量中
        {
            buf[recvBytes-1] = '\0';     // 将 buf 数组的最后一个字符设置为字符串结束符，以确保它是一个有效的字符串

            // 根据输入内容执行相应的命令
            if (strncmp(buf, "sendto", 6) == 0)               // 如果输入以 "sendto" 开头
            {
                sendto_cmd(&buf[7]);       // 调用 sendto_cmd 函数，传递输入字符串中的用户名部分
            }
            else if (strcmp(buf, "ls") == 0)                  // 如果输入的是 "ls"
            {
                ls_cmd();                 // 调用 ls_cmd 函数，显示在线用户列表
            }
            else if (strcmp(buf, "help") == 0)                // 如果输入的是 "help"
            {
                help_cmd();               // 调用 help_cmd 函数，显示帮助信息
            }
            else if (strncmp(buf, "getfile", 7) == 0)         // 如果输入以 "getfile" 开头
            {
                getfile_cmd(&buf[8]);     // 调用 getfile_cmd 函数，传递输入字符串中的文件名部分
            }
            else if (strncmp(buf, "sendfile", 8) == 0)        // 如果输入以 "sendfile" 开头
            {
                sendfile_cmd(&buf[9]);    // 调用 sendfile_cmd 函数，传递输入字符串中的用户名部分
            }
            else if (strcmp(buf, "SFL") == 0)                 // 如果输入的是 "SFL"
            {
                ls_sendfile();            // 调用 ls_sendfile 函数，显示发送文件列表
            }
            else if (strcmp(buf, "RFL") == 0)                 // 如果输入的是 "RFL"
            {
                ls_rcvfile();             // 调用 ls_rcvfile 函数，显示接收文件列表
            }
            else if (strcmp(buf, "exit") == 0)                // 如果输入的是 "exit"
            {
                send_BR_EXIT();           // 调用 send_BR_EXIT 函数，发送退出广播
                break;                    // 退出无限循环
            }
        }
    }
    pthread_exit(NULL);                   // 结束线程执行
}

void help_cmd(void)                     // 定义一个名为 help_cmd 的函数，用于显示帮助信息
{
    printf("\n*******************************************************\n"); // 打印分隔线
    printf("\t显示好友列表:\t\tls\n");                             // 打印“显示好友列表”的命令提示
    printf("\t给好友发送消息:\t\tsendto[空格]好友名\n");             // 打印“给好友发送消息”的命令提示
    printf("\t显示发送文件列表:\tSFL\n");                            // 打印“显示发送文件列表”的命令提示
    printf("\t显示接收文件列表:\tRFL\n");                            // 打印“显示接收文件列表”的命令提示
    printf("\t发送文件:\t\tsendfile[空格]好友名\n");                // 打印“发送文件”的命令提示
    printf("\t接收文件:\t\tgetfile[空格]文件名\n");                 // 打印“接收文件”的命令提示
    printf("\t退出程序:\t\texit\n");                                // 打印“退出程序”的命令提示
    printf("*********************************************************\n\n"); // 打印分隔线
}

void ls_cmd(void) // 定义一个名为 ls_cmd 的函数，用于遍历并显示在线用户列表，没有参数和返回值
{
    struct user* cur = userlist_head->next; // 定义一个 struct user 类型的指针 cur，并将其指向用户列表的第一个元素
    printf("\n***************************************************\n"); // 打印分隔线
    printf("****\t好友列表:                                          ****\n"); // 打印“好友列表”标题
    while (cur != NULL) // 当 cur 不为空时，即列表未结束时，循环执行
    {
        printf("****\tname=%s", cur->name); // 打印当前用户的名字
        printf("\tip=%s \t\t           ****\n", inet_ntoa(cur->sin_addr)); // 打印当前用户的 IP 地址
        cur = cur->next; // 将 cur 指向下一个用户
    }
    printf("******************************************************\n"); // 打印分隔线

    return; // 函数返回
}

// 发送消息
void sendto_cmd(char *input_name) // 定义一个名为 sendto_cmd 的函数，用于发送消息给指定的用户
{
    struct user *cur_user = getUser_name(input_name); // 通过用户名获取用户的信息，并将信息存储在 cur_user 中

    if (cur_user == NULL) // 如果用户不存在
    {
        printf("该用户不存在!\n"); // 打印错误信息
        return; // 函数返回
    }

    char buff[BUFF_SIZE], codingbuff[BUFF_SIZE]; // 定义两个字符数组 buff 和 codingbuff，用于存储消息

    int set = 1, sendBytes, recvBytes; // 定义三个整数变量 set, sendBytes 和 recvBytes
    struct sockaddr_in send_sock_addr; // 定义一个 sockaddr_in 结构体变量，用于存储目标地址信息
    memset(&send_sock_addr, 0, sizeof(struct sockaddr_in)); // 将 send_sock_addr 的内容清零
    send_sock_addr.sin_family = AF_INET; // 设置地址族为 AF_INET
    send_sock_addr.sin_port = htons(MSG_PORT); // 设置端口号
    send_sock_addr.sin_addr = cur_user->sin_addr; // 设置目标 IP 地址

    while(1) // 开始一个无限循环，用于发送消息
    {
        printf("[to %s]:", cur_user->name ); // 打印提示信息，显示正在给谁发送消息
        memset(&buff, 0, sizeof(buff)); // 将 buff 数组的内容清零

        // 读取键盘输入，即消息内容
        if ((recvBytes = read(0, buff, sizeof(buff)) ) != -1 ) // 如果读取成功
        {
            buff[recvBytes-1] = '\0'; // 在字符串末尾添加空字符，以形成一个合法字符串
            if (strcmp(buff, "exit") != 0) // 如果输入的不是 "exit"
            {
                if (recvBytes <= 1) continue; // 如果接收到的消息长度小于等于1，则跳过当前循环
                coding(codingbuff, IPMSG_SENDMSG, buff); // 调用 coding 函数，对消息进行编码
                // 将编码后的消息发送出去
                if((sendBytes = sendto(udp_sock, codingbuff, strlen(codingbuff), 0,
                                       (struct sockaddr *)&send_sock_addr, sizeof(struct sockaddr))) == -1)
                {
                    printf("sendto fail\n"); // 如果发送失败，则打印错误信息
                }
            }
            else // 如果输入的是 "exit"
            {
                return; // 结束函数
            }
        }
    }
}

// 退出程序
void send_BR_EXIT(void) // 定义一个名为 send_BR_EXIT 的函数，用于发送退出广播
{
    struct sockaddr_in exit_send_addr; // 定义一个 sockaddr_in 结构体变量，用于存储目标地址信息
    int sendBytes; // 定义一个整数变量，用于存储发送字节的数量
    char buff[BUFF_SIZE]; // 定义一个字符数组，用于存储消息
    memset(&buff, 0, sizeof(buff)); // 将 buff 数组的内容清零

    // 设置套接字类型
    int set = 1; // 设置广播和地址重用选项
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &set, sizeof(set));

    memset(&exit_send_addr, 0, sizeof(exit_send_addr)); // 将 exit_send_addr 的内容清零
    exit_send_addr.sin_family = AF_INET; // 设置地址族为 AF_INET
    exit_send_addr.sin_port = htons(MSG_PORT); // 设置端口号
    inet_pton(AF_INET, "172.20.10.255", &exit_send_addr.sin_addr); // 设置广播地址

    int len = sizeof(exit_send_addr); // 获取地址结构体的大小
    coding(buff, IPMSG_BR_EXIT, NULL); // 调用 coding 函数，对消息进行编码

    // 发送广播消息
    if((sendBytes = sendto(udp_sock, buff, strlen(buff), 0, (struct sockaddr *)&exit_send_addr, len)) == -1)
    {
        printf("sendto fail\n"); // 如果发送失败，则打印错误信息
    }
}


// 查看发送文件列表
void ls_sendfile(void) // 定义一个名为 ls_sendfile 的函数，用于遍历并显示发送文件列表，没有参数和返回值
{
    struct sendfile* file = send_filelist_head->next; // 定义一个 struct sendfile 类型的指针 file，并将其指向发送文件列表的第一个元素
    printf("\n***************************************************\n"); // 打印分隔线
    printf("****\t发送文件列表:                                          \n"); // 打印“发送文件列表”标题
    while (file != NULL) // 当 file 不为空时，即列表未结束时，循环执行
    {
        printf("****\tname=%s", file->name); // 打印当前文件的名字
        printf("\tip=%s \t\t           \n", inet_ntoa(file->sin_addr)); // 打印当前文件的 IP 地址
        file = file->next; // 将 file 指向下一个文件
    }
    printf("******************************************************\n"); // 打印分隔线
    return; // 函数返回
}

// 查看接收文件列表
void ls_rcvfile(void) // 定义一个名为 ls_rcvfile 的函数，用于遍历并显示接收文件列表，没有参数和返回值
{
    struct rcvfile* file = rcv_filelist_head->next; // 定义一个 struct rcvfile 类型的指针 file，并将其指向接收文件列表的第一个元素
    printf("\n***************************************************\n"); // 打印分隔线
    printf("****\t接收文件列表:                                          \n"); // 打印“接收文件列表”标题
    while (file != NULL) // 当 file 不为空时，即列表未结束时，循环执行
    {
        printf("****\tname=%s", file->name); // 打印当前文件的名字
        printf("\tip=%s \t\t           \n", inet_ntoa(file->sin_addr)); // 打印当前文件的 IP 地址
        file = file->next; // 将 file 指向下一个文件
    }
    printf("******************************************************\n"); // 打印分隔线
    return; // 函数返回
}

// 接收文件
void getfile_cmd(char *input_name) // 定义一个名为 getfile_cmd 的函数，用于接收指定的文件，接受一个参数 input_name，表示文件的名字
{
    printf("开始接收文件\n"); // 打印提示信息
    char cmd_buf[BUFFER_SIZE]; // 定义一个字符数组 cmd_buf，用于存储命令缓冲
    int con_sockt; // 定义一个整数变量，表示连接的套接字
    struct rcvfile *file = get_rcvFile_name(input_name); // 通过文件名获取文件的信息，并将信息存储在 file 中
    int ren=0; // 定义一个整数变量 ren，初始化为0
    int len=0; // 定义一个整数变量 len，初始化为0

    if(file == NULL) // 如果文件不存在
    {
        printf("该文件不存在!\n"); // 打印错误信息
        return; // 函数返回
    }
    FILE* fd; // 定义一个 FILE 指针 fd，用于文件操作
    if((fd=fopen(input_name,"rb+"))==NULL) // 尝试以读/写方式打开文件，如果文件不存在
    {
        int sockfd; // 定义一个整数变量 sockfd，表示套接字描述符
        int ren; // 重新定义一个整数变量 ren
        struct sockaddr_in myaddr; // 定义一个 sockaddr_in 结构体变量 myaddr，用于存储地址信息
        // 连接设置
        myaddr.sin_family=AF_INET; // 设置地址族为 AF_INET
        myaddr.sin_port=htons(MSG_PORT); // 设置端口号
        // 通过文件列表获取文件名和文件ip
        char *server_ip=inet_ntoa(file->sin_addr); // 获取文件的 IP 地址
        inet_pton(AF_INET,server_ip,&myaddr.sin_addr); // 将 IP 地址转换为网络字节序并存储在 myaddr 中
        // 创建套接字
        sockfd=socket(AF_INET,SOCK_STREAM,0); // 创建一个套接字，并将套接字描述符存储在 sockfd 中
        if(sockfd<0) // 如果创建套接字失败
        {
            perror("socket"); // 打印错误信息
            exit(-1); // 退出程序
        }
        char sendbuf[BUFFER_SIZE]; // 定义一个字符数组 sendbuf，用于存储发送的数据
        printf("开始连接\n"); // 打印提示信息
        ren=connect(sockfd,(struct sockaddr *)&myaddr,sizeof(myaddr)); // 连接到服务器
        if(ren!=0) // 如果连接失败
        {
            perror("connect"); // 打印错误信息
        }
        bzero(cmd_buf,BUFFER_SIZE); // 将 cmd_buf 清零
        char codingbuff[BUFF_SIZE]; // 定义一个字符数组 codingbuff，用于存储编码后的数据
        // 打包发送消息
        int sendBytes; // 定义一个整数变量 sendBytes，用于存储发送的字节数
        sprintf(cmd_buf,"%lx:%lx:%d",file->pkgnum,file->num,0); // 将文件的包号和编号等信息格式化存储在 cmd_buf 中
        coding(codingbuff,IPMSG_GETFILEDATA,cmd_buf); // 调用 coding 函数，将 cmd_buf 的内容进行编码，并存储在 codingbuff 中
        // 发送数据
        if((sendBytes = send(sockfd, codingbuff,strlen(codingbuff),0)) == -1) // 如果发送失败
        {
            printf("send fail\n"); // 打印错误信息
            return; // 函数返回
        }
        if((fd=fopen(input_name,"wb+"))==NULL) // 尝试以写方式打开文件，如果文件不存在或打开失败
        {
            perror("open"); // 打印错误信息
            exit(-1); // 退出程序
        }
        else // 如果文件打开成功
        {
            char sendbuf[BUFFER_SIZE]; // 重新定义一个字符数组 sendbuf，用于存储发送的数据
            int size=0; // 定义一个整数变量 size，用于存储接收到的数据的大小
            int  recv_len = 0; // 定义一个整数变量 recv_len，用于存储接收到的数据的长度
            do
            {
                bzero(sendbuf,sizeof(sendbuf)); // 将 sendbuf 清零
                recv_len = recv(sockfd,sendbuf,sizeof(sendbuf),0); // 从套接字接收数据，并存储在 sendbuf 中
                size = size + recv_len; // 更新接收到的数据的总大小
                fwrite(sendbuf,sizeof(char),recv_len,fd); // 将接收到的数据写入文件
                printf("***size = %d***file->size=%ld***\n",size,file->size); // 打印接收到的数据的大小和文件的大小

            } while(size < (file->size)); // 当接收到的数据的大小小于文件的大小时，继续循环接收数据

            if(size == file->size) // 如果接收到的数据的大小等于文件的大小
            {
                printf("successfully receive file! \n"); // 打印成功接收文件的信息
            }
            else // 否则
            {
                printf("cannot receive file !!! \n"); // 打印无法接收文件的信息
            }
            del_rcvFile_name(file->name); // 从接收文件列表中删除该文件
            fclose(fd); // 关闭文件
            close(sockfd); // 关闭套接字
            return; // 函数返回
        }
    }
}


//发送文件
void sendfile_cmd(char *input_name)
{
    struct user *cur_user = getUser_name(input_name); // 通过用户名获取用户信息，保存在 cur_user 结构体指针中

    if (cur_user == NULL) // 如果 cur_user 为空，表示没有找到该用户
    {
        printf("该用户不存在!\n"); // 输出错误信息
        return; // 返回，不执行后续代码
    }
    int ren; // 定义一个整数变量 ren，用于保存返回值
    struct sockaddr_in myaddr; // 定义一个 sockaddr_in 结构体变量 myaddr，用于存储地址信息
    char *server_ip=inet_ntoa(cur_user->sin_addr); // 将网络字节序的 IP 地址转换为点分十进制字符串
    char buff[BUFF_SIZE], codingbuff[BUFF_SIZE]; // 定义两个字符数组 buff 和 codingbuff，用于存储数据
    int sendBytes, recvBytes; // 定义两个整数变量 sendBytes 和 recvBytes，用于存储发送和接收的字节数
    inet_pton(AF_INET,server_ip,&myaddr.sin_addr); // 将 IP 地址转换为网络字节序并存储在 myaddr 中
    printf("[to %s]:", cur_user->name); // 输出信息，提示将要发送给指定用户
    memset(&buff, 0, sizeof(buff)); // 将 buff 数组清零

    // 接收键盘输入信息
    if ((recvBytes = read(0, buff, sizeof(buff))) != -1 ) // 读取键盘输入，保存在 buff 中
    {
        buff[recvBytes-1] = '\0'; // 将最后一个字符设置为结束符 '\0'
        FILE *fd; // 定义一个 FILE 指针 fd，用于文件操作
        // 以二进制读模式打开文件
        fd=fopen(buff,"rb");
        if(fd == NULL) // 如果文件打开失败
        {
            printf("File:%s not found in current path\n",buff); // 输出文件未找到的错误信息
            exit(-1); // 退出程序
        }
        // 将文件添加到发送文件列表
        add_sendFile(&myaddr.sin_addr,buff);
        struct sendfile* sendfiles; // 定义一个 sendfile 结构体指针 sendfiles
        sendfiles=get_sendFile_name(buff); // 通过文件名获取发送文件信息，保存在 sendfiles 中

        if (strcmp(buff, "exit") != 0) // 如果输入的不是 "exit"
        {
            memset(&udp_sock_addr, 0, sizeof(struct sockaddr_in)); // 将 udp_sock_addr 结构体清零
            udp_sock_addr.sin_family = AF_INET; // 设置地址族为 AF_INET
            udp_sock_addr.sin_port = htons(MSG_PORT); // 设置端口号
            udp_sock_addr.sin_addr = cur_user->sin_addr; // 设置 IP 地址
            char cmd_tmp[BUFF_SIZE]; // 定义一个字符数组 cmd_tmp，用于存储临时数据
            char num[BUFF_SIZE]; // 定义一个字符数组 num，用于存储数字
            sprintf(cmd_tmp,"%d:%s:%lx:%lx:%lx",sendfiles->num,buff,sendfiles->size,sendfiles->ltime,IPMSG_FILE_REGULAR); // 将格式化的数据写入 cmd_tmp
            coding(codingbuff,IPMSG_SENDMSG|IPMSG_SENDCHECKOPT|IPMSG_FILEATTACHOPT,NULL); // 编码数据
            sprintf(num,"%s%c%s",codingbuff,'0',cmd_tmp); // 将格式化的数据写入 num
            // 发送数据
            if((sendBytes = sendto(udp_sock, num, strlen(num), 0,
                                   (struct sockaddr *)&udp_sock_addr, sizeof(udp_sock_addr))) == -1)
            {
                printf("sendto fail\n"); // 如果发送失败，输出错误信息
                return; // 返回，不执行后续代码
            }
            fclose(fd); // 关闭文件
            return; // 返回，不执行后续代码
        }
        else // 如果输入的是 "exit"
        {
            del_sendFile_name(buff); // 从发送文件列表中删除该文件
            return; // 返回，不执行后续代码
        }
    }
}





/*

1. **#include指令**: 导入了一系列头文件，以使用标准输入输出、网络、线程和时间处理相关的功能。

2. **函数声明**: 声明了几个函数，这些函数的实现在代码后面或其他文件中。

3. **key_scan_process 函数**: 这个函数在一个无限循环中，从键盘读取用户的输入并根据输入执行相应的操作。
- 用于输入的缓冲区`buf`被清空。
- 通过`read`函数从标准输入（键盘）读取数据到`buf`中。
- 根据读取到的内容执行不同的操作，如`sendto`，`ls`，`help`，`getfile`，`sendfile`等。

4. **help_cmd 函数**: 显示帮助信息，列出了用户可以使用的所有命令及其说明。

5. **ls_cmd 函数**: 列出在线用户。它遍历一个用户链表并打印出每个用户的名字和IP地址。

6. **sendto_cmd 函数**: 向指定用户发送消息。
- 首先通过名字查找用户。
- 如果用户不存在，打印错误消息。
- 在一个循环中，从键盘读取要发送的消息，并通过UDP socket将其发送给目标用户。
- 可以输入`exit`来退出发送消息模式。

7. **send_BR_EXIT 函数**: 当程序退出时，它向网络广播一个退出消息。

8. **ls_sendfile 函数**: 列出要发送的文件列表。它遍历一个发送文件链表并打印每个文件的名字和IP地址。

9. **ls_rcvfile 函数**: 列出要接收的文件列表。类似于`ls_sendfile`，但是遍历的是接收文件链表。

10. **getfile_cmd 函数**: 用于接收文件。
- 根据输入的名字找到文件。
- 如果文件不存在，打印错误消息并返回。
- 创建一个TCP套接字并连接到发送方。
- 通过套接字接收文件数据，并将数据写入本地文件。

11. **sendfile_cmd 函数**: 用于发送文件。
- 首先通过名字查找用户。
- 如果用户不存在，打印错误消息并返回。
- 从键盘读取要发送的文件的名字，并打开该文件。
- 添加文件到发送文件列表。
- 通过UDP socket发送一个包含文件信息的消息给目标用户。

*/
