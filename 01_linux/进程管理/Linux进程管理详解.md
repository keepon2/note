# Linux 进程管理详解

> 参考书目：《Linux内核设计与实现》、《图解Linux内核 基于6.x》

---

## 一、进程概述

### 1.1 什么是进程

进程是程序执行的一个实例，是系统进行资源分配和调度的基本单位。从内核的角度看，进程由以下部分组成：

- **代码段**：存放程序代码
- **数据段**：存放程序数据
- **堆栈段**：存放局部变量和函数调用信息
- **进程控制块（PCB）**：存放进程的描述信息

### 1.2 进程与线程的区别

| 特性 | 进程 | 线程 |
|------|------|------|
| 资源拥有 | 独立拥有 | 共享进程资源 |
| 地址空间 | 独立 | 共享 |
| 通信方式 | IPC（管道、消息队列等） | 共享内存、全局变量 |
| 创建开销 | 大 | 小 |
| 切换开销 | 大 | 小 |

### 1.3 进程层次结构

```mermaid
graph TD
    A[init进程 PID=1] --> B[系统守护进程]
    A --> C[用户登录进程]
    C --> D[Shell进程]
    D --> E[用户进程1]
    D --> F[用户进程2]
    F --> G[子进程1]
    F --> H[子进程2]
    
    style A fill:#ff6b6b
    style C fill:#4ecdc4
    style D fill:#45b7d1
```

---

## 二、进程描述符（task_struct）

### 2.1 task_struct 结构概述

`task_struct` 是 Linux 内核中最重要的数据结构之一，包含了进程的所有信息。在 Linux 6.x 内核中，它定义在 `include/linux/sched.h` 中。

```mermaid
graph TB
    A[task_struct 进程描述符] --> B[进程标识信息]
    A --> C[进程状态信息]
    A --> D[进程调度信息]
    A --> E[进程亲属关系]
    A --> F[内存管理信息]
    A --> G[文件系统信息]
    A --> H[信号处理信息]
    A --> I[时间信息]
    A --> J[资源限制信息]
    
    B --> B1[PID/TGID]
    B --> B2[用户/组ID]
    B --> B3[会话ID]
    
    C --> C1[state]
    C --> C2[exit_state]
    C --> C3[flags]
    
    D --> D1[prio]
    D --> D2[static_prio]
    D --> D3[normal_prio]
    D --> D4[rt_priority]
    
    E --> E1[parent]
    E --> E2[real_parent]
    E --> E3[children]
    E --> E4[sibling]
    
    F --> F1[mm]
    F --> F2[active_mm]
    F --> F3[stack]
    
    G --> G1[fs]
    G --> G2[files]
    G --> G3[nsproxy]
    
    style A fill:#2ecc71
    style B fill:#3498db
    style C fill:#e74c3c
    style D fill:#f39c12
```

### 2.2 关键字段详解

#### 2.2.1 进程标识相关

```c
struct task_struct {
    volatile long state;          /* 进程状态 */
    int exit_state;               /* 退出状态 */
    unsigned int flags;           /* 进程标志 */
    unsigned int ptrace;          /* ptrace标志 */
    
    /* 进程标识符 */
    pid_t pid;                    /* 进程ID */
    pid_t tgid;                   /* 线程组ID */
    
    /* 用户标识符 */
    uid_t uid;                    /* 真实用户ID */
    gid_t gid;                    /* 真实组ID */
    uid_t euid;                   /* 有效用户ID */
    gid_t egid;                   /* 有效组ID */
    uid_t suid;                   /* 保存的用户ID */
    gid_t sgid;                   /* 保存的组ID */
    
    /* ... 其他字段 */
};
```

#### 2.2.2 进程调度相关

```c
struct task_struct {
    int prio, static_prio, normal_prio;
    unsigned int rt_priority;
    const struct sched_class *sched_class;
    struct sched_entity se;
    struct sched_rt_entity rt;
    
    /* 调度相关统计 */
    u64 utime;                    /* 用户态运行时间 */
    u64 stime;                    /* 内核态运行时间 */
    u64 gtime;                    /* 虚拟机运行时间 */
    unsigned long nvcsw;          /* 自愿上下文切换次数 */
    unsigned long nivcsw;         /* 非自愿上下文切换次数 */
    
    /* ... 其他字段 */
};
```

#### 2.2.3 进程亲属关系

```mermaid
graph LR
    A[parent] --> B[current task]
    B --> C[children 链表]
    B --> D[sibling 链表]
    E[real_parent] --> B
    
    style B fill:#3498db
    style A fill:#e74c3c
    style E fill:#f39c12
```

### 2.3 task_struct 的分配与释放

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant Sys as 系统调用
    participant Kernel as 内核
    participant Slab as Slab分配器
    
    App->>Sys: fork()/clone()
    Sys->>Kernel: 创建新进程
    Kernel->>Slab: 分配task_struct
    Slab-->>Kernel: 返回task_struct指针
    Kernel->>Kernel: 初始化task_struct
    Kernel->>Kernel: 复制父进程信息
    Kernel-->>Sys: 返回新进程PID
    Sys-->>App: 返回子进程PID
    
    Note over Kernel: 进程退出时
    Kernel->>Slab: 释放task_struct
    Slab-->>Kernel: 释放完成
```

---

## 三、进程状态

### 3.1 进程状态转换图

```mermaid
stateDiagram-v2
    [*] --> TASK_RUNNING: fork()
    TASK_RUNNING --> TASK_INTERRUPTIBLE: 等待资源
    TASK_RUNNING --> TASK_UNINTERRUPTIBLE: 等待IO
    TASK_RUNNING --> __TASK_TRACED: 被ptrace跟踪
    TASK_RUNNING --> __TASK_STOPPED: 收到SIGSTOP信号
    
    TASK_INTERRUPTIBLE --> TASK_RUNNING: 资源可用/收到信号
    TASK_UNINTERRUPTIBLE --> TASK_RUNNING: 等待完成
    
    __TASK_TRACED --> TASK_RUNNING: ptrace结束
    __TASK_STOPPED --> TASK_RUNNING: 收到SIGCONT信号
    
    TASK_RUNNING --> TASK_DEAD: exit()
    TASK_DEAD --> [*]: 等待父进程回收
    
    note right of TASK_RUNNING
        可运行状态
        可被调度执行
    end note
    
    note right of TASK_INTERRUPTIBLE
        可中断睡眠
        可被信号唤醒
    end note
    
    note right of TASK_UNINTERRUPTIBLE
        不可中断睡眠
        只能被事件唤醒
    end note
```

### 3.2 进程状态详细说明

| 状态 | 宏定义 | 说明 |
|------|--------|------|
| 可运行 | `TASK_RUNNING` | 进程正在CPU上运行或在运行队列中等待 |
| 可中断睡眠 | `TASK_INTERRUPTIBLE` | 进程被挂起，等待某个条件，可被信号唤醒 |
| 不可中断睡眠 | `TASK_UNINTERRUPTIBLE` | 进程被挂起，等待某个条件，不能被信号唤醒 |
| 被跟踪 | `__TASK_TRACED` | 进程被ptrace程序跟踪 |
| 已停止 | `__TASK_STOPPED` | 进程收到SIGSTOP等信号停止 |
| 已死亡 | `TASK_DEAD` | 进程已退出，等待父进程回收 |

### 3.3 进程状态操作函数

```c
// 设置进程状态
set_current_state(state);

// 设置进程状态并检查是否有抢占请求
set_current_state(state);
preempt_check_resched();

// 唤醒进程
wake_up_process(task);
wake_up_state(task, state);

// 进程睡眠
schedule();                // 主动让出CPU
wait_event(wq, condition); // 等待事件
```

---

## 四、进程创建

### 4.1 进程创建流程

```mermaid
flowchart TD
    A[应用程序调用fork] --> B[进入内核态]
    B --> C[do_fork函数]
    C --> D[copy_process函数]
    D --> E[分配task_struct]
    E --> F[复制进程信息]
    F --> G[复制内核栈]
    G --> H[复制文件描述符]
    H --> I[复制内存管理信息]
    I --> J[唤醒新进程]
    J --> K[返回子进程PID]
    
    F --> F1[复制寄存器]
    F --> F2[复制信号处理]
    F --> F3[复制调度信息]
    F --> F4[建立父子关系]
    
    style C fill:#3498db
    style D fill:#e74c3c
    style J fill:#2ecc71
```

### 4.2 copy_process 函数详解

```mermaid
graph TB
    A[copy_process] --> B[检查标志位]
    B --> C[分配task_struct]
    C --> D[复制进程标志]
    D --> E[复制PID]
    E --> F[复制进程凭证]
    F --> G[复制调度信息]
    G --> H[复制信号处理]
    H --> I[复制内存信息]
    I --> J[复制文件信息]
    J --> K[复制命名空间]
    K --> L[复制内核栈]
    L --> M[复制寄存器]
    M --> N[建立父子关系]
    N --> O[唤醒新进程]
    
    style A fill:#2ecc71
    style O fill:#f39c12
