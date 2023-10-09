
/*
用于创建并绑定两个套接字：一个UDP套接字和一个TCP套接字。这个函数的主要目的是为了准备这两个套接字，以便它们可以用于接收和发送数据。
*/

#include <stdio.h>          // 引入标准输入输出库
#include <stdlib.h>         // 引入标准库，提供一些基本功能，如内存管理和环境变量
#include <unistd.h>         // 引入Unix标准库，提供POSIX API，如read(), write() 等
#include <string.h>         // 引入字符串处理函数库
#include <sys/socket.h>     // 引入套接字编程接口
#include <netinet/in.h>     // 引入Internet地址族定义
#include <arpa/inet.h>      // 引入提供对Internet操作的接口
#include <pthread.h>        // 引入线程库
#include "public.h"         // 引入自定义的公共头文件

void creat_socket(void)     // 定义一个名为creat_socket的函数，不接受参数
{
    // 创建一个UDP套接字，udp_sock在其他地方定义的变量
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
    {
        printf("create socket error\n"); // 如果创建失败，打印错误信息
        exit(-1);                       // 退出程序，返回-1
    }

    // 创建一个TCP套接字，tcp_sock在其他地方定义的变量
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        printf("create socket error\n"); // 如果创建失败，打印错误信息
        exit(-1);                       // 退出程序，返回-1
    }

    int set = 1;                        // 定义一个整型变量set并初始化为1
    // 设置udp_sock套接字的选项，允许重用地址
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));

    // 初始化udp_sock的地址结构体
    memset(&udp_sock_addr, 0, sizeof(struct sockaddr_in)); // 将地址结构体的内存设置为0
    udp_sock_addr.sin_family = AF_INET; // 设置地址族为IPv4
    udp_sock_addr.sin_port = htons(MSG_PORT); // 设置端口号，MSG_PORT在其他地方定义，htons函数将主机字节顺序转换为网络字节顺序
    udp_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 设置地址为任意地址，允许套接字接收任何地址发来的数据

    // 初始化tcp_sock的地址结构体
    memset(&tcp_sock_addr, 0, sizeof(struct sockaddr_in)); // 将地址结构体的内存设置为0
    tcp_sock_addr.sin_family = AF_INET; // 设置地址族为IPv4
    tcp_sock_addr.sin_port = htons(MSG_PORT); // 设置端口号，MSG_PORT在其他地方定义，htons函数将主机字节顺序转换为网络字节顺序
    tcp_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 设置地址为任意地址，允许套接字接收任何地址发来的数据

    // 绑定udp_sock套接字到指定的地址和端口
    //绑定目的是限制了服务端进程创建的socket只接受那些目的地为此IP地址的客户链接，也就是说，如果不绑定，
    //那么任何客户端都可以链接到这个服务端进程创建的socket上来。
    if (bind(udp_sock, (struct sockaddr *)&udp_sock_addr, sizeof(udp_sock_addr)) == -1)
    {
        printf("bind fail1\n"); // 如果绑定失败，打印错误信息
        exit(-1);              // 退出程序，返回-1
    }

    // 绑定tcp_sock套接字到指定的地址和端口
    if (bind(tcp_sock, (struct sockaddr *)&tcp_sock_addr, sizeof(tcp_sock_addr)) == -1)
    {
        printf("bind fail\n"); // 如果绑定失败，打印错误信息
        exit(-1);             // 退出程序，返回-1
    }
}




/*
这段代码是用C语言编写的，用于创建并绑定两个套接字：一个UDP套接字和一个TCP套接字。这个函数的目的是为这两个套接字做准备，以便它们可以用于接收和发送数据。

1. 通过`#include`指令包含了一些必要的库，包括标准输入输出（`stdio.h`）、内存管理（`stdlib.h`）、Unix标准库API（`unistd.h`）、
字符串处理（`string.h`）、套接字编程接口（`sys/socket.h`）、Internet地址族定义（`netinet/in.h`）、Internet操作接口（`arpa/inet.h`）
以及线程库（`pthread.h`）和一些自定义的公共头文件。

2. 定义了一个`creat_socket`函数，它不接收任何参数。

3. 在这个函数中，首先创建一个UDP套接字。这是通过调用`socket`函数并指定地址族为`AF_INET`（IPv4），类型为`SOCK_DGRAM`（数据报，用于UDP）来完成的。
如果套接字创建失败，则打印错误信息并退出程序。

4. 然后，创建一个TCP套接字，与创建UDP套接字的方式相同，但类型指定为`SOCK_STREAM`（流，用于TCP）。如果套接字创建失败，则打印错误信息并退出程序。

5. 设置UDP套接字的选项，允许重用地址。这通常用于使服务器能够在关闭后立即重新启动，而不必等待套接字超时。

6. 初始化UDP套接字的地址结构。使用`memset`函数将结构体内存设置为0，然后设置地址族为IPv4，端口号为`MSG_PORT`（在其他地方定义），地址设置为`INADDR_ANY`（接收任何地址发来的数据）。

7. 同样，初始化TCP套接字的地址结构。

8. 接下来，使用`bind`函数将UDP套接字绑定到指定的地址和端口。绑定是为了限制服务端进程创建的套接字只接受那些目的地为此IP地址和端口的客户端连接。
如果绑定失败，则打印错误信息并退出程序。

9. 使用与UDP相同的方法将TCP套接字绑定到指定的地址和端口。
*/
