
/*涉及文件的发送和接收。它主要定义了两个链表send_filelist和rcv_filelist，分别用于存储待发送和待接收的文件。它包含初始化链表、添加文件到链表、
根据IP地址或文件名从链表删除文件、根据IP地址或文件名获取文件等功能。*/


#include <stdio.h>            // 包含标准输入输出库
#include <pthread.h>          // 包含线程相关库
#include <stdlib.h>           // 包含标准库，例如malloc和free
#include <string.h>           // 包含字符串操作相关函数
#include <arpa/inet.h>        // 包含网络编程库
#include <sys/stat.h>         // 包含文件状态查询的库
#include <unistd.h>           // 包含Unix标准库
#include <time.h>             // 包含时间相关库
#include "filelist.h"         // 包含自定义的文件列表头文件
#include "public.h"           // 包含自定义的公共头文件

// 初始化发送文件链表
int init_send_filelist()
{
    send_filelist_head = (struct sendfile*)malloc(sizeof(struct sendfile)); // 动态分配内存空间给链表头节点
    if (send_filelist_head == NULL) // 判断内存是否分配成功
    {
        perror("申请空间失败！"); // 如果分配失败，输出错误信息
        return 1;                // 返回1表示失败
    }
    memset(send_filelist_head, 0 , sizeof(struct sendfile)); // 将链表头节点的内存空间初始化为0
    send_filelist_head->next = NULL;                         // 设置头节点的next指针为NULL
    return 0;                                                // 返回0表示成功
}

// 添加文件到发送文件链表
int add_sendFile(struct in_addr* sin_addr, char name[])
{
    struct stat buf;                     // 定义一个文件状态结构体变量
    struct sendfile* newFile = NULL;     // 定义一个新文件节点指针并初始化为NULL
    struct sendfile* rear = send_filelist_head; // 定义一个指针指向链表的尾部，初始为头节点
    static unsigned int file_num = 0;    // 定义一个静态变量，用于文件编号

    stat(name, &buf);                    // 获取文件状态信息并存储到buf变量中

    if ( (newFile = (struct sendfile*)malloc(sizeof(struct sendfile))) == NULL ) // 为新节点分配内存
    {
        printf("newFile failed!\n");    // 如果分配失败，输出错误信息
        return 1;                       // 返回1表示失败
    }

    newFile->sin_addr.s_addr = sin_addr->s_addr; // 设置新节点的IP地址
    strncpy(newFile->name, name, 20);   // 设置新节点的文件名
    newFile->num = file_num;            // 设置新节点的文件编号
    newFile->pkgnum = time(NULL);       // 设置新节点的包数量为当前时间
    newFile->size = buf.st_size;        // 设置新节点的文件大小
    newFile->ltime = buf.st_mtime;      // 设置新节点的最后修改时间
    while (rear->next != NULL)          // 找到链表的尾部
    {
        rear = rear->next;              // 向后移动
    }

    rear->next = newFile;               // 将尾节点的next指针指向新的文件节点
    newFile->next = NULL;               // 设置新节点的next指针为NULL

    return 0;                           // 返回0表示成功
}

// 根据IP地址删除发送文件链表中的节点，成功返回0，失败返回1
int del_sendFile(struct in_addr sin_addr)
{
    struct sendfile* pre, *suc;  // 定义两个指针，pre指向当前节点的前一个节点，suc指向当前节点
    pre = send_filelist_head;    // 初始化pre为链表头节点
    suc = send_filelist_head->next;  // 初始化suc为头节点的下一个节点
    while (suc->sin_addr.s_addr != sin_addr.s_addr) // 遍历链表，寻找匹配的IP地址
    {
        pre = suc;                // 向后移动
        suc = suc->next;          // 向后移动
    }
    pre->next = suc->next;        // 删除当前节点
    free(suc);                    // 释放当前节点的内存空间

    return 0;                     // 返回0表示删除成功
}

// 根据文件名删除发送文件链表中的节点，成功返回0，失败返回1
int del_sendFile_name(char nam[])
{
    struct sendfile* pre, *suc;  // 定义两个指针，pre指向当前节点的前一个节点，suc指向当前节点
    pre = send_filelist_head;    // 初始化pre为链表头节点
    suc = send_filelist_head->next;  // 初始化suc为头节点的下一个节点
    while (strcmp(suc->name, nam) != 0) // 遍历链表，寻找匹配的文件名
    {
        pre = suc;                // 向后移动
        suc = suc->next;          // 向后移动
    }
    pre->next = suc->next;        // 删除当前节点
    free(suc);                    // 释放当前节点的内存空间

    return 0;                     // 返回0表示删除成功
}

// 根据IP地址获取发送文件链表中的节点
struct sendfile* get_sendFile(struct in_addr *sin_addr)
{
    struct sendfile* cur = send_filelist_head; // 定义一个指针指向链表头节点
    while ((cur->next != NULL) && (cur->sin_addr.s_addr != sin_addr->s_addr)) // 遍历链表，寻找匹配的IP地址
    {
        cur = cur->next;          // 向后移动
    }
    if (cur->sin_addr.s_addr == sin_addr->s_addr) // 如果找到匹配的IP地址
        return cur;               // 返回当前节点
    else
        return NULL;              // 否则返回NULL
}

// 根据文件名获取file结构体
struct sendfile* get_sendFile_name(char name[])
{
    struct sendfile* cur;                  // 定义一个当前节点指针
    cur = send_filelist_head;              // 将当前节点指针初始化为链表头
    while ((cur->next != NULL) && strcmp(name, cur->next->name) != 0) // 遍历链表，直到找到文件名匹配或到达链表尾部
    {
        cur = cur->next;                  // 向后移动
    }
    if (cur->next != NULL && strcmp(name, cur->next->name) == 0) // 如果找到匹配的文件名
        return cur->next;                 // 返回匹配的节点
    else
        return NULL;                      // 否则返回NULL
}

// 根据文件编号获取file结构体
struct sendfile* get_sendFile_num(int num)
{
    struct sendfile* cur;                 // 定义一个当前节点指针
    cur = send_filelist_head;             // 将当前节点指针初始化为链表头
    while ((cur->next != NULL) && (num != cur->next->num)) // 遍历链表，直到找到文件编号匹配或到达链表尾部
    {
        cur = cur->next;                  // 向后移动
    }
    if (cur->next != NULL && (num == cur->next->num)) // 如果找到匹配的文件编号
        return cur->next;                 // 返回匹配的节点
    else
        return NULL;                      // 否则返回NULL
}

// 根据IP地址获取文件名
char* get_sendFileName(struct in_addr* sin_addr)
{
    int none = 0;                        // 定义一个标记变量，表示是否找到匹配的IP地址

    struct sendfile * cur;               // 定义一个当前节点指针
    cur = send_filelist_head->next;      // 将当前节点指针初始化为链表头的下一个节点
    while (cur->sin_addr.s_addr != sin_addr->s_addr) // 遍历链表，直到找到IP地址匹配或到达链表尾部
    {
        cur = cur->next;                 // 向后移动
        if (cur->sin_addr.s_addr == sin_addr->s_addr) // 如果找到匹配的IP地址
            break;                       // 结束循环

        if (cur->next == NULL && cur->sin_addr.s_addr != sin_addr->s_addr) // 如果到达链表尾部且没有找到匹配的IP地址
        {
            none = 1;                    // 设置标记变量为1
            break;                       // 结束循环
        }
        else
        {
            return NULL;                 // 这个分支似乎是错误的，应该移除
        }
    }
    // 这个函数没有返回值，应该添加逻辑返回文件名或NULL
}

// 初始化接收文件链表
int init_rcv_filelist(void)
{
    rcv_filelist_head = (struct rcvfile*)malloc(sizeof(struct rcvfile)); // 动态分配一个节点作为接收文件链表的头节点
    if (rcv_filelist_head == NULL)         // 如果分配失败
    {
        perror("申请空间失败！");           // 输出错误信息
        return 1;                          // 返回1表示失败
    }
    memset(rcv_filelist_head, 0, sizeof(struct rcvfile)); // 将新节点的内存空间初始化为0
    rcv_filelist_head->next = NULL;        // 设置新节点的next指针为NULL
    return 0;                              // 返回0表示成功
}

// 向接收文件链表中添加文件
int add_rcvFile(struct in_addr* sin_addr, char name[], long num, long size)
{
    struct rcvfile* newFile = NULL;        // 定义一个新节点指针
    struct rcvfile* rear = rcv_filelist_head; // 定义一个指针，初始化为接收文件链表的头节点
    if ((newFile = (struct rcvfile*)malloc(sizeof(struct rcvfile))) == NULL) // 动态分配一个新节点
    {
        printf("newFile failed!\n");      // 如果分配失败，输出错误信息
        return 1;                         // 返回1表示失败
    }

    newFile->sin_addr.s_addr = sin_addr->s_addr; // 设置新节点的IP地址
    strncpy(newFile->name, name, 20);     // 设置新节点的文件名
    newFile->num = num;                   // 设置新节点的文件编号
    newFile->pkgnum = time(NULL);         // 设置新节点的包数量为当前时间戳
    newFile->size = size;                 // 设置新节点的文件大小
    while (rear->next != NULL)            // 遍历链表，找到尾部节点
    {
        rear = rear->next;                // 向后移动
    }

    rear->next = newFile;                 // 将尾部节点的next指针指向新节点
    newFile->next = NULL;                 // 设置新节点的next指针为NULL

    return 0;                             // 返回0表示成功
}

// 根据sin_addr删除文件，正确删除返回0，错误返回1
int del_rcvFile(struct in_addr sin_addr) // 删除
{
    struct rcvfile* pre, *suc;             // 定义两个指针，pre指向当前节点的前一个节点，suc指向当前节点
    pre = rcv_filelist_head;               // 初始化pre为链表头
    suc = rcv_filelist_head->next;         // 初始化suc为链表头的下一个节点
    while (suc->sin_addr.s_addr != sin_addr.s_addr) // 遍历链表，直到找到匹配的IP地址或到达链表尾部
    {
        pre = suc;                        // 更新pre为suc
        suc = suc->next;                  // 更新suc为suc的下一个节点
    }
    pre->next = suc->next;                // 使pre的next指向suc的next，从链表中移除suc
    free(suc);                            // 释放suc所占的内存
    // printf("删除成功！\n");                // 输出删除成功的信息，此行已被注释掉

    return 0;                             // 返回0表示删除成功
}

// 根据文件名删除文件，正确删除返回0，错误返回1
int del_rcvFile_name(char nam[]) // 删除
{
    struct rcvfile* pre, *suc;            // 定义两个指针，pre指向当前节点的前一个节点，suc指向当前节点
    pre = rcv_filelist_head;              // 初始化pre为链表头
    suc = rcv_filelist_head->next;        // 初始化suc为链表头的下一个节点
    while (strcmp(suc->name, nam) != 0)   // 遍历链表，直到找到匹配的文件名或到达链表尾部
    {
        pre = suc;                       // 更新pre为suc
        suc = suc->next;                 // 更新suc为suc的下一个节点
    }
    pre->next = suc->next;               // 使pre的next指向suc的next，从链表中移除suc
    free(suc);                           // 释放suc所占的内存
    // printf("删除成功！\n");               // 输出删除成功的信息

    return 0;                            // 返回0表示删除成功
}

// 根据sin_addr获取file结构体，没有返回NULL
struct rcvfile* get_rcvFile(struct in_addr *sin_addr) // 根据ip查找文件信息
{
    struct rcvfile* cur = rcv_filelist_head; // 定义一个当前节点指针，初始化为链表头
    while ((cur->next != NULL) && (cur->sin_addr.s_addr != sin_addr->s_addr)) // 遍历链表，直到找到匹配的IP地址或到达链表尾部
    {
        cur = cur->next;                // 向后移动
    }
    if (cur->sin_addr.s_addr == sin_addr->s_addr) // 如果找到匹配的IP地址
        return cur;                     // 返回匹配的节点
    else
        return NULL;                    // 否则返回NULL
}

// 根据文件名获取file结构体
struct rcvfile* get_rcvFile_name(char nam[])
{
    struct rcvfile* cur;                // 定义一个当前节点指针
    cur = rcv_filelist_head;            // 初始化为链表头
    while ((cur->next != NULL) && strcmp(nam, cur->next->name) != 0) // 遍历链表，直到找到匹配的文件名或到达链表尾部
    {
        cur = cur->next;               // 向后移动
    }
    if (cur->next != NULL && strcmp(nam, cur->next->name) == 0) // 如果找到匹配的文件名
        return cur->next;              // 返回匹配的节点
    else
        return NULL;                   // 否则返回NULL
}

// 根据ip得到filename
char* get_rcvFileName(struct in_addr* sin_addr) // 根据ip查找filename
{
    int none = 0;                      // 定义一个标志变量，初始化为0
    struct rcvfile * cur;              // 定义一个当前节点指针
    cur = rcv_filelist_head->next;     // 初始化为链表头的下一个节点
    while (cur->sin_addr.s_addr != sin_addr->s_addr) // 遍历链表，直到找到匹配的IP地址或到达链表尾部
    {
        cur = cur->next;              // 向后移动
        if (cur->sin_addr.s_addr == sin_addr->s_addr)
            break;                    // 如果找到匹配的IP地址，退出循环

        if (cur->next == NULL && cur->sin_addr.s_addr != sin_addr->s_addr) // 如果到达链表尾部且没有找到匹配的IP地址
        {
            none = 1;                 // 设置标志变量为1
            break;                    // 退出循环
        }
    }
    // 此函数没有返回值，逻辑不完整
}





/*
这段代码主要用于管理发送和接收文件的链表。代码中定义了两种链表结构：`sendfile` 用于管理待发送的文件，`rcvfile` 用于管理接收的文件。
每个结构都包含文件信息和指向下一个文件的指针。

1. `init_send_filelist()` 初始化发送文件链表：
- 动态分配内存来创建一个`sendfile`链表的头节点，并把头节点的指针赋给全局变量`send_filelist_head`。
- 如果内存分配失败，则打印错误信息并返回1。
- 使用`memset`将链表头的内存区域初始化为0。
- 将头节点的`next`指针设置为NULL，表示链表为空。
- 返回0，表示成功。

2. `add_sendFile(struct in_addr* sin_addr, char name[])` 向发送文件链表中添加一个文件：
- 使用`stat`函数获取文件的信息，并存储在`buf`结构体中。
- 为新文件动态分配一个`sendfile`结构的内存空间，并将指针存储在`newFile`。
- 将输入参数`sin_addr`和`name`以及文件信息存储在`newFile`中。
- 遍历链表，找到链表尾部，并将`newFile`添加到链表的尾部。
- 将`newFile`的`next`指针设置为NULL。

3. `del_sendFile(struct in_addr sin_addr)` 根据IP地址从发送文件链表中删除一个文件：
- 遍历链表，找到与输入的IP地址匹配的节点。
- 更新链表以跳过找到的节点。
- 释放找到的节点的内存。

4. `get_sendFile(struct in_addr *sin_addr)` 根据IP地址在发送文件链表中查找文件信息：
- 遍历链表，找到与输入的IP地址匹配的节点。
- 如果找到，返回该节点的指针；否则返回NULL。

5. `init_rcv_filelist()` 类似于`init_send_filelist()`，但用于初始化接收文件链表。

6. `add_rcvFile(struct in_addr* sin_addr, char name[], long num, long size)` 类似于`add_sendFile()`，但用于向接收文件链表添加一个文件。

7. `del_rcvFile(struct in_addr sin_addr)` 类似于`del_sendFile()`，但用于从接收文件链表中删除一个文件。

8. `get_rcvFile(struct in_addr *sin_addr)` 类似于`get_sendFile()`，但用于在接收文件链表中查找文件信息。

此外，还有一些辅助函数，如根据文件名删除和获取文件，以及根据IP地址获取文件名等。
*/


