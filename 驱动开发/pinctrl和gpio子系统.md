# 目录

## pinctrl子系统
### pinctrl 子系统简介
Linux 驱动讲究驱动分离与分层， pinctrl 和 gpio 子系统就是驱动分离与分层思想下的产物，驱动分离与分层其实就是按照面向对象编程的设计思想而设计的设备驱动框架。  
如果使用过 STM32 的话应该都记得， STM32 也是要先设置某个 PIN 的复用功能、速度、上下拉等，然后再设置 PIN 所对应的 GPIO。  
大多数 SOC 的 pin 都是支持复用的，比如 I.MX6ULL 的 GPIO1_IO03 既可以作为普通的GPIO 使用，也可以作为 I2C1 的 SDA 等等。  

```sh
传统的配置 pin 的方式就是直接操作相应的寄存器，但是这种配置方式比较繁琐、而且容易出问题(比如 pin 功能冲突)。 pinctrl 子系统就是为了解决这个问题而引入的， pinctrl 子系统主要工作内容如下：
①、获取设备树中 pin 信息。
②、根据获取到的 pin 信息来设置 pin 的复用功能
③、根据获取到的 pin 信息来设置 pin 的电气特性，比如上/下拉、速度、驱动能力等。

对于我们使用者来讲，只需要在设备树里面设置好某个 pin 的相关属性即可，其他的初始
化工作均由 pinctrl 子系统来完成， pinctrl 子系统源码目录为 drivers/pinctrl。
```

### pinctrl 子系统主要工作内容
获取设备树中 pin 信息，管理系统中所有的可以控制的 pin， 在系统初始化的时候， 枚举所有可以控制的 pin， 并标识这些 pin根据获取到的 pin 信息来设置 pin 的复用功能，对于 SOC 而言， 其引脚除了配置成普通的 GPIO 之外，若干个引脚还可以组成一个 pin group， 形成特定的功能根据获取到的 pin 信息来设置 pin 的电气特性，比如上/下拉、速度、驱动能力等。  


### pinctrl的设备树设置
在设备树里面创建一个节点来描述 PIN 的配置信息。pinctrl 子系统一般在iomuxc子节点下，所有需要配置用户自己的pinctrl需要在该节点下添加。  
```sh
iomuxc: iomuxc@020e0000 {
	compatible = "fsl,imx6ul-iomuxc";
	reg = <0x020e0000 0x4000>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;
	imx6ul-evk {
		pinctrl_hog_1: hoggrp-1 {
			fsl,pins = <
				MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059
				.......
			>;
	};
......
}
```

### 设备树中添加pinctrl模板
1.添加pinctrl设备结点  
同一个外设的 PIN 都放到一个节点里面，在 iomuxc 节点中下添加“pinctrl_test”节点。节点前缀一定要为“pinctrl_”。  
设备树是通过属性来保存信息的，因此需要添加一个属性，属性名字一定要为** fsl,pins **  
```sh
&iomuxc {
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;
	imx6ul-evk {
		......
			pinctrl_led: ledgrp{
						fsl,pins = <
							MX6UL_PAD_GPIO1_IO03__GPIO1_IO03	0x10b0
						>;
				};
		......
	};
```

2.添加具体设备节点，调用pinctrl信息    
在根节点“/”下创建 LED 灯节点，节点名为“gpioled”  
只需要关注gpioled设备节点下的pinctrl-names 和 pinctrl-0 两条语句， 这两句就是引用在 iomuxc 中配置的 pinctrl 节点  
```sh
test {
    pinctrl-names = "default"， "wake up";
    pinctrl-0 = <&pinctrl_test>;
    pinctrl-1 = <&pinctrl_test_2>;
    /* 其他节点内容 */
};
pinctrl-names = “default”， “wake up”; 设备的状态， 可以有多个状态， default 为状态 0， wake up 为状态 1。

pinctrl-0 = <&pinctrl_test>;第 0 个状态所对应的引脚配置， 也就是 default 状态对应的引脚在 pin controller 里面定义好的节点 pinctrl_test里面的管脚配置。

pinctrl-1 = <&pinctrl_test_2>;第 1 个状态所对应的引脚配置， 也就是 wake up 状态对应的引脚在 pin controller 里面定义好的节点 pinctrl_test_2里面的管脚配置。
```



## gpio子系统
**当使用 pinctrl 子系统将引脚的复用设置为 GPIO，可以使用 GPIO 子系统来操作GPIO  **  

gpio 子系统顾名思义，就是用于初始化 GPIO 并且提供相应的 API 函数，比如设置 GPIO为输入输出，读取 GPIO 的值等。   
gpio 子系统的主要目的就是方便驱动开发者使用 gpio，驱动开发者在设备树中添加 gpio 相关信息，然后就可以在驱动程序中使用 gpio 子系统提供的 API 函数来操作 GPIO， Linux 内核向驱动开发者屏蔽掉了 GPIO 的设置过程，极大的方便了驱动开发者使用 GPIO。

### GPIO子系统工作内容
引脚功能的配置（设置为 GPIO，GPIO 的方向， 输入输出模式，读取/设置 GPIO 的值）  
实现软硬件的分离（分离出硬件差异， 有厂商提供的底层支持； 软件分层。 驱动只需要调用接口 API 即可操作 GPIO）  
iommu 内存管理（直接调用宏即可操作 GPIO）  

### GPIO子系统设备树设置
在具体设备节点中添加GPIO信息  
```sh
gpioled {
		#address-cells = <1>;
		#size-cells = <1>;
		compatible = "songwei-gpioled";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_led>;
		//gpio信息
		led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;
		status = "okay";
	};
led-gpio 属性指定了 LED 灯所使用的 GPIO，在这里就是 GPIO1 的 IO03，低电平有效。
稍后编写驱动程序的时候会获取 led-gpio 属性的内容来得到 GPIO 编号，因为 gpio 子系统的 API 操作函数需要 GPIO 编号
```

### API函数
#### gpio_request  
gpio_request 函数用于申请一个 GPIO 管脚，在使用一个 GPIO 之前一定要使用 gpio_request进行申请。
```sh
int gpio_request(unsigned gpio, const char *label);

gpio：要申请的 gpio 标号，使用 of_get_named_gpio 函数从设备树获取指定 GPIO 属性信息，此函数会返回这个 GPIO 的标号
label：给 gpio 设置个名字
```
#### gpio_free  
如果不使用某个 GPIO ，需要调用 gpio_free 函数进行释放。
```sh
void gpio_free(unsigned gpio)；  // gpio：要释放的 gpio 标号。
```

#### gpio_direction_input  
设置某个 GPIO 为输入  
```sh
int gpio_direction_input(unsigned gpio) //gpio：要设置为输入的 GPIO 标号。
```

#### gpio_direction_output  
设置某个 GPIO 为输出，并且设置默认输出值。  
```sh
int gpio_direction_output(unsigned gpio, int value)

gpio：要设置为输出的 GPIO 标号。
value： GPIO 默认输出值。   
```

#### gpio_get_value  
获取某个 GPIO 的值(0 或 1)  
```sh
#define gpio_get_value __gpio_get_value
int __gpio_get_value(unsigned gpio)

gpio：要获取的 GPIO 标号。
返回值： 非负值，得到的 GPIO 值；负值，获取失败
```

#### gpio_set_value  
设置某个 GPIO 的值  
```sh
#define gpio_set_value __gpio_set_value
void __gpio_set_value(unsigned gpio, int value)

gpio：要设置的 GPIO 标号。
value： 要设置的值
```

### GPIO相关OF函数
#### of_gpio_named_count
获取设备树某个属性里面定义了几个GPIO 信息。  
```sh
int of_gpio_named_count(struct device_node *np, const char *propname);

np：设备节点。
propname：要统计的 GPIO 属性。
返回值： 正值，统计到的 GPIO 数量；负值，失败
```
#### of_gpio_count
此函数统计的是gpios属性的 GPIO 数量，而 of_gpio_named_count 函数可以统计任意属性的GPIO 信息
```sh
int of_gpio_count(struct device_node *np);

np：设备节点。
返回值： 正值，统计到的 GPIO 数量；负值，失败
```

#### of_get_named_gpio
获取 GPIO 编号，在Linux 内核中关于 GPIO 的 API 函数都要使用 GPIO 编号，此函数会将设备树中类似<&gpio5 7 GPIO_ACTIVE_LOW>的属性信息转换为对应的 GPIO 编号。  
```sh
int of_get_named_gpio(struct device_node *np,
                       const char *propname,
                       int index)
np：设备节点。
propname：包含要获取 GPIO 信息的属性名。
index： GPIO 索引，因为一个属性里面可能包含多个 GPIO，此参数指定要获取哪个 GPIO的编号，如果只有一个 GPIO 信息的话此参数为 0。
返回值： 正值，获取到的 GPIO 编号；负值，失败。
```