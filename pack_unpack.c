/*代码包含两个函数：transcode和coding。transcode函数将接收到的字符串解析并填充到一个cmd结构体中。
coding函数则执行相反的操作，它将要发送的信息（包括命令、主机名等）编码为字符串格式以便发送。*/


#include "pack_unpack.h"   // 引入自定义头文件（可能包含与打包和解包相关的函数声明）
#include <unistd.h>        // 引入POSIX标准库，例如gethostname函数
#include <stdio.h>         // 引入标准输入输出库
#include <time.h>          // 引入时间处理库
#include "IPMSG.H"         // 引入自定义头文件（可能包含与IP消息相关的宏和声明）
#include "public.h"        // 引入自定义头文件（可能包含公共变量或函数）

// 将接收到的字符串分解为struct cmd结构体
void transcode(struct cmd *object, char *buffer, int len)
{
    // 使用sscanf函数从字符串缓冲区中解析信息并存储到cmd结构体中
    sscanf(buffer, "1:%[^:]:%[^:]:%[^:]:%d:%[^\n]", object->id, object->name, object->hostname, &object->cmdid, object->buf);
}

// 将要发送的信息编码为字符串保存到buf中
int coding(char *buffer, unsigned int cmd, char *append)
{
    char hname[20];       // 定义一个字符数组，用于存储主机名
    time_t h;             // 定义一个time_t类型的变量，用于存储当前时间
    time(&h);             // 获取当前时间并存储到h变量中
    gethostname(hname, sizeof(hname));  // 获取当前主机名并存储到hname数组中
    if (append == NULL) append = "";    // 如果附加字符串为空，将其设置为空字符串

    // 使用sprintf函数将信息格式化为字符串并存储到buffer中
    sprintf(buffer, "1:%ld:%s:%s:%d:%s", h, myname, hname, cmd, append);
}




/*

代码定义了两个函数：`transcode`和`coding`，它们分别用于将字符串解析为一个结构体以及将结构体编码为一个字符串。这种编码和解码的操作在网络编程中
非常常见，因为在网络上传输的数据通常是字节流。

1. `transcode`函数：这个函数接受一个`struct cmd *object`参数，一个`char *buffer`参数和一个`int len`参数。`struct cmd *object`参数
是一个指向`struct cmd`结构体的指针，这个结构体用于保存解析后的数据。`char *buffer`是一个字符串，包含了待解析的原始数据。
函数内部，使用`sscanf`函数从字符串缓冲区中解析信息并存储到`cmd`结构体中。其中，`"1:%[^:]:%[^:]:%[^:]:%d:%[^\n]"`是用于`sscanf`的格式字符串，
它将`buffer`中的字符串按照指定的格式解析并赋值给相应的变量。

2. `coding`函数：这个函数接受一个`char *buffer`参数，一个`unsigned int cmd`参数，和一个`char *append`参数。`char *buffer`用于
保存编码后的字符串，`unsigned int cmd`是命令代码，`char *append`是要附加到编码后字符串的额外信息。在函数内部，首先获取当前主机名和时间，并检查
是否有附加信息，如果没有则将其设置为空字符串。然后，使用`sprintf`函数将这些信息格式化为字符串并存储到`buffer`中。其中，`"1:%ld:%s:%s:%d:%s"`
是用于`sprintf`的格式字符串，它指定了将变量按照何种格式存储到`buffer`中。
*/
