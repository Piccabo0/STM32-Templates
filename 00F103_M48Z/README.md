# STM32F103C8T6-M48Z 最小系统板工程文件

STM32F103C8T6-M48Z 的 Keil工程模板，保留了 STM32F10x 标准外设库、CMSIS 启动文件和 FreeRTOS 基础框架。

## 板载引脚信息

根据 STM32F103C8T6-M48Z 最小系统板硬件参考手册见：http://www.openedv.com/docs/boards/xiaoxitongban/M48Z-M3.html，主要使用以下引脚：

| 功能 | GPIO | 说明 |
| --- | --- | --- |
| WKUP 按键 | PA0 | KEY_UP 按键的 WK_UP 信号 |
| LED0 | PA8 | LED0 的 LED 信号 |
| USART1_TX | PA9 | 串口 1 发送 |
| USART1_RX | PA10 | 串口 1 接收 |
| USB_DM | PA11 | USB SLAVE 接口 USB_D- 信号 |
| USB_DP | PA12 | USB SLAVE 接口 USB_D+ 信号 |
| SWDIO | PA13 | SWD 调试接口 SWDIO |
| SWCLK | PA14 | SWD 调试接口 SWCLK |
| OSC32_IN | PC14 | 32.768 kHz 晶振输入 |
| OSC32_OUT | PC15 | 32.768 kHz 晶振输出 |
| OSC_IN | PD0 | 8 MHz 晶振输入 |
| OSC_OUT | PD1 | 8 MHz 晶振输出 |

## 当前基本功能

- FreeRTOS 已集成，入口在 `USER/main.c`，任务创建集中在 `USER/MyTask.c`。
- LED0 使用 `PA8`，低电平点亮，高电平熄灭。
- WKUP 按键使用 `PA0`，每按下一次切换 LED0 的亮灭状态。
- USART1 使用 `PA9/PA10`，默认波特率 `9600`，8 数据位、1 停止位、无校验。
- 串口接收以 `0xFF` 开头、以 `0xFE` 结尾的数据帧，收到完整帧后会将该帧原样发送回去。
- 串口最大帧长当前为 `64` 字节，可在 `USER/MyTask.c` 中修改 `SERIAL_FRAME_MAX_LEN`。

## 工程结构

| 目录 | 说明 |
| --- | --- |
| `CORE/` | CMSIS 内核文件和启动文件 |
| `FWLIB/` | STM32F10x 标准外设库 |
| `FreeRTOS/` | FreeRTOS 源码、配置和移植层 |
| `USER/` | 用户代码、系统中断文件和 Keil 工程文件 |

`OBJ/`、`USER/Objects/`、`USER/Listings/` 为 Keil 编译生成目录，不属于模板源码内容。
