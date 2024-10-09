# 目录
- [目录](#目录)
  - [socket编程](#socket编程)
    - [socket是什么](#socket是什么)
    - [IP和端口](#ip和端口)
    - [字节序](#字节序)
    - [通用套接字地址结构](#通用套接字地址结构)
  - [TCP](#tcp)
  - [UDP](#udp)

## socket编程
### socket是什么
    应用层与运输层之间的产物
**作用：将运输层的很多复杂操作封装成一些简单的接口，来让应用层调用，以此来实现进程在网络中的通信**

    原理：一种特殊的文件API函数，返回值是一个特殊的文件描述符。用于打开网络接口的文件,我们的一般网络通信的时候,使用的是两台电脑上的不同进程,故socket在网卡的相关位置创建一个虚拟文件系统,用与接收和发送消息。

**本质一：一个非负整数（一种特殊的文件描述符）  
本质二：一个打开的文件（和操作其他文件一样）**

创建套接字函数原型  

    int socket(int domain, int type, int protocol);
```sh
参数信息
domain: 指定套接字的地址族
        AF_UNIX/AF_LOCAL —— 用于本地通讯
        AF_INET —— IPv4地址族，用于使用IPv4地址通讯
        AF_INET6 —— IPv6地址族，用于使用IPv6地址进行通讯
type: 指定套接字的类型
        SOCK_STREAM —— 流套接字（TCP通信）
        SOCK_DGRAM —— 数据报套接字（UDP通信）
protocol: 指定使用的协议
        通常使用0表示默认方式
        IPPROTO_TCP
        IPPROTO_UDP
        IPPROTO_SCTP
        只有当第一个参数为AF_INET或AF_INET6时且使用SCTP协议时才需要配置，其他情况下配0即可
```

### IP和端口

**IP是什么**  
一种网络协议，用于在Internet上唯一表示和定位设备  
IPv4由32位二进制数字组成
IPv6由128位二进制数字组成

**IP地址作用**  
将数据包从源设备发送到目标设备，类似于邮件中的寄件人和收件人地址

**端口是什么**  
用于标识特定应用程序或服务的数字

在网络设备中，每个设备都有65535个端口号


### 字节序

**什么是字节序**  
程序在内存中存储的方式

```sh
大端字节序（网络字节序）
    低地址  存  高数据位
    高地址  存  低数据位

    使用场景：ARM 和 网络通讯数据

小端字节序（本机字节序）
    低地址  存  低数据位
    高地址  存  高数据位

    使用场景：inter芯片 和 x86 
```

**IP地址转换成网络字节序**
十进制  —>  IP网络字节序（二进制数据）
```sh
in_addr_t inet_addr(const char *cp);
```
例：192.168.100.1 -> 0x164a8c0(成功返回的是32位无符号整数，用16进制打印出来表示)  
c0 ---> 192  
a8 ---> 168  
64 ---> 100  

十进制字符串形成的IPv4地址转换为网络字节
```sh
int inet_aton(const char *cp, struct in_addr *inp);
```
struct in_addr结构体指针，用于保存转换后的IPv4地址  

```sh
int inet_pton(int af, const char 'src, void "dst);
```
af —— 地址族，用于指定IPv4或IPv6
src —— 点分十进制形式的IPv4或IPv6地址的C字符串
dst —— 指向存储转换后地址的内存空间

**网络字节序转换成IP字符串**  
IP网络字节序（二进制数据）转换为点分十进制
```sh
char *inet_ntoa(struct in_addr in);
```
网络字节序转换成字节序
```sh
const char "inet_ntop(int af, const void *src, char *dst, socklen_t size);
```
同上inet_pton
size —— 缓存区dst大小

字符串转int
```sh
int atoi(const char *nptr);
```

本机字节序转换成网络字节序
```sh
uint16_t htons(uint16_t hostshort);
```

网络字节序转本机字节序
```sh
uint16_t ntohs(uint16_t netshort);
```

### 通用套接字地址结构
struct sockaddr

struct sockaddr_in

struct sockaddr_storage

## TCP


## UDP