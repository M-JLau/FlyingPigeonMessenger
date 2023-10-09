/*
其作用是通过UDP套接字广播一条消息。这个函数的核心部分是使用sendto函数将消息发送到一个指定的广播地址
*/

#include <stdio.h>          // 引入标准输入输出库
#include <stdlib.h>         // 引入标准库，提供一些基本功能，如内存管理和环境变量
#include <unistd.h>         // 引入Unix标准库，提供POSIX API，如read(), write() 等
#include <string.h>         // 引入字符串处理函数库
#include <sys/socket.h>     // 引入套接字编程接口
#include <netinet/in.h>     // 引入Internet地址族定义
#include <arpa/inet.h>      // 引入提供对Internet操作的接口
#include "pack_unpack.h"   // 引入自定义的头文件(可能包含数据包的打包和解包函数)
#include "IPMSG.H"         // 引入自定义的头文件(可能包含一些消息的定义)
#include "public.h"        // 引入自定义的公共头文件

void broadcast(void)         // 定义一个名为broadcast的函数，不接受参数
{
    struct sockaddr_in broad_cast_addr;     // 定义一个Internet地址结构体变量
    int sendBytes;                          // 定义一个整型变量，用于存储发送字节数
    char msg[1024];                         // 定义一个字符数组，用于存储要发送的消息

    // 设置套接字类型
    int set = 1;                           // 定义一个整型变量set并初始化为1
    // 设置udp_sock套接字的选项，允许广播，udp_sock是在其他地方定义的套接字
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &set, sizeof(set));

    // 初始化地址结构体
    memset(&broad_cast_addr, 0, sizeof(broad_cast_addr));  // 将地址结构体的内存设置为0
    broad_cast_addr.sin_family = AF_INET;  // 设置地址族为IPv4
    broad_cast_addr.sin_port = htons(MSG_PORT);            // 设置端口号，MSG_PORT在其他地方定义，htons函数将主机字节顺序转换为网络字节顺序
    inet_pton(AF_INET, "172.20.10.255", &broad_cast_addr.sin_addr); // 将点分十进制的IP地址转换为网络字节顺序，并存储在地址结构体中

    int len = sizeof(broad_cast_addr);     // 定义一个整型变量len并初始化为地址结构体的大小

    // 用户上线，打包，广播
    coding(msg, IPMSG_BR_ENTRY, myname);   // 调用coding函数，将IPMSG_BR_ENTRY和myname打包到msg中，myname在其他地方定义

    // 通过udp_sock套接字发送消息
    if ((sendBytes = sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr *)&broad_cast_addr, len)) == -1)
    {
        printf("broadcast fail\n");         // 如果发送失败，打印错误信息
        exit(-1);                          // 退出程序，返回-1
    }
}




/*
这段代码是用C语言编写的，它定义了一个函数`broadcast`，这个函数主要用于通过UDP套接字向局域网广播一个用户上线的消息。

1. 首先，程序通过`#include`指令包含了一些库。这些库提供了各种功能，包括标准输入输出（`stdio.h`）、内存管理（`stdlib.h`）、Unix标准库API（`unistd.h`）、
字符串处理（`string.h`）、套接字编程接口（`sys/socket.h`）、Internet地址族定义（`netinet/in.h`）、Internet操作接口（`arpa/inet.h`）
以及一些自定义头文件（可能包含了数据包的打包和解包函数、一些消息定义以及一些公共头文件）。

2. 在函数`broadcast`中，首先定义了一些变量。`broad_cast_addr`是一个套接字地址结构体，用于存储要广播的目标地址。`sendBytes`用于
存储发送的字节数。`msg`是一个字符数组，用于存储要发送的消息。

3. 然后，函数通过`setsockopt`设置了套接字的选项，使其允许广播。

4. 在设定地址结构体时，首先使用`memset`将其内存初始化为0，然后设定其地址族为IPv4，端口号为`MSG_PORT`（在其他地方定义），并通过`inet_pton`
函数将IP地址"10.132.22.255"转换为网络字节序。

5. 然后，函数调用了`coding`函数，这个函数的任务可能是将消息类型`IPMSG_BR_ENTRY`和用户名`myname`编码到`msg`中。

6. 最后，函数使用`sendto`函数通过套接字发送这个消息。如果发送失败，函数将打印出错误信息，并以错误码`-1`退出。
*/