```

### 4.3 fork、vfork、clone 的区别

| 特性 | fork | vfork | clone |
|------|------|-------|-------|
| 创建方式 | 完全复制 | 共享地址空间 | 可定制复制 |
| 父子进程地址空间 | 独立 | 共享 | 可选 |
| 执行顺序 | 并发 | 子进程先执行 | 并发 |
| 用途 | 创建新进程 | 创建后立即exec | 创建线程 |
| 性能 | 较低 | 较高 | 高 |

### 4.4 写时复制（Copy-on-Write）

```mermaid
graph TB
    A[fork前] --> B[父进程]
    B --> C[物理页1]
    B --> D[物理页2]
    
    B --> E[fork]
    E --> F[fork后]
    F --> G[父子进程共享物理页]
    G --> H[页表项标记为只读]
    
    H --> I{进程写入?}
    I -->|否| J[继续共享]
    I -->|是| K[触发缺页异常]
    K --> L[复制物理页]
    L --> M[修改页表项]
    M --> N[各自独立]
    
    style C fill:#3498db
    style D fill:#3498db
    style G fill:#f39c12
    style N fill:#2ecc71
```

---

## 五、进程调度

### 5.1 调度器架构

Linux 6.x 使用 **CFS（Completely Fair Scheduler，完全公平调度器）** 作为普通进程的调度算法。

```mermaid
graph TB
    A[schedule函数] --> B[选择下一个进程]
    B --> C{调度类}
    C -->|实时进程| D[rt_sched_class]
    C -->|普通进程| E[fair_sched_class]
    C -->|空闲进程| E[idle_sched_class]
    
    D --> F[pick_next_task_rt]
    E --> G[pick_next_task_fair]
    
    F --> H[上下文切换]
    G --> H
    
    H --> I[切换到新进程]
    
    style A fill:#e74c3c
    style E fill:#3498db
    style H fill:#2ecc71
```

### 5.2 CFS 调度原理

CFS 使用红黑树来管理可运行进程，红黑树的键值是进程的虚拟运行时间（vruntime）。

```mermaid
graph TB
    A[红黑树] --> B[最左节点]
    B --> C[vruntime最小的进程]
    C --> D[下一个运行的进程]
    
    A --> E[节点结构]
    E --> F[进程task_struct]
    E --> G[vruntime]
    E --> H[红黑树节点]
    
    I[进程运行] --> J[增加vruntime]
    J --> K[重新插入红黑树]
    K --> L[保持平衡]
    
    style B fill:#e74c3c
    style C fill:#f39c12
```

### 5.3 vruntime 计算公式

```
vruntime = 实际运行时间 × (NICE_0_LOAD / 进程权重)
```

其中：
- 实际运行时间：进程在CPU上运行的时间
- NICE_0_LOAD：nice值为0的进程权重（默认为1024）
- 进程权重：由nice值决定

### 5.4 进程优先级

```mermaid
graph LR
    A[nice值] --> B[静态优先级]
    B --> C[动态优先级]
    C --> D[调度优先级]
    
    A -->|范围: -20~19| A1[-20: 最高优先级]
    A -->|范围: -20~19| A2[19: 最低优先级]
    
    C -->|受睡眠时间影响| C1[睡眠越久优先级越高]
    
    style A fill:#3498db
    style D fill:#e74c3c
```

### 5.5 调度时机

```mermaid
graph TB
    A[调度触发] --> B[主动调度]
    A --> C[抢占调度]
    
    B --> B1[进程调用schedule]
    B --> B2[进程阻塞]
    
    C --> C1[新进程唤醒]
    C --> C2[时间片耗尽]
    C --> C3[高优先级进程就绪]
    C --> C4[内核返回用户态前]
    
    style A fill:#e74c3c
    style B fill:#3498db
    style C fill:#f39c12
```

---

## 六、进程终止

### 6.1 进程终止流程

```mermaid
flowchart TD
    A[进程调用exit] --> B[do_exit函数]
    B --> C[释放资源]
    C --> D[释放文件描述符]
    D --> E[释放内存]
    E --> F[释放信号量]
    F --> G[设置退出状态]
    G --> H[通知父进程]
    H --> I[设置状态为TASK_DEAD]
    I --> J[schedule]
    J --> K[进程不再运行]
    
    L[父进程调用wait] --> M[do_wait函数]
    M --> N[查找子进程]
    N --> O[收集退出信息]
    O --> P[释放task_struct]
    P --> Q[子进程彻底消失]
    
    style B fill:#e74c3c
    style M fill:#3498db
    style P fill:#2ecc71
