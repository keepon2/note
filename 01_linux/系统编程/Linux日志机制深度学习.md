# Linux日志机制深度学习

## 目录
1. [概述](#概述)
2. [内核空间日志机制](#内核空间日志机制)
   - [printk函数详解](#printk函数详解)
   - [printk的变体](#printk的变体)
   - [内核日志缓冲区](#内核日志缓冲区)
3. [用户空间日志机制](#用户空间日志机制)
   - [printf函数家族](#printf函数家族)
   - [syslog机制](#syslog机制)
   - [用户空间日志库](#用户空间日志库)
4. [日志存储机制](#日志存储机制)
   - [内核日志存储](#内核日志存储)
   - [用户空间日志存储](#用户空间日志存储)
5. [日志级别与管理](#日志级别与管理)
6. [性能优化与最佳实践](#性能优化与最佳实践)

## 概述

Linux系统提供了完整的日志记录机制，分为内核空间和用户空间两个层面。这些机制不仅帮助开发者调试程序，还为系统管理员监控系统运行状态提供了重要手段。

## 内核空间日志机制

### printk函数详解

`printk`是内核空间最基础的日志输出函数，类似于用户空间的`printf`，但具有特殊的日志级别功能。

#### 基本语法
```c
printk(const char *fmt, ...);
```

#### 日志级别
内核定义了8个日志级别，定义在`<linux/kernel.h>`中：

```c
#define KERN_EMERG   "<0>"  /* 系统不可用 */
#define KERN_ALERT   "<1>"  /* 必须立即采取行动 */
#define KERN_CRIT    "<2>"  /* 严重情况 */
#define KERN_ERR     "<3>"  /* 错误情况 */
#define KERN_WARNING "<4>"  /* 警告情况 */
#define KERN_NOTICE  "<5>"  /* 正常但重要的信息 */
#define KERN_INFO    "<6>"  /* 信息性消息 */
#define KERN_DEBUG   "<7>"  /* 调试级别消息 */
```

#### 使用示例
```c
printk(KERN_INFO "驱动程序初始化成功\n");
printk(KERN_ERR "设备打开失败: %d\n", error_code);
printk(KERN_DEBUG "调试信息: 变量值 = %d\n", value);
```

### printk的变体

#### pr_*系列函数
Linux内核提供了更便捷的pr_*系列函数，自动添加日志级别前缀：

```c
pr_emerg("紧急情况: %s\n", msg);
pr_alert("警报: %s\n", msg);
pr_crit("严重错误: %s\n", msg);
pr_err("错误: %s\n", msg);
pr_warn("警告: %s\n", msg);
pr_notice("通知: %s\n", msg);
pr_info("信息: %s\n", msg);
pr_debug("调试: %s\n", msg); // 仅在DEBUG模式下编译
```

#### dev_*系列函数
设备驱动程序专用的日志函数，包含设备信息：

```c
struct device *dev;
dev_err(dev, "设备错误: %d\n", error);
dev_warn(dev, "设备警告: %s\n", warning_msg);
dev_info(dev, "设备信息: %s\n", info_msg);
```

#### netdev_*系列函数
网络设备专用日志函数：

```c
struct net_device *netdev;
netdev_err(netdev, "网络设备错误: %d\n", error);
netdev_info(netdev, "网络设备信息: %s\n", info);
```

### 内核日志缓冲区

#### 环形缓冲区机制
内核使用环形缓冲区存储日志消息，特点包括：
- 固定大小（通常为几MB）
- 新消息覆盖旧消息
- 可通过`/proc/kmsg`或dmesg命令访问

#### 缓冲区操作
```c
// 查看内核日志缓冲区大小
cat /proc/sys/kernel/printk

// 修改缓冲区大小（需要root权限）
echo 8192 > /proc/sys/kernel/printk
```

#### 内核参数控制
```
/proc/sys/kernel/printk格式：
[控制台日志级别] [默认消息日志级别] [最小控制台日志级别] [默认控制台日志级别]
```

## 用户空间日志机制

### printf函数家族

#### 标准printf函数
```c
#include <stdio.h>
printf("格式化字符串: %d %s\n", number, string);
```

#### printf变体
```c
// 输出到文件
fprintf(FILE *stream, "格式化字符串", ...);

// 输出到字符串
sprintf(char *str, "格式化字符串", ...);
snprintf(char *str, size_t size, "格式化字符串", ...);

// 输出到stderr
fprintf(stderr, "错误信息: %s\n", error_msg);
```

### syslog机制

#### syslog函数族
```c
#include <syslog.h>

// 打开日志连接
openlog("程序名称", LOG_PID | LOG_CONS, LOG_USER);

// 写入日志
syslog(LOG_INFO, "信息消息: %s", message);
syslog(LOG_ERR, "错误消息: %s", error_msg);

// 关闭日志连接
closelog();
```

#### 日志级别定义
```c
#define LOG_EMERG   0   /* 系统不可用 */
#define LOG_ALERT   1   /* 必须立即采取行动 */
#define LOG_CRIT    2   /* 严重情况 */
#define LOG_ERR     3   /* 错误情况 */
#define LOG_WARNING 4   /* 警告情况 */
#define LOG_NOTICE  5   /* 正常但重要的信息 */
#define LOG_INFO    6   /* 信息性消息 */
#define LOG_DEBUG   7   /* 调试级别消息 */
```

#### 日志设施(facility)
```c
#define LOG_KERN     (0<<3)   /* 内核消息 */
#define LOG_USER     (1<<3)   /* 用户级消息 */
#define LOG_MAIL     (2<<3)   /* 邮件系统 */
#define LOG_DAEMON   (3<<3)   /* 系统守护进程 */
#define LOG_AUTH     (4<<3)   /* 安全/认证消息 */
#define LOG_SYSLOG   (5<<3)   /* syslogd内部消息 */
#define LOG_LPR      (6<<3)   /* 行打印机子系统 */
#define LOG_NEWS     (7<<3)   /* 网络新闻子系统 */
#define LOG_UUCP     (8<<3)   /* UUCP子系统 */
#define LOG_CRON     (9<<3)   /* 时钟守护进程 */
#define LOG_AUTHPRIV (10<<3)  /* 私有安全/认证消息 */
#define LOG_FTP      (11<<3)  /* FTP守护进程 */
```

### 用户空间日志库

#### syslog配置文件
```bash
# /etc/syslog.conf 或 /etc/rsyslog.conf
auth,authpriv.*                 /var/log/auth.log
kern.*                          /var/log/kern.log
mail.*                          /var/log/mail.log
daemon.*                        /var/log/daemon.log
*.info;*.!err;*.!warning        /var/log/messages
```

#### 使用示例
```c
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

int main() {
    // 打开日志
    openlog("myapp", LOG_PID | LOG_CONS, LOG_USER);
    
    // 记录不同级别的日志
    syslog(LOG_INFO, "程序启动，PID: %d", getpid());
    syslog(LOG_DEBUG, "调试信息: 变量值 = %d", 42);
    syslog(LOG_WARNING, "警告: 配置文件未找到，使用默认配置");
    
    // 模拟错误情况
    if (error_condition) {
        syslog(LOG_ERR, "严重错误: 无法连接数据库");
    }
    
    // 关闭日志
    closelog();
    return 0;
}
```

#### 现代日志库
```c
// 使用zlog库示例
#include "zlog.h"

int main() {
    dzlog_init("zlog.conf", "my_cat");
    
    dzlog_info("程序启动");
    dzlog_debug("调试信息");
    dzlog_error("错误信息");
    
    zlog_fini();
    return 0;
}
```

## 日志存储机制

### 内核日志存储

#### 内核环形缓冲区
- 位置：内核内存空间
- 大小：可配置，通常为几MB
- 访问方式：
  - `/proc/kmsg` - 阻塞读取
  - `/dev/kmsg` - 非阻塞读取
  - `dmesg` 命令

#### 内核日志持久化
```bash
# 查看内核日志
dmesg

# 清空内核缓冲区
dmesg -c

# 实时查看内核日志
dmesg -w

# 带时间戳显示
dmesg -T

# 按级别过滤
dmesg -l err,warn
```

#### 内核日志配置
```bash
# /etc/sysctl.conf
kernel.printk = 4 4 1 7
kernel.dmesg_restrict = 1  # 限制普通用户访问dmesg
```

### 用户空间日志存储

#### 传统syslog存储
```bash
# 常见日志文件位置
/var/log/syslog        # 系统日志
/var/log/auth.log      # 认证日志
/var/log/kern.log      # 内核日志
/var/log/mail.log      # 邮件日志
/var/log/daemon.log    # 守护进程日志
```

#### 日志轮转配置
```bash
# /etc/logrotate.conf
weekly
rotate 4
compress
delaycompress
missingok
notifempty
create 644 root adm

# /etc/logrotate.d/rsyslog
/var/log/syslog {
    rotate 7
    daily
    missingok
    notifempty
    delaycompress
    compress
    postrotate
        /usr/lib/rsyslog/rsyslog-rotate
    endscript
}
```

#### 现代日志系统
```bash
# systemd journalctl
journalctl -f                    # 实时查看
journalctl -u nginx.service      # 查看特定服务
journalctl --since "1 hour ago"  # 时间过滤
journalctl -p err                # 级别过滤
```

## 日志级别与管理

### 动态控制内核日志级别
```c
// 在内核代码中动态控制
int console_loglevel = console_printk[0];
int default_message_loglevel = console_printk[1];
int minimum_console_loglevel = console_printk[2];
int default_console_loglevel = console_printk[3];
```

### 运行时控制
```bash
# 查看当前日志级别
cat /proc/sys/kernel/printk

# 临时修改日志级别
echo 8 > /proc/sys/kernel/printk

# 永久修改
echo "kernel.printk = 8 8 1 8" >> /etc/sysctl.conf
```

### 日志过滤与匹配
```bash
# 使用grep过滤日志
dmesg | grep -i error
journalctl | grep "network"

# 使用正则表达式
journalctl -G "error|warning|fail"

# 按进程过滤
journalctl _PID=1234
```

## 性能优化与最佳实践

### 内核日志优化

#### 减少日志开销
```c
// 使用条件编译避免不必要的日志处理
#ifdef DEBUG
    pr_debug("调试信息: %d\n", value);
#endif

// 使用unlikely优化错误路径
if (unlikely(error)) {
    pr_err("发生错误: %d\n", error);
    return error;
}
```

#### 异步日志记录
```c
// 使用工作队列延迟日志处理
static void log_work_handler(struct work_struct *work) {
    printk(KERN_INFO "异步日志消息\n");
}

static DECLARE_WORK(log_work, log_work_handler);

// 在关键路径中调度日志工作
schedule_work(&log_work);
```

### 用户空间日志优化

#### 缓冲日志写入
```c
// 使用缓冲I/O减少系统调用
setvbuf(log_file, NULL, _IOFBF, 8192);

// 批量写入日志
void batch_write_logs(const char **logs, int count) {
    for (int i = 0; i < count; i++) {
        fprintf(log_file, "%s\n", logs[i]);
    }
    fflush(log_file);  // 批量刷新
}
```

#### 日志级别控制
```c
// 运行时可配置的日志级别
static int current_log_level = LOG_INFO;

void log_message(int level, const char *format, ...) {
    if (level > current_log_level) {
        return;  // 跳过低优先级日志
    }
    
    va_list args;
    va_start(args, format);
    vsyslog(level, format, args);
    va_end(args);
}
```

### 安全考虑

#### 敏感信息处理
```c
// 避免记录敏感信息
void log_user_action(const char *action, const struct user *user) {
    // 不要记录密码等敏感信息
    syslog(LOG_INFO, "用户 %s 执行操作: %s", 
           user->username, action);
    // syslog(LOG_INFO, "用户 %s 密码: %s", user->username, user->password); // 错误!
}
```

#### 日志权限控制
```bash
# 设置适当的文件权限
chmod 640 /var/log/app.log
chown root:adm /var/log/app.log

# 使用logrotate管理权限
create 640 root adm
```

## 总结

Linux的日志机制是一个复杂而强大的系统，涵盖了从内核到用户空间的各个层面。理解这些机制对于系统调试、性能分析和安全监控都至关重要。

### 关键要点
1. **内核空间**：使用printk及其变体，通过环形缓冲区管理
2. **用户空间**：使用printf、syslog等机制，支持多种存储方式
3. **日志级别**：合理使用日志级别，避免信息过载
4. **性能优化**：注意日志记录的性能影响，特别是在关键路径中
5. **安全考虑**：避免记录敏感信息，合理设置访问权限

通过深入理解这些机制，开发者可以构建更加健壮和可维护的Linux应用程序。