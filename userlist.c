/*代码是管理用户列表的一系列函数。用户列表由一个链表实现，链表的每个节点包含用户的信息（IP地址，用户名，主机名）。
代码包含初始化用户列表、添加用户、删除用户、根据IP地址查找用户和根据用户名查找用户等功能。*/


#include <stdio.h>      // 引入标准输入输出库
#include <pthread.h>    // 引入POSIX线程库
#include <stdlib.h>     // 引入标准库，包含malloc和free等函数
#include <string.h>     // 引入字符串处理函数库
#include <arpa/inet.h>  // 引入用于internet操作的函数库
#include "userlist.h"   // 引入"userlist.h"头文件
#include "public.h"     // 引入"public.h"头文件

// 初始化用户列表
int init_userlist() // 初始化
{
    // 为用户列表分配内存
    userlist_head = (struct user*)malloc(sizeof(struct user));
    // 如果内存分配失败，打印错误信息
    if (userlist_head == NULL)
    {
        perror("申请空间失败！");
        return 1;
    }
    // 将用户列表的内存区域设置为0
    memset(userlist_head, 0 , sizeof(struct user));
    // 将用户列表的next指针设置为NULL，表示列表为空
    userlist_head->next = NULL;
    // 返回0表示成功
    return 0;
}

// 添加用户到用户列表
int addUser(struct in_addr* sin_addr, char name[], char hostname[])
{
    struct user* newUser = NULL;   // 定义一个新用户指针，并初始化为NULL
    struct user* rear = userlist_head; // 定义一个指向用户列表尾部的指针

    // 为新用户分配内存
    if ( (newUser = (struct user*)malloc(sizeof(struct user))) == NULL )
    {
        printf("newUser failed!\n");
        return 1;
    }

    // 设置新用户的IP地址
    newUser->sin_addr.s_addr = sin_addr->s_addr;
    // 复制用户名到新用户的name字段
    strncpy(newUser->name, name, 20);
    // 复制主机名到新用户的host字段
    strncpy(newUser->host, hostname, 20);

    // 寻找用户列表的最后一个元素
    while (rear->next != NULL)
    {
        rear = rear->next;
    }

    // 将新用户添加到用户列表的尾部
    rear->next = newUser;
    // 设置新用户的next指针为NULL，表示这是列表的最后一个元素
    newUser->next = NULL;

    // 返回0表示成功
    return 0;
}

// 根据IP地址删除用户，删除成功返回0，失败返回1
int delUser(struct in_addr sin_addr) // 删除
{
    struct user* pre;  // 定义一个指向要删除用户的前一个用户的指针
    struct user* suc;  // 定义一个指向要删除用户的指针
    pre = userlist_head; // 初始化为用户列表头部
    suc = userlist_head->next; // 初始化为用户列表的第一个元素

    // 遍历用户列表，查找要删除的用户
    while (suc->sin_addr.s_addr != sin_addr.s_addr)
    {
        pre = suc;
        suc = suc->next;
    }
    // 删除找到的用户
    pre->next = suc->next;
    // 释放用户占用的内存
    free(suc);

    // 返回0表示删除成功
    return 0;
}

// 根据IP地址获取用户信息，找到返回用户结构体指针，没有返回NULL
struct user* getUser(struct in_addr* sin_addr) // 根据ip查找用户信息
{
    struct user* cur; // 定义一个指向当前用户的指针
    cur = userlist_head; // 初始化为用户列表头部

    // 遍历用户列表，查找匹配的IP地址
    while ((cur->next != NULL) && (cur->sin_addr.s_addr != sin_addr->s_addr))
    {
        cur = cur->next;
    }

    // 如果找到匹配的IP地址，返回相应的用户结构体指针
    if (cur->sin_addr.s_addr == sin_addr->s_addr)
        return cur;
    else
        return NULL; // 否则返回NULL
}

// 根据用户名获取用户信息，找到返回用户结构体指针，没有返回NULL
struct user* getUser_name(char name[])
{
    struct user* cur; // 定义一个指向当前用户的指针
    cur = userlist_head; // 初始化为用户列表头部

    // 遍历用户列表，查找匹配的用户名
    while ((cur->next != NULL) && (strcmp(name, cur->next->name) != 0))
    {
        cur = cur->next;
    }

    // 如果找到匹配的用户名，返回相应的用户结构体指针
    if (cur->next != NULL && strcmp(name, cur->next->name) == 0)
        return cur->next;
    else
        return NULL; // 否则返回NULL
}

// 根据IP地址获取用户名
char* getName(struct in_addr* sin_addr) {
    // 定义一个指向当前用户的指针，并初始化为用户列表的第一个元素
    struct user* current = userlist_head->next;

    // 遍历用户列表，查找匹配的IP地址
    while (current != NULL) {
        if (current->sin_addr.s_addr == sin_addr->s_addr) {
            // 如果找到匹配的IP地址，返回相应的用户名
            return current->name;
        }
        current = current->next; // 移动到下一个元素
    }
    // 如果没有找到匹配的IP地址，返回NULL
    return NULL;
}



/*

使用链表来管理用户列表。其中定义了一些函数来完成对用户列表的基本操作，包括初始化用户列表、添加用户、删除用户、根据IP地址或用户名查找用户等功能。


1. `init_userlist()`: 这个函数用于初始化用户列表。它首先为用户列表头节点分配内存，如果分配失败，返回错误信息；如果分配成功，则将这块内存区域清零，
并将头节点的next指针设置为NULL，表示列表为空。

2. `addUser(struct in_addr* sin_addr, char name[], char hostname[])`: 这个函数用于向用户列表添加新用户。首先，它为新用户分配内存，
如果分配失败，返回错误信息；如果分配成功，就将输入参数的值（IP地址、用户名和主机名）复制到新用户的相应字段，然后遍历链表找到最后一个元素，并将新用户添加到链表的尾部。

3. `delUser(struct in_addr sin_addr)`: 这个函数用于从用户列表中删除指定IP地址的用户。它会遍历用户列表，找到匹配的IP地址，然后更改其前一个节点的next指针，
使其跳过当前节点，连接到下一个节点，最后释放当前节点的内存。

4. `getUser(struct in_addr* sin_addr)`: 这个函数用于根据IP地址查找用户。它遍历用户列表，如果找到匹配的IP地址，则返回这个用户的结构体指针；如果没有找到匹配的IP地址，则返回NULL。

5. `getUser_name(char name[])`: 这个函数用于根据用户名查找用户。它遍历用户列表，如果找到匹配的用户名，则返回这个用户的结构体指针；如果没有找到匹配的用户名，则返回NULL。

6. `getName(struct in_addr* sin_addr)`: 这个函数用于根据IP地址查找用户名。它遍历用户列表，如果找到匹配的IP地址，则返回这个用户的用户名；如果没有找到匹配的IP地址，则返回NULL。

*/