```

### 6.2 孤儿进程与僵尸进程

#### 僵尸进程
```mermaid
graph LR
    A[子进程退出] --> B[变为TASK_DEAD状态]
    B --> C[保留task_struct]
    C --> D[等待父进程回收]
    D -->|父进程未回收| E[僵尸进程]
    
    style E fill:#e74c3c
```

#### 孤儿进程
```mermaid
graph LR
    A[父进程先退出] --> B[子进程仍在运行]
    B --> C[子进程变为孤儿]
    C --> D[被init进程收养]
    D --> E[init负责回收]
    
    style D fill:#f39c12
```

### 6.3 exit 与 _exit 的区别

| 特性 | exit | _exit |
|------|------|-------|
| 实现位置 | glibc库函数 | 系统调用 |
| 清理工作 | 执行 | 不执行 |
| 调用atexit | 是 | 否 |
| 刷新stdio缓冲区 | 是 | 否 |
| 关闭文件描述符 | 是 | 否 |

---

## 七、进程间通信（IPC）

### 7.1 IPC 机制分类

```mermaid
graph TB
    A[进程间通信] --> B[管道]
    A --> C[消息队列]
    A --> D[共享内存]
    A --> E[信号量]
    A --> F[信号]
    A --> G[套接字]
    
    B --> B1[匿名管道]
    B --> B2[命名管道FIFO]
    
    style A fill:#e74c3c
    style D fill:#3498db
    style G fill:#2ecc71
```

### 7.2 管道通信

```mermaid
graph LR
    A[父进程] -->|写入| B[管道内核缓冲区]
    B -->|读取| C[子进程]
    
    B --> B1[单向通信]
    B --> B2[半双工]
    B --> B3[基于文件描述符]
    
    style A fill:#3498db
    style C fill:#3498db
    style B fill:#f39c12
```

### 7.3 共享内存

```mermaid
graph TB
    A[进程1] -->|访问| B[共享内存区域]
    C[进程2] -->|访问| B
    D[进程3] -->|访问| B
    
    B --> B1[最快的IPC方式]
    B --> B2[无需数据拷贝]
    B --> B3[需要同步机制]
    
    style B fill:#2ecc71
```

### 7.4 消息队列

```mermaid
graph LR
    A[进程1] -->|发送消息| B[消息队列]
    B -->|接收消息| C[进程2]
    B -->|接收消息| D[进程3]
    
    B --> B1[消息链表]
    B --> B2[按类型接收]
    B --> B3[支持优先级]
    
    style B fill:#f39c12
```

---

## 八、线程管理

### 8.1 线程实现方式

Linux 使用 **轻量级进程（LWP）** 来实现线程，线程和进程使用相同的 `task_struct` 结构。

```mermaid
graph TB
    A[进程] --> B[主线程]
    B --> C[task_struct]
    
    A --> D[线程1]
    D --> E[task_struct]
    E --> E1[共享进程资源]
    
    A --> F[线程2]
    F --> G[task_struct]
    G --> G1[共享进程资源]
    
    E1 --> H[共享内存空间]
    E1 --> I[共享文件描述符]
    E1 --> J[共享信号处理]
    
    E1 --> K[独立项]
    K --> K1[独立的PID]
    K --> K2[独立的栈]
    K --> K3[独立的寄存器]
    
    style A fill:#e74c3c
    style H fill:#3498db
    style K fill:#f39c12
```

### 8.2 线程创建（pthread_create）

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant Lib as pthread库
    participant Sys as 系统调用
    participant Kernel as 内核
    
    App->>Lib: pthread_create()
    Lib->>Sys: clone(CLONE_VM|CLONE_FS...)
    Sys->>Kernel: 创建新线程
    Kernel->>Kernel: 分配task_struct
    Kernel->>Kernel: 共享父进程资源
    Kernel->>Kernel: 创建独立栈
    Kernel-->>Sys: 返回新线程PID
    Sys-->>Lib: 返回成功
    Lib-->>App: 返回线程ID
    Kernel->>Kernel: 执行线程函数
```

### 8.3 线程同步

```mermaid
graph TB
    A[线程同步机制] --> B[互斥锁 mutex]
    A --> C[条件变量 condition]
    A --> D[读写锁 rwlock]
    A --> E[信号量 semaphore]
    A --> F[屏障 barrier]
    
    B --> B1[保护临界区]
    B --> B2[同一时刻只有一个线程]
    
    C --> C1[等待条件满足]
    C --> C2[通知机制]
    
    D --> D1[读者-写者问题]
    D --> D2[多读单写]
    
    style A fill:#e74c3c
    style B fill:#3498db
```

---

## 九、进程管理相关系统调用

### 9.1 进程控制

| 系统调用 | 功能 |
|----------|------|
| `fork()` | 创建子进程 |
| `vfork()` | 创建共享地址空间的子进程 |
| `clone()` | 创建进程或线程 |
| `execve()` | 执行新程序 |
| `exit()` | 终止当前进程 |
| `wait()` / `waitpid()` | 等待子进程终止 |
| `getpid()` | 获取进程ID |
| `getppid()` | 获取父进程ID |

### 9.2 进程调度

| 系统调用 | 功能 |
|----------|------|
| `nice()` | 设置进程nice值 |
| `setpriority()` | 设置进程优先级 |
| `getpriority()` | 获取进程优先级 |
| `sched_setscheduler()` | 设置调度策略 |
| `sched_getscheduler()` | 获取调度策略 |
| `sched_yield()` | 让出CPU |

### 9.3 进程间通信

| 系统调用 | 功能 |
|----------|------|
| `pipe()` | 创建管道 |
| `mkfifo()` | 创建命名管道 |
| `shmget()` | 创建共享内存 |
| `shmat()` | 附加共享内存 |
| `msgget()` | 创建消息队列 |
| `msgsnd()` | 发送消息 |
| `msgrcv()` | 接收消息 |
| `semget()` | 创建信号量集 |
| `semop()` | 操作信号量 |

---

## 十、进程管理调试工具

### 10.1 常用命令

```mermaid
graph TB
    A[进程管理工具] --> B[ps]
    A --> C[top]
    A --> D[htop]
    A --> E[pgrep]
    A --> F[pkill]
    A --> G[strace]
    A --> H[ltrace]
    
    B --> B1[查看进程快照]
    C --> C1[实时监控进程]
    D --> D1[交互式进程监控]
    E --> E1[查找进程]
    F --> F1[终止进程]
    G --> G1[跟踪系统调用]
    H --> H1[跟踪库函数调用]
    
    style A fill:#e74c3c
    style C fill:#3498db
    style G fill:#f39c12
```

### 10.2 /proc 文件系统

```mermaid
graph TB
    A[/proc文件系统] --> B[/proc/PID]
    B --> C[cmdline]
    B --> D[status]
    B --> E[stat]
    B --> F[maps]
    B --> G[fd]
    B --> H[stack]
    
    C --> C1[命令行参数]
    D --> D1[进程状态信息]
    E --> E1[进程统计信息]
    F --> F1[内存映射]
    G --> G1[打开的文件描述符]
    H --> H1[内核栈]
    
    style A fill:#2ecc71
    style B fill:#3498db
```

---

## 十一、总结

### 11.1 进程管理核心概念

1. **进程描述符（task_struct）**：内核管理进程的核心数据结构
2. **进程状态**：可运行、可中断睡眠、不可中断睡眠等
3. **进程创建**：fork、vfork、clone，使用写时复制优化
4. **进程调度**：CFS完全公平调度器，基于vruntime
5. **进程终止**：exit系统调用，父进程wait回收
6. **进程间通信**：管道、消息队列、共享内存、信号量等
7. **线程管理**：轻量级进程，共享进程资源

### 11.2 学习要点

```mermaid
mindmap
  root((进程管理))
    task_struct
      字段结构
      分配与释放
      亲属关系
    进程状态
      状态转换
      状态操作
      睡眠与唤醒
    进程创建
      fork/vfork/clone
      写时复制
      exec加载
    进程调度
      CFS调度器
      vruntime
      优先级
      调度时机
    进程终止
      exit流程
      僵尸进程
      孤儿进程
    IPC
      管道
      消息队列
      共享内存
      信号量
    线程
      轻量级进程
      pthread库
      同步机制
```

---

## 参考资源

- 《Linux内核设计与实现》
- 《图解Linux内核 基于6.x》
- Linux内核源码：https://github.com/torvalds/linux
- Linux内核文档：https://www.kernel.org/doc/html/latest/

---

*文档创建时间：2026年1月17日*
*内核版本参考：Linux 6.x*