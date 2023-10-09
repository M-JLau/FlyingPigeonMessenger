/*用于处理UDP消息的函数。代码通过UDP套接字接收消息，并根据收到的消息类型（如上线通知，下线通知，消息发送等）执行相应的操作。
代码涉及套接字编程和多线程，并使用一些自定义的函数和结构来实现功能。*/


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

struct cmd cmd_obj;                // 定义一个cmd结构体变量cmd_obj，用于存储命令信息
char recvbuf[BUFF_SIZE];           // 定义一个字符数组recvbuf，大小为BUFF_SIZE，用于存储接收的数据

// udp消息处理函数，参数包括一个指向cmd结构体的指针和一个指向sockaddr_in结构体的指针
void udp_msg_handle(struct cmd *msg, struct sockaddr_in* send_addr)
{
    unsigned long tmp = 0;           // 定义一个无符号长整型变量tmp，并初始化为0

    // 如果接收到的消息是用户广播上线信息
    if (GET_MODE(msg->cmdid) == IPMSG_BR_ENTRY)
    {
        // 如果用户列表中没有此用户，则添加此用户
        // 通过sin_addr判断
        if (getUser(&send_addr->sin_addr) == NULL)
        {
            addUser(&send_addr->sin_addr, msg->name, msg->hostname);
        }
        char buff[BUFF_SIZE];        // 定义一个字符数组buff，大小为BUFF_SIZE，用于存储编码后的数据
        memset(buff, 0, sizeof(buff)); // 将buff数组清零

        // 发送IPMSG_ANSENTRY信息
        coding(buff, IPMSG_ANSENTRY, myname);
        int sendBytes;
        // 使用sendto函数发送数据
        if ((sendBytes = sendto(udp_sock, buff, strlen(buff), 0,
                                (struct sockaddr *)send_addr, sizeof(struct sockaddr))) == -1)
        {
            printf("sendto fail\n"); // 如果发送失败，输出错误信息
        }
    }

    // 如果接收到应答上线信息
    if (GET_MODE(msg->cmdid) == IPMSG_ANSENTRY)
    {
        // 如果用户列表中没有此用户，则添加此用户
        // 通过sin_addr判断
        if (getUser(&send_addr->sin_addr) == NULL)
        {
            addUser(&send_addr->sin_addr, msg->buf, msg->hostname);
        }
    }

    // 如果接收到用户下线信息
    if (GET_MODE(msg->cmdid) == IPMSG_BR_EXIT)
    {
        // 如果用户列表中有此用户，则删除此用户
        struct user* logout_user;
        if ((logout_user = getUser(&send_addr->sin_addr)) != NULL)
        {
            // 通过sin_addr删除用户
            delUser(send_addr->sin_addr);
        }
    }

    // 如果接收到消息
    if (GET_MODE(msg->cmdid) == IPMSG_SENDMSG)
    {
        char codingbuff[BUFF_SIZE];  // 定义一个字符数组codingbuff，大小为BUFF_SIZE，用于存储编码后的数据
        if ((msg->cmdid & IPMSG_SENDCHECKOPT) == IPMSG_SENDCHECKOPT)
        {
            coding(codingbuff, IPMSG_RECVMSG, msg->id);
            // 使用sendto函数发送数据
            sendto(udp_sock, codingbuff, strlen(codingbuff), 0,
                   (struct sockaddr *)&udp_sock_addr, sizeof(udp_sock_addr));
        }
        printf("[recv msg from: %s :%s]#", msg->name, inet_ntoa(send_addr->sin_addr));
        printf("%s\n", msg->buf);   // 输出接收到的消息
    }

    // 如果接收到文件
    if ((msg->cmdid & IPMSG_FILEATTACHOPT) == IPMSG_FILEATTACHOPT)
    {
        char codingbuff[BUFF_SIZE];  // 定义一个字符数组codingbuff，大小为BUFF_SIZE，用于存储编码后的数据
        coding(codingbuff, IPMSG_RECVMSG, msg->id);
        // 使用sendto函数发送数据
        sendto(udp_sock, codingbuff, strlen(codingbuff), 0,
               (struct sockaddr *)&udp_sock_addr, sizeof(udp_sock_addr));

        struct rcvfile rcvfiles;     // 定义一个rcvfile结构体变量rcvfiles，用于存储接收到的文件信息
        memset(&rcvfiles, 0, sizeof(rcvfiles)); // 将rcvfiles结构体清零
        rcvfiles.sin_addr = udp_sock_addr.sin_addr;

        char *p1, *p2, i, *pp;
        p1 = strrchr(recvbuf, 0);   // 查找recvbuf中最后一个字符0的位置
        printf("接收到包含文件信息的UDP数据包:%s \n", recvbuf);
        //printf("接收到的数据包解析：%s\n",p1);
        p2 = (p1 + 1);              // p2指向p1之后的位置
        //printf("接收到的数据包再解析：%s\n",p2);
        //printf("p2: %s\n", p2);
        sscanf(p2, "%lx:%[^:]:%lx", &rcvfiles.num, rcvfiles.name, &rcvfiles.size); // 从p2中解析文件信息

        pp = strtok(p2, ":");      // 使用strtok函数分割p2字符串
        for (i = 0; i < strlen(pp); i++)
        {
            tmp = tmp * 10 + (*p2 - 0x30);  // 将字符转换为数字
            p2++;
        }
        rcvfiles.num = tmp;
        printf("用户: %s向您发送文件：", inet_ntoa(udp_sock_addr.sin_addr));
        printf("%s\n", rcvfiles.name); // 输出文件名
        add_rcvFile(&rcvfiles.sin_addr, rcvfiles.name, rcvfiles.num, rcvfiles.size); // 将接收到的文件信息添加到文件列表中
    }
    /*if ((msg->cmdid & IPMSG_FILEATTACHOPT) == IPMSG_FILEATTACHOPT)
    {
        char codingbuff[BUFF_SIZE];  // 定义一个字符数组codingbuff，大小为BUFF_SIZE，用于存储编码后的数据
        coding(codingbuff, IPMSG_RECVMSG, msg->id);
        // 使用sendto函数发送数据
        sendto(udp_sock, codingbuff, strlen(codingbuff), 0,
               (struct sockaddr *)&udp_sock_addr, sizeof(udp_sock_addr));

        struct rcvfile rcvfiles;     // 定义一个rcvfile结构体变量rcvfiles，用于存储接收到的文件信息
        memset(&rcvfiles, 0, sizeof(rcvfiles)); // 将rcvfiles结构体清零
        rcvfiles.sin_addr = udp_sock_addr.sin_addr;

        char *p1, *p2, i, *pp;
        p1 = strrchr(recvbuf, 0);   // 查找recvbuf中最后一个字符0的位置

        // 检测是从 Linux 还是 Windows 发送的
        if (strstr(msg->buf, "DESKTOP-") != NULL) // 假设Windows的计算机名包含"DESKTOP-"
        {
            // 来自 Windows 的处理方式
            printf("接收到包含文件信息的UDP数据包:%s \n", recvbuf);
            printf("接收到的数据包解析：%s\n", p1);
            p2 = (p1 + 1);              // p2指向p1之后的位置
            printf("接收到的数据包再解析：%s\n", p2);
            sscanf(p2, "%lx:%[^:]:%lx", &rcvfiles.num, rcvfiles.name, &rcvfiles.size); // 从p2中解析文件信息
        }
        else
        {
            // 来自 Linux 的处理方式
            p2 = msg->buf; // 直接读取buf区的信息
            sscanf(p2, "%*[^:]:%[^:]:%*[^:]:%lx", rcvfiles.name, &rcvfiles.size); // 从p2中解析文件信息
        }

        printf("用户: %s向您发送文件：", inet_ntoa(udp_sock_addr.sin_addr));
        printf("%s\n", rcvfiles.name); // 输出文件名
        add_rcvFile(&rcvfiles.sin_addr, rcvfiles.name, rcvfiles.num, rcvfiles.size); // 将接收到的文件信息添加到文件列表中
    }*/
}

