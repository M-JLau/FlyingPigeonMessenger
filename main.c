/*
程序允许用户输入昵称，并通过网络进行广播和文件传输。该程序使用UDP和TCP套接字，并使用线程来并行处理不同的任务（如接收消息、扫描键盘输入和发送文件）。
*/


#include <stdio.h>            // 引入标准输入输出库
#include <pthread.h>           // 引入POSIX线程库
#include <stdlib.h>            // 引入标准库，例如exit函数
#include <netinet/in.h>        // 引入Internet地址族库
#include <unistd.h>            // 引入POSIX标准库，例如read, write等
#include <string.h>            // 引入字符串操作库
#include "create_socket.h"     // 引入自定义头文件（创建套接字相关）
#include "userlist.h"          // 引入自定义头文件（用户列表相关）
#include "broadcast.h"         // 引入自定义头文件（广播相关）
#include "filelist.h"          // 引入自定义头文件（文件列表相关）

char myname[20];               // 定义一个字符数组来存储用户昵称
int udp_sock;                  // 定义UDP套接字的文件描述符
int tcp_sock;                  // 定义TCP套接字的文件描述符
struct sockaddr_in udp_sock_addr; // 定义UDP套接字的地址结构
struct sockaddr_in tcp_sock_addr; // 定义TCP套接字的地址结构

struct rcvfile *rcv_filelist_head = NULL;    // 接收文件列表的头指针
struct sendfile *send_filelist_head = NULL;  // 发送文件列表的头指针
struct user *userlist_head = NULL;           // 用户列表的头指针

void *udp_msg_process();           // 声明UDP消息处理函数
void *key_scan_process();          // 声明键盘扫描处理函数
void *tcp_send_file();             // 声明TCP发送文件函数

int main()                         // 程序入口点
{
    pthread_t udp_rcv, key_scan, tcp_send;   // 定义三个线程ID
    int ret = 0;                             // 定义返回值变量并初始化为0

    setbuf(stdin, NULL);                     // 设置标准输入无缓冲
    setbuf(stdout, NULL);                    // 设置标准输出无缓冲

    puts("请输入昵称:");                     // 打印提示信息
    fgets(myname, 20, stdin);                // 从标准输入读取昵称
    myname[strlen(myname)-1] = '\0';         // 去掉换行符

    init_userlist();                         // 初始化用户列表
    init_rcv_filelist();                     // 初始化接收文件列表
    init_send_filelist();                    // 初始化发送文件列表

    creat_socket();                          // 创建网络服务
    broadcast();                             // 广播上线信息

    ret = pthread_create(&udp_rcv, 0, udp_msg_process, NULL); // 创建处理UDP消息的线程
    if (ret != 0)                            // 检查线程创建是否成功
    {
        perror("pthread_create");            // 打印错误信息
        exit(-1);                            // 退出程序
    }
    ret = pthread_create(&key_scan, 0, key_scan_process, NULL); // 创建处理键盘扫描的线程
    if (ret != 0)                            // 检查线程创建是否成功
    {
        perror("pthread_create");            // 打印错误信息
        exit(-1);                            // 退出程序
    }
    ret = pthread_create(&tcp_send, 0, tcp_send_file, NULL); // 创建发送文件的线程
    if (ret != 0)                            // 检查线程创建是否成功
    {
        perror("pthread_create");            // 打印错误信息
        exit(-1);                            // 退出程序
    }

    pthread_join(udp_rcv, NULL);             // 等待处理UDP消息的线程结束
    pthread_join(key_scan, NULL);            // 等待处理键盘扫描的线程结束
    pthread_join(tcp_send, NULL);            // 等待发送文件的线程结束

    return 0;                                // 返回0，表示程序正常退出
}


/*

基于UDP和TCP的多线程聊天和文件传输程序。它的工作流程是这样的：首先，用户输入自己的昵称，然后程序创建套接字并广播用户上线信息，之后创建3个线程并行
处理接收UDP消息，扫描键盘输入和发送文件的任务。

这里详细解释一下关键代码部分：

1. `#include`部分是引入所需要的库文件，包括标准库、网络相关库以及多线程库。另外，"create_socket.h", "userlist.h", "broadcast.h", "filelist.h"
是自定义头文件，在具体实现中写出这些头文件中包含的函数和数据结构的定义。

2. `char myname[20];`等代码定义了一些全局变量，这些全局变量在后续的函数中会被使用。

3. 在`main()`函数中，首先，用`setbuf(stdin, NULL);`和`setbuf(stdout, NULL);`禁用了标准输入输出的缓冲，这样可以使得输入输出的实时性更高。

4. 然后，程序通过`fgets()`函数获取用户输入的昵称，然后用`init_userlist();`等函数初始化用户列表、接收和发送文件列表。

5. 接着调用`creat_socket();`创建网络服务，用`broadcast();`广播用户上线信息。

6. 创建3个线程处理UDP消息、扫描键盘输入和发送文件的任务，对应的函数是`udp_msg_process()`, `key_scan_process()`和`tcp_send_file()`。这些函数需要在具体实现中编写。

7. 最后，用`pthread_join()`函数等待3个线程的结束。
*/






