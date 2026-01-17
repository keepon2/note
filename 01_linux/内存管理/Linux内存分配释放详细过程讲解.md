# Linux内存分配释放详细过程讲解

## 📋 目录
1. [内存分配完整流程](#内存分配完整流程)
2. [内存释放完整流程](#内存释放完整流程)
3. [关键数据结构](#关键数据结构)
4. [优化机制](#优化机制)
5. [性能监控](#性能监控)

---

## 🚀 内存分配完整流程

### 1. 整体架构概览

```mermaid
graph TB
    subgraph 用户空间
        A[用户程序<br/>malloc/free]
        B[glibc ptmalloc<br/>分配器]
    end
    
    subgraph 系统调用层
        C[brk系统调用<br/>小内存]
        D[mmap系统调用<br/>大内存]
    end
    
    subgraph 内核空间
        E[SLUB分配器<br/>kmalloc/kfree]
        F[伙伴系统<br/>alloc_pages/free_pages]
        G[页面分配<br/>物理内存]
    end
    
    A -->|请求内存| B
    B -->|<=128KB| C
    B -->|>128KB| D
    C --> E
    D --> E
    E --> F
    F --> G
    
    style A fill:#e1f5ff
    style G fill:#ffe1e1
```

### 2. 用户空间分配流程

```mermaid
flowchart TD
    Start([用户调用malloc size]) --> CheckSize{判断内存大小}
    
    CheckSize -->|<= 64字节| FastBin[Fast Bin<br/>单链表快速分配]
    CheckSize -->|64-512字节| SmallBin[Small Bin<br/>双向链表FIFO]
    CheckSize -->|512-128KB| LargeBin[Large Bin<br/>按大小排序]
    CheckSize -->|> 128KB| Mmap[mmap直接分配]
    
    FastBin --> IntMalloc[_int_malloc分配]
    SmallBin --> IntMalloc
    LargeBin --> IntMalloc
    
    IntMalloc --> CheckArena{检查Arena}
    CheckArena -->|有可用chunk| ReturnChunk[返回chunk指针]
    CheckArena -->|无可用chunk| SysCall[系统调用]
    
    SysCall --> Brk[brk系统调用]
    SysCall --> Mmap
    
    Brk --> ReturnChunk
    Mmap --> ReturnChunk
    
    ReturnChunk --> End([返回给用户])
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style FastBin fill:#FFE4B5
    style SmallBin fill:#FFE4B5
    style LargeBin fill:#FFE4B5
    style Mmap fill:#FFB6C1
```

### 3. ptmalloc分配策略选择

```mermaid
graph LR
    subgraph ptmalloc分配器
        A[malloc请求] --> B{大小判断}
        
        B -->|32-64字节| C[Fast Bin]
        B -->|64-512字节| D[Small Bin]
        B -->|512-128KB| E[Large Bin]
        B -->|>128KB| F[Huge Bin<br/>mmap]
        
        C --> G[单链表<br/>LIFO]
        D --> H[双向链表<br/>FIFO]
        E --> I[排序链表]
        F --> J[独立VMA]
        
        G --> K[快速分配释放]
        H --> K
        I --> K
        J --> K
    end
    
    style C fill:#FFD700
    style D fill:#FFA500
    style E fill:#FF8C00
    style F fill:#FF4500
```

### 4. 系统调用层分配

```mermaid
sequenceDiagram
    participant U as 用户程序
    participant G as glibc ptmalloc
    participant K as 内核
    participant B as brk系统调用
    participant M as mmap系统调用
    participant S as SLUB分配器
    participant P as 伙伴系统
    
    U->>G: malloc(size)
    
    alt size <= 128KB
        G->>K: brk(new_brk)
        K->>B: 执行brk
        B->>B: 检查brk地址合法性
        B->>B: 扩展或收缩堆区域
        B->>G: 返回新brk地址
    else size > 128KB
        G->>K: mmap(addr, len, prot, flags)
        K->>M: 执行mmap
        M->>M: 参数验证
        M->>M: 创建VMA
        M->>G: 返回映射地址
    end
    
    G->>S: kmalloc(size)
    S->>S: 查找kmem_cache
    S->>P: alloc_pages(order)
    P->>P: 从zone分配页面
    P->>S: 返回page
    S->>G: 返回对象指针
    G->>U: 返回内存指针
```

### 5. SLUB分配器详细流程

```mermaid
flowchart TD
    Start([kmalloc请求]) --> GetCPU[获取当前CPU缓存]
    GetCPU --> CheckFastPath{per-cpu freelist有对象?}
    
    CheckFastPath -->|是| AllocFast[快速路径分配]
    AllocFast --> UpdateFreelist[更新freelist指针]
    UpdateFreelist --> ReturnFast[返回对象]
    
    CheckFastPath -->|否| CheckPage{当前page有可用对象?}
    
    CheckPage -->|是| AllocFromPage[从page分配]
    AllocFromPage --> UpdatePage[更新page->freelist]
    UpdatePage --> CheckFull{page已满?}
    
    CheckFull -->|是| AddFull[添加到full链表]
    CheckFull -->|否| ReturnPage[返回对象]
    
    CheckPage -->|否| GetPartial[从node获取partial slab]
    GetPartial --> HasPartial{有partial slab?}
    
    HasPartial -->|是| UsePartial[使用partial slab]
    UsePartial --> AllocFromPage
    
    HasPartial -->|否| NewSlab[分配新slab页面]
    NewSlab --> AllocFromBuddy[从伙伴系统分配]
    AllocFromBuddy --> InitSlab[初始化slab]
    InitSlab --> AllocFromPage
    
    ReturnFast --> End([返回内存])
    ReturnPage --> End
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style AllocFast fill:#87CEEB
    style NewSlab fill:#FFB6C1
```

### 6. 伙伴系统分配算法

```mermaid
flowchart TD
    Start([alloc_pages order]) --> FindZone[查找合适的zone]
    FindZone --> TryDirect[尝试直接分配]
    TryDirect --> CheckOrder{order有可用块?}
    
    CheckOrder -->|是| AllocDirect[直接分配]
    AllocDirect --> ReturnPage[返回page]
    
    CheckOrder -->|否| FindBigger[查找更大的order]
    FindBigger --> LoopOrder[循环查找order到MAX_ORDER]
    LoopOrder --> CheckAvailable{当前order有可用?}
    
    CheckAvailable -->|是| SplitBlock[分裂块]
    CheckAvailable -->|否| NextOrder[检查下一个order]
    
    NextOrder --> CheckMax{达到MAX_ORDER?}
    CheckMax -->|否| LoopOrder
    CheckMax -->|是| SlowPath[进入慢速路径]
    
    SlowPath --> Reclaim[内存回收]
    SlowPath --> Compaction[内存整理]
    
    SplitBlock --> Expand[expand函数]
    Expand --> SplitLoop[循环分裂]
    SplitLoop --> SplitDone{分裂完成?}
    
    SplitDone -->|否| SplitLoop
    SplitDone -->|是| AddBuddy[伙伴添加到freelist]
    AddBuddy --> ReturnPage
    
    Reclaim --> Retry[重试分配]
    Compaction --> Retry
    
    style Start fill:#90EE90
    style ReturnPage fill:#90EE90
    style SlowPath fill:#FFB6C1
    style SplitBlock fill:#87CEEB
```

### 7. 伙伴系统页面分裂示例

```mermaid
graph TB
    subgraph 分配order=0 (1页)
        A0[Order 3: 8页] --> B0[分裂为两个Order 2]
        B0 --> C0[Order 2: 4页<br/>使用] & D0[Order 2: 4页<br/>空闲]
    end
    
    subgraph 分配order=1 (2页)
        A1[Order 3: 8页] --> B1[分裂为两个Order 2]
        B1 --> C1[Order 2: 4页] & D1[Order 2: 4页]
        C1 --> E1[分裂为两个Order 1]
        E1 --> F1[Order 1: 2页<br/>使用] & G1[Order 1: 2页<br/>空闲]
    end
    
    style A0 fill:#FFE4B5
    style A1 fill:#FFE4B5
    style C0 fill:#90EE90
    style F1 fill:#90EE90
```

---

## 🗑️ 内存释放完整流程

### 1. 整体释放流程

```mermaid
graph TB
    subgraph 用户空间
        A[用户程序<br/>free]
        B[glibc ptmalloc<br/>释放器]
    end
    
    subgraph 系统调用层
        C[brk收缩]
        D[munmap]
    end
    
    subgraph 内核空间
        E[SLUB分配器<br/>kfree]
        F[伙伴系统<br/>free_pages]
        G[页面回收<br/>物理内存]
    end
    
    A -->|释放内存| B
    B -->|判断类型| C
    B --> D
    C --> E
    D --> E
    E --> F
    F --> G
    
    style A fill:#e1f5ff
    style G fill:#ffe1e1
```

### 2. 用户空间释放流程

```mermaid
flowchart TD
    Start([用户调用free ptr]) --> CheckNull{指针为空?}
    CheckNull -->|是| ReturnNull[直接返回]
    CheckNull -->|否| GetChunk[获取chunk指针]
    
    GetChunk --> CheckMmap{是mmap分配?}
    
    CheckMmap -->|是| MunmapChunk[munmap_chunk]
    MunmapChunk --> SysMunmap[系统调用munmap]
    SysMunmap --> End([释放完成])
    
    CheckMmap -->|否| CheckFastBin{大小<=FASTBIN_MAX?}
    
    CheckFastBin -->|是| ReturnFastBin[放回fast bin]
    ReturnFastBin --> UpdateFastList[更新fast bin链表]
    UpdateFastList --> End
    
    CheckFastBin -->|否| CheckMerge{可合并?}
    
    CheckMerge -->|前chunk空闲| MergePrev[与前chunk合并]
    CheckMerge -->|后chunk空闲| MergeNext[与后chunk合并]
    CheckMerge -->|都可合并| MergeBoth[双向合并]
    
    MergePrev --> CheckMerge
    MergeNext --> CheckMerge
    MergeBoth --> CheckMerge
    
    CheckMerge -->|合并完成| CheckSize{合并后大小?}
    
    CheckSize -->|<MMAP_THRESHOLD| UnsortedBin[放回unsorted bin]
    CheckSize -->|>=MMAP_THRESHOLD| MunmapLarge[munmap大块]
    
    UnsortedBin --> End
    MunmapLarge --> SysMunmap
    
    ReturnNull --> End
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style MunmapChunk fill:#FFB6C1
    style MergeBoth fill:#87CEEB
```

### 3. SLUB释放详细流程

```mermaid
flowchart TD
    Start([kfree ptr]) --> CheckNull{指针为空?}
    CheckNull -->|是| ReturnNull[直接返回]
    CheckNull -->|否| GetPage[获取对应的page]
    
    GetPage --> CheckSlab{是SLAB页面?}
    
    CheckSlab -->|否| FreePages[调用伙伴系统释放]
    FreePages --> End([释放完成])
    
    CheckSlab -->|是| GetCPU[获取当前CPU缓存]
    GetCPU --> CheckFastPath{本地CPU且同一page?}
    
    CheckFastPath -->|是| FreeFast[快速路径释放]
    FreeFast --> SetFreelist[设置freelist指针]
    SetFreelist --> UpdateStat[更新统计信息]
    UpdateStat --> End
    
    CheckFastPath -->|否| FreeSlow[慢速路径释放]
    FreeSlow --> GetNode[获取node缓存]
    GetNode --> AddToFreelist[添加到page freelist]
    AddToFreelist --> DecInuse[page->inuse--]
    
    DecInuse --> CheckInuse{page->inuse?}
    
    CheckInuse -->|== 0| SlabEmpty[slab完全空闲]
    SlabEmpty --> RemovePartial[从partial移除]
    RemovePartial --> DiscardSlab[丢弃slab页面]
    DiscardSlab --> FreeToBuddy[释放到伙伴系统]
    
    CheckInuse -->|== objects-1| SlabPartial[从满变部分满]
    SlabPartial --> AddPartialNode[添加到node partial]
    
    CheckInuse -->|其他| UpdateInuse[仅更新inuse]
    
    FreeToBuddy --> End
    AddPartialNode --> End
    UpdateInuse --> End
    
    ReturnNull --> End
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style FreeFast fill:#87CEEB
    style SlabEmpty fill:#FFB6C1
```

### 4. 伙伴系统释放流程

```mermaid
flowchart TD
    Start([__free_pages page order]) --> CheckRef{引用计数为0?}
    CheckRef -->|否| ReturnRef[不释放]
    CheckRef -->|是| ResetPage[重置页面状态]
    
    ResetPage --> GetZone[获取所属zone]
    GetZone --> LockZone[获取zone锁]
    LockZone --> FreeOnePage[释放页面]
    
    FreeOnePage --> CheckFree{页面可释放?}
    CheckFree -->|否| ReturnFree[返回]
    CheckFree -->|是| MergeLoop[循环合并伙伴]
    
    MergeLoop --> CalcBuddy[计算伙伴PFN]
    CalcBuddy --> CheckBuddy{伙伴空闲且可合并?}
    
    CheckBuddy -->|否| BreakLoop[跳出循环]
    CheckBuddy -->|是| RemoveBuddy[从freelist移除伙伴]
    RemoveBuddy --> Combine[合并成更大块]
    Combine --> UpdateOrder[order++]
    
    UpdateOrder --> CheckMax{order < MAX_ORDER-1?}
    CheckMax -->|是| MergeLoop
    CheckMax -->|否| BreakLoop
    
    BreakLoop --> AddFreeList[添加到freelist]
    AddFreeList --> UpdateNrFree[nr_free++]
    UpdateNrFree --> UnlockZone[释放zone锁]
    UnlockZone --> End([释放完成])
    
    ReturnRef --> End
    ReturnFree --> End
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style Combine fill:#87CEEB
```

### 5. 伙伴系统合并示例

```mermaid
graph TB
    subgraph 释放order=0 (1页)
        A0[释放Order 0] --> B0{伙伴Order 0空闲?}
        B0 -->|是| C0[合并为Order 1]
        B0 -->|否| D0[直接放回freelist]
        
        C0 --> E0{伙伴Order 1空闲?}
        E0 -->|是| F0[合并为Order 2]
        E0 -->|否| G0[Order 1放回freelist]
    end
    
    subgraph 释放order=1 (2页)
        A1[释放Order 1] --> B1{伙伴Order 1空闲?}
        B1 -->|是| C1[合并为Order 2]
        B1 -->|否| D1[直接放回freelist]
    end
    
    style A0 fill:#FFE4B5
    style A1 fill:#FFE4B5
    style C0 fill:#87CEEB
    style F0 fill:#87CEEB
    style C1 fill:#87CEEB
    style D0 fill:#90EE90
    style D1 fill:#90EE90
```

### 6. munmap系统调用流程

```mermaid
flowchart TD
    Start([munmap addr len]) --> CheckLen{len == 0?}
    CheckLen -->|是| ReturnZero[返回0]
    CheckLen -->|否| AlignLen[PAGE_ALIGN len]
    
    AlignLen --> DoMunmap[do_munmap]
    DoMunmap --> FindVMA[查找对应VMA]
    FindVMA --> CheckVMA{VMA存在?}
    
    CheckVMA -->|否| ReturnError[返回-EINVAL]
    CheckVMA -->|是| CheckRange{地址范围合法?}
    
    CheckRange -->|否| ReturnZero
    CheckRange -->|是| CheckSplit{需要分割VMA?}
    
    CheckSplit -->|是| SplitVMA[分割VMA]
    CheckSplit -->|否| DetachVMA[分离VMA]
    
    SplitVMA --> DetachVMA
    DetachVMA --> UnmapRegion[unmap_region]
    
    UnmapRegion --> GatherTLB[收集TLB条目]
    GatherTLB --> UnmapVmas[解除页表映射]
    UnmapVmas --> FreePgtables[释放页表]
    FreePgtables --> FlushTLB[刷新TLB]
    
    FlushTLB --> End([解除映射完成])
    
    ReturnZero --> End
    ReturnError --> End
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style SplitVMA fill:#FFB6C1
    style UnmapRegion fill:#87CEEB
```

---

## 🏗️ 关键数据结构

### 1. 内存管理层次结构

```mermaid
classDiagram
    class Page {
        +unsigned long flags
        +atomic_t _count
        +void* virtual
        +struct zone* zone
        +unsigned int order
    }
    
    class Zone {
        +spinlock_t lock
        +unsigned long free_pages
        +FreeArea free_area[MAX_ORDER]
        +List lru
    }
    
    class FreeArea {
        +int nr_free
        +List free_list[MIGRATE_TYPES]
    }
    
    class KmemCache {
        +char* name
        +int object_size
        +int objects_per_slab
        +KmemCacheCPU* cpu_slab
        +KmemCacheNode* node
    }
    
    class KmemCacheCPU {
        +void** freelist
        +Page* page
        +unsigned int tid
        +unsigned int node
    }
    
    class KmemCacheNode {
        +spinlock_t list_lock
        +List partial
        +List full
    }
    
    Zone *-- FreeArea
    Zone *-- Page
    KmemCache *-- KmemCacheCPU
    KmemCache *-- KmemCacheNode
    KmemCacheCPU o-- Page
```

### 2. ptmalloc数据结构

```mermaid
classDiagram
    class MallocState {
        +Mutex mutex
        +mchunkptr fastbinsY[NFASTBINS]
        +mchunkptr bins[NBINS]
        +unsigned int binmap
        +mchunkptr top
    }
    
    class MallocChunk {
        +size_t prev_size
        +size_t size
        +fd, bk
    }
    
    class HeapInfo {
        +mchunkptr ar_ptr
        +size_t size
        +mchunkptr mprotect_size
        +HeapInfo* prev
        +HeapInfo* next
    }
    
    MallocState *-- MallocChunk
    MallocState *-- HeapInfo
    MallocChunk --> MallocChunk : fd/bk链表
```

---

## ⚡ 优化机制

### 1. Per-CPU缓存优化

```mermaid
graph TB
    subgraph 传统方式
        A1[CPU0] -->|加锁| B1[全局缓存]
        A2[CPU1] -->|加锁| B1
        A3[CPU2] -->|加锁| B1
        B1 --> C1[锁竞争严重]
    end
    
    subgraph Per-CPU方式
        A4[CPU0] -->|无锁| B4[CPU0缓存]
        A5[CPU1] -->|无锁| B5[CPU1缓存]
        A6[CPU2] -->|无锁| B6[CPU2缓存]
        B4 --> C4[无锁竞争]
        B5 --> C4
        B6 --> C4
    end
    
    style C1 fill:#FFB6C1
    style C4 fill:#90EE90
```

### 2. SLAB颜色算法

```mermaid
graph LR
    subgraph 无颜色
        A1[SLAB1] -->|缓存行0| B1[对象1]
        A2[SLAB2] -->|缓存行0| B2[对象1]
        B1 -.->|冲突| B2
    end
    
    subgraph 有颜色
        A3[SLAB1] -->|缓存行0| B3[对象1]
        A4[SLAB2] -->|缓存行1| B4[对象1]
        A5[SLAB3] -->|缓存行2| B5[对象1]
        B3 -.->|无冲突| B4
        B4 -.->|无冲突| B5
    end
    
    style B1 fill:#FFB6C1
    style B2 fill:#FFB6C1
    style B3 fill:#90EE90
    style B4 fill:#90EE90
    style B5 fill:#90EE90
```

### 3. 批量操作优化

```mermaid
sequenceDiagram
    participant CPU as CPU
    participant Local as Per-CPU缓存
    participant Node as Node缓存
    participant Buddy as 伙伴系统
    
    Note over CPU,Buddy: 批量分配
    
    CPU->>Local: 检查本地缓存
    alt 本地缓存为空
        CPU->>Node: 从partial获取slab
        alt partial为空
            CPU->>Buddy: 分配新页面
            Buddy-->>CPU: 返回page
            CPU->>Local: 填充本地缓存
        else partial有slab
            Node-->>CPU: 返回slab
            CPU->>Local: 填充本地缓存
        end
    end
    CPU-->>CPU: 快速分配对象
    
    Note over CPU,Buddy: 批量释放
    
    CPU->>Local: 释放对象到本地缓存
    alt 本地缓存满
        CPU->>Node: 刷新到node缓存
        Node->>Node: 检查slab状态
        alt slab完全空闲
            Node->>Buddy: 释放页面
        end
    end
```

---

## 📊 性能监控

### 1. 监控命令流程

```mermaid
graph TB
    Start([监控命令]) --> CatSlab[cat /proc/slabinfo]
    Start --> CatBuddy[cat /proc/buddyinfo]
    Start --> CatMem[cat /proc/meminfo]
    Start --> CatSmaps[cat /proc/pid/smaps]
    
    CatSlab --> ShowSlab[显示SLAB缓存信息]
    ShowSlab --> SlabData[active_objs, num_objs,<br/>objsize, objperslab]
    
    CatBuddy --> ShowBuddy[显示伙伴系统信息]
    ShowBuddy --> BuddyData[各order的空闲页面数]
    
    CatMem --> ShowMem[显示内存使用情况]
    ShowMem --> MemData[总内存, 空闲内存,<br/>缓存, 缓冲区]
    
    CatSmaps --> ShowSmaps[显示进程内存映射]
    ShowSmaps --> SmapsData[VMA详情,<br/>RSS, PSS, Shared]
    
    style Start fill:#90EE90
    style SlabData fill:#E6E6FA
    style BuddyData fill:#E6E6FA
    style MemData fill:#E6E6FA
    style SmapsData fill:#E6E6FA
```

### 2. 内存统计结构

```mermaid
classDiagram
    class VmStat {
        +unsigned long nr_free_pages
        +unsigned long nr_inactive_anon
        +unsigned long nr_active_anon
        +unsigned long nr_inactive_file
        +unsigned long nr_active_file
        +unsigned long nr_slab_reclaimable
        +unsigned long nr_slab_unreclaimable
        +unsigned long nr_page_table_pages
        +unsigned long nr_kernel_stack
    }
    
    class SlabInfo {
        +char name[32]
        +unsigned long active_objs
        +unsigned long num_objs
        +unsigned long objsize
        +unsigned long objperslab
        +unsigned long pagesperslab
    }
    
    class BuddyInfo {
        +int node
        +char zone[16]
        +unsigned long free[MAX_ORDER]
    }
```

---

## 🎯 总结

### 内存分配关键路径

```mermaid
flowchart LR
    A[malloc] --> B[ptmalloc]
    B --> C[brk/mmap]
    C --> D[SLUB]
    D --> E[伙伴系统]
    E --> F[物理页面]
    
    style A fill:#90EE90
    style F fill:#FFB6C1
```

### 内存释放关键路径

```mermaid
flowchart LR
    A[free] --> B[ptmalloc]
    B --> C[munmap/brk]
    C --> D[SLUB]
    D --> E[伙伴系统]
    E --> F[物理页面回收]
    
    style A fill:#90EE90
    style F fill:#FFB6C1
```

### 调试命令速查

```bash
# SLAB缓存信息
cat /proc/slabinfo

# 伙伴系统信息
cat /proc/buddyinfo

# 内存使用情况
cat /proc/meminfo

# 进程内存映射
cat /proc/<pid>/smaps

# 内存分配跟踪
echo 1 > /proc/sys/vm/drop_caches
```

通过以上图表，可以清晰地理解Linux内存分配释放的完整流程和关键机制！