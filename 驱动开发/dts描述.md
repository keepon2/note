# 目录

Linux只能通过系统调用和硬件中断从用户空间进入内核空间

执行系统调用的内核代码运行在进程上下文中，他代表调用进程执行操作，因此能够访问进程地址空间的所有数据  
处理硬件中断的内核代码运行在中断上下文中，他和进程是异步的，与任何一个特定进程无关通常，一个驱动程序模块中的某些函数作为系统调用的一部分，而其他函数负责中断处理

**label: node-name@unit-address**  
label:节点标签，方便访问节点：通过&label访问节点，追加节点信息
node-name：节点名字，为字符串，描述节点功能  
unit-address：设备的地址或寄存器首地址，若某个节点没有地址或者寄存器，可以省略

**#address-cells 和#size-cells 属性**  
用于描述子节点的地址信息,reg属性的address 和 length的字长。
#address-cells 属性值决定了子节点 reg 属性中地址信息所占用的字长(32 位)  
#size-cells 属性值决定了子节点 reg 属性中长度信息所占的字长(32 位)。  
子节点的地址信息描述来自于父节点的#address-cells 和#size-cells的值，而不是该节点本身的值（当前节点的信息是描述子节点的，自己的信息在父节点里）

model 属性值是一个字符串，一般 model 属性描述设备模块信息。  
reg 属性一般用于描述设备地址空间资源信息  

ranges属性值可以为空或者按照( child-bus-address , parent-bus-address , length )格式编写的数字  
ranges 是一个地址映射/转换表， ranges 属性每个项目由子地址、父地址和地址空间长度这三部分组成。  
如果 ranges 属性值为空值，说明子地址空间和父地址空间完全相同，不需要进行地址转换。

在根节点“/”中有两个特殊的子节点： aliases 和 chosen

aliases 节点的主要功能就是定义别名，定义别名的目的就是为了方便访问节点。  	
但是，一般会在节点命名的时候会加上 label，然后通过&label来访问节点。

chosen 不是一个真实的设备， chosen 节点主要是为了 uboot 向 Linux 内核传递数据(bootargs 参数)。

### OF操作函数
Linux 内核提供了一系列的函数来获取设备树中的节点或者属性信息，这一系列的函数都有一个统一的前缀“of_” (称为OF 函数）  

#### 查找属性
Linux 内核使用 device_node 结构体来描述一个节点：  
```sh
struct device_node {
    const char *name; /* 节点名字 */
    const char *type; /* 设备类型 */
    phandle phandle;
    const char *full_name; /* 节点全名 */
    struct fwnode_handle fwnode;
 
    struct property *properties; /* 属性 */
    struct property *deadprops; /* removed 属性 */
    struct device_node *parent; /* 父节点 */
    struct device_node *child; /* 子节点
    ...
}
```
通过节点名字查找指定的节点：of_find_node_by_name  
```sh
struct device_node *of_find_node_by_name(struct device_node *from,const char *name)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
name：要查找的节点名字。
返回值： 找到的节点，如果为 NULL 表示查找失败。
```

通过 device_type 属性查找指定的节点：of_find_node_by_type
```sh
struct device_node *of_find_node_by_type(struct device_node *from, const char *type)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
type：要查找的节点对应的 type 字符串， device_type 属性值。
返回值： 找到的节点，如果为 NULL 表示查找失败
```

通过device_type 和 compatible两个属性查找指定的节点：of_find_compatible_node  
```sh
struct device_node *of_find_compatible_node(struct device_node *from,
                                            const char *type,
                                            const char *compatible)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
type：要查找的节点对应的 type 字符串，device_type 属性值，可以为 NULL
compatible： 要查找的节点所对应的 compatible 属性列表。
返回值： 找到的节点，如果为 NULL 表示查找失败
```
通过 of_device_id 匹配表来查找指定的节点：of_find_matching_node_and_match
```sh
struct device_node *of_find_matching_node_and_match(struct device_node *from,
                                            const struct of_device_id *matches,
                                            const struct of_device_id **match)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
matches： of_device_id 匹配表，在此匹配表里面查找节点。
match： 找到的匹配的 of_device_id。
返回值： 找到的节点，如果为 NULL 表示查找失败
```

通过路径来查找指定的节点：of_find_node_by_path
```sh
inline struct device_node *of_find_node_by_path(const char *path)
path：设备树节点中绝对路径的节点名，可以使用节点的别名
返回值： 找到的节点，如果为 NULL 表示查找失败
```

#### 获取属性值
Linux 内核中使用结构体 property 表示属性
```sh
struct property {
    char *name; /* 属性名字 */
    int length; /* 属性长度 */
    void *value; /* 属性值 */
    struct property *next; /* 下一个属性 */
    unsigned long _flags;
    unsigned int unique_id;
    struct bin_attribute attr;
}
```

查找指定的属性：of_find_property
```sh
property *of_find_property(const struct device_node *np,
                           const char *name,
                           int *lenp)
np：设备节点。
name： 属性名字。
lenp：属性值的字节数，一般为NULL
返回值： 找到的属性。
```

获取属性中元素的数量(数组)：of_property_count_elems_of_size
```sh
int of_property_count_elems_of_size(const struct device_node *np,
                                    const char *propname
                                    int elem_size)
np：设备节点。
proname： 需要统计元素数量的属性名字。
elem_size：元素长度。
返回值： 得到的属性元素数量
```

从属性中获取指定标号的 u32 类型数据值:of_property_read_u32_index
```sh
int of_property_read_u32_index(const struct device_node *np,
                                const char *propname,
                                u32 index,
                                u32 *out_value)
np：设备节点。
proname： 要读取的属性名字。
index：要读取的值标号。
out_value：读取到的值
返回值： 0 读取成功;
负值: 读取失败，
-EINVAL 表示属性不存在
-ENODATA 表示没有要读取的数据，
-EOVERFLOW 表示属性值列表太小
```

读取属性中 u8、 u16、 u32 和 u64 类型的数组数据
```sh
of_property_read_u8_array
of_property_read_u16_array 
of_property_read_u32_array 
of_property_read_u64_array 
int of_property_read_u8_array(const struct device_node *np,
                                const char *propname,
                                u8 *out_values,
                                size_t sz)
np：设备节点。
proname： 要读取的属性名字。
out_value：读取到的数组值，分别为 u8、 u16、 u32 和 u64。
sz： 要读取的数组元素数量。
返回值： 0：读取成功;
负值: 读取失败
-EINVAL 表示属性不存在
-ENODATA 表示没有要读取的数据
-EOVERFLOW 表示属性值列表太小
```

读取属性中字符串值：of_property_read_string
```sh
int of_property_read_string(struct device_node *np,
                            const char *propname,
                            const char **out_string)
np：设备节点。
proname： 要读取的属性名字。
out_string：读取到的字符串值。
返回值： 0，读取成功，负值，读取失败
```

获取 #address-cells 属性值：of_n_addr_cells ，获取 #size-cells 属性值：of_size_cells 。
```sh
int of_n_addr_cells(struct device_node *np)
int of_n_size_cells(struct device_node *np)
np：设备节点。
返回值： 获取到的#address-cells 属性值。
返回值： 获取到的#size-cells 属性值。
```

### 内存映射
我们之前逻辑操作可以直接操作寄存器，但现在在linux操作系统中，有着严格的内存管理。
在操作系统中我们能看到的地址都是映射后的地址（虚拟地址）

```sh
of_iomap 函数用于直接内存映射，前面通过 ioremap 函数来完成物理地址到虚拟地址的映射，采用设备树以后就可以直接通过 of_iomap 函数来获取内存地址所对应的虚拟地址。这样就不用再去先获取reg属性值，再用属性值映射内存。

of_iomap 函数本质上也是将 reg 属性中地址信息转换为虚拟地址，如果 reg 属性有多段的话，可以通过 index 参数指定要完成内存映射的是哪一段， of_iomap 函数原型如下：
void __iomem *of_iomap(struct device_node *np,  int index)
np：设备节点。
index： reg 属性中要完成内存映射的段，如果 reg 属性只有一段的话 index 就设置为 0。
返回值： 经过内存映射后的虚拟内存首地址，如果为 NULL 的话表示内存映射失败。
```
