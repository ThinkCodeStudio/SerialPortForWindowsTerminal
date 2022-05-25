# SerialPortForWindowsTerminal
Serial port for windows terminal, 让Windows Terminal支持串口的插件

该插件为WindowsTerminal提供访问串口终端能力


## 使用说明

程序可以在终端中正常使用，`Ctrl+Q`退出程序  
```shell
SerialPortForWindowsTerminal.exe <串口> [其他参数]
```
## 参数说明
```shell
#程序帮助
SerialPortForWindowsTerminal.exe COMx -h
```
|参数|说明|选项|默认值|
|:--:|:--|:--:|:--:|
|-br|波特率|无|9600|
|-cs|数据位|无|8|
|-sb|停止位|none,onepointfive,two|none|
|-p|校验模式|none,add,even|none|
|-fc|流控模式|none,software,hardware|none|

## 配置说明

打开windows terminal进入设置 > 添加新配置文件 > 新建空配置文件 > 在命令行添加命令 > 保存  
```shell
<你的的程序在计算机的位置> COM1 -br 115200
```