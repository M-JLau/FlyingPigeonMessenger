
#ifndef _PUBLIC_H_                 // 如果_PUBLIC_H_没有被定义，则执行下面的代码。这是一个头文件保护，防止头文件被重复包含
#define _PUBLIC_H_                 // 定义_PUBLIC_H_标识符，表明这个头文件已经被包含了

#define BUFF_SIZE  128             // 定义一个宏BUFF_SIZE，表示缓冲区的大小为128字节
#define MSG_PORT   2425            // 定义一个宏MSG_PORT，表示消息使用的端口号为2425
#define BUFFER_SIZE 1024           // 定义一个宏BUFFER_SIZE，表示缓冲区的大小为1024字节

extern char myname[20];            // 声明一个外部的全局变量myname，它是一个字符数组，用于存储名称，大小为20字节

extern int udp_sock;               // 声明一个外部的全局变量udp_sock，它是一个整数，用于存储UDP套接字的描述符
extern int tcp_sock;               // 声明一个外部的全局变量tcp_sock，它是一个整数，用于存储TCP套接字的描述符
extern struct sockaddr_in udp_sock_addr;  // 声明一个外部的全局变量udp_sock_addr，它是sockaddr_in结构体，用于存储UDP套接字的地址信息
extern struct sockaddr_in tcp_sock_addr;  // 声明一个外部的全局变量tcp_sock_addr，它是sockaddr_in结构体，用于存储TCP套接字的地址信息

extern struct rcvfile *rcv_filelist_head; // 声明一个外部的全局变量rcv_filelist_head，它是一个rcvfile结构体的指针，用于存储接收文件列表的头指针
extern struct sendfile *send_filelist_head; // 声明一个外部的全局变量send_filelist_head，它是一个sendfile结构体的指针，用于存储发送文件列表的头指针
extern struct user *userlist_head; // 声明一个外部的全局变量userlist_head，它是一个user结构体的指针，用于存储用户列表的头指针

#endif                             // 结束头文件保护的代码块