// UDP消息处理线程函数
void *udp_msg_process()
{
    int addrLen = sizeof(udp_sock_addr);  // 定义整型变量addrLen，并初始化为udp_sock_addr的大小
    int recvbytes;                        // 定义整型变量recvbytes，用于存储接收到的字节数
    memset(&udp_sock_addr, 0, sizeof(struct sockaddr_in));  // 将udp_sock_addr结构体清零

    while (1) // 无限循环
    {
        // 接收用户信息, 用来接收任何来源的广播信息
        if ((recvbytes = recvfrom(udp_sock, recvbuf, sizeof(recvbuf), 0,
                                  (struct sockaddr *)&udp_sock_addr, &addrLen)) != -1)
        {
            recvbuf[recvbytes] = '\0'; // 在接收到的数据末尾添加结束字符

            memset(&cmd_obj, 0, sizeof(struct cmd)); // 将cmd_obj结构体清零
            transcode(&cmd_obj, recvbuf, recvbytes); // 调用transcode函数对接收到的数据进行解码

            udp_msg_handle(&cmd_obj, &udp_sock_addr); // 调用udp_msg_handle函数处理解码后的命令
        }
        else
        {
            printf("UDP接收失败\n");  // 如果接收失败，输出错误信息
        }
    }
}





/*
这段代码是用于处理通过UDP协议接收到的消息。它包含两个主要的函数：`udp_msg_handle`和`udp_msg_process`。其中`udp_msg_process`函数
是一个线程函数，用于接收数据，而`udp_msg_handle`则是用于处理接收到的数据。

1. 首先，包含了一系列需要的头文件，以及几个自定义头文件（如"IPMSG.H", "pack_unpack.h"等）。

2. 定义了一个结构体变量`cmd_obj`用于存储命令信息，和一个字符数组`recvbuf`用于存储接收到的数据。

3. `udp_msg_handle`函数：
- 参数：一个指向`cmd`结构体的指针`msg`，和一个指向`sockaddr_in`结构体的指针`send_addr`。
- 函数内首先定义了一个无符号长整型变量`tmp`并初始化为0。
- 检查接收到的消息是否是用户上线广播信息，如果是则将此用户添加到用户列表中（如果不在列表中），并发送回应信息。
- 检查接收到的消息是否是应答上线信息，如果是则将此用户添加到用户列表中（如果不在列表中）。
- 检查接收到的消息是否是用户下线信息，如果是则将此用户从用户列表中删除。
- 检查接收到的消息是否是常规消息，如果是则在控制台上输出接收到的消息。
- 检查接收到的消息是否包含文件，如果是则输出文件信息。

4. `udp_msg_process`函数（一个线程函数）：
- 定义了整型变量`addrLen`和`recvbytes`。
- 通过一个无限循环，持续接收消息。
- 使用`recvfrom`函数接收UDP数据包。如果成功接收到数据，则将接收到的数据传递给`udp_msg_handle`函数进行处理。
- 如果接收失败，输出错误信息。

这段代码的主要目的是接收和处理通过UDP协议发送的消息。它可以处理多种类型的消息，包括用户上线/下线通知、普通消息和文件。对于每种类型的消息，
代码中有特定的处理逻辑。在无限循环中，它不断监听UDP数据包，并将接收到的数据包传递给处理函数。*/
