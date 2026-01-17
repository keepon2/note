# 目录

    1.如果这个工程没有编译过，那么我们的所有C文件都要编译并被链接。  
    2.如果这个工程的某几个C文件被修改，那么我们只编译被修改的C文件，并链接目标程序。  
    3.如果这个工程的头文件被改变了，那么我们需要编译引用了这几个头文件的C文件，并链接目标程序。



## makefile赋值
|赋值|说明|
|:-:|-|
|`=`|基本的赋值 会在makefile的最后才赋值|
|`:=`|覆盖之前的值 会立即赋值|
|`?=`|如果没有赋值过就赋值|
|`+=`|添加后面的值|


|符号|含义|
|---|---|
|$@|目标文件|
|$^|所有依赖文件|
|$+|所有依赖文件|
|$<|第一个依赖文件|
|$?|所有比目标新的依赖文件|

```sh
CROSS_COMPILE = arm-none-eabi-  //交叉编译  
CC  = $(CROSS_COMPILE)gcc  
LD  = $(CROSS_COMPILE)ld  
OBJCOPY = $(CROSS_COMPILE)objcopy  
APP = fs4412.elf  
BIN = fs4412.bin
CINCLUDES = -I ./include    //头文件路径

LDFLAGS += -static -L ./lib -lc -lm -lnosys
LDFLAGS += -static -L ./lib -lgcc
-static:静态链接  
-L ./lib: 指定了链接器搜索库文件的路径  
-lc: 告诉链接器链接 C 标准库
-lm: 告诉链接器链接数学库
-lnosys :告诉链接器链接一个名为 nosys 的库

include config.mk
//当 Makefile 在执行时遇到 include 指令，它会暂停当前文件的执行，去读取指定的文件

#COBJS += driver/led.o
#COBJS += driver/key.o
#COBJS += driver/beep.o
#COBJS += driver/music.o
#COBJS += driver/uart.o
#COBJS += driver/interrupt.o
#COBJS += driver/key-interrupt.o
#COBJS += driver/iic_con.o
#COBJS += driver/mpu6050.o

#目标:依赖
#	命令

$(APP):start/start.o main.o  $(COBJS)
	$(LD) -Ttext=0x40000000 $^ -o $@ $(LDFLAGS)
	$(OBJCOPY) -O binary $(APP) $(BIN)
	
	
%.o:%.S
	$(CC) -c $< -o $@
	
#start/start.o:start/start.S
#	arm-none-eabi-gcc -c start/start.s -o start/start.o
	

#%.o:%.c
#	$(CC) -c $< -o $@  $(CINCLUDES)

driver/led.o:driver/led.c
	arm-none-eabi-gcc -c driver/led.c -o driver/led.o -I ./include
	
clean:
	rm -rf driver/*.o
```