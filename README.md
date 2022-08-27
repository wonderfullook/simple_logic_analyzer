# 极简逻辑分析仪

基于 [SIPEED Tang Primer 20K](https://en.wiki.sipeed.com/hardware/zh/tang/tang-primer-20k/primer-20k.html)
和 FT2232H 开发的极简逻辑分析仪：

* 8 路采样
* 采样率可调（`COUNTER_SIZE`），默认采用率 7.5MHz

![](./res/overview.jpg)

## 目录说明

* app：FT2232H 简单测试程序，使用 Visual Studio 编译；
* fpga：高云项目

## 准备工作

1. [下载](http://www.gowinsemi.com.cn/faq.aspx) 安装高云教育版 IDE；

1. 用[这个下载器](https://dl.sipeed.com/shareURL/TANG/programmer) 替换 IDE 里默认的下载器；

1. 下载安装 Visual Studio 2022 Community 用于编译桌面软件；

1. 准备一块 [FT2232H 开发版](https://m.tb.cn/h.UZyYVa9?tk=kiAu2uOPLSL)。

    * 使用 [FT_Prog](https://ftdichip.cn/Support/Utilities.htm#FT_PROG) 将端口 A 配置为 FIFO 模式

## 原理说明

FT2232H 输出的 60MHz CLKOUT 信号经 `counter` 分频后，触发对 8 路 `capture` 信号进行采样。采样后的数据
通过 FT2232H 同步 245 FIFO 模式传输到 PC。

FT2232H FIFO 模式支持双向数据传输，这里的只用到上传数据（即向 FIFO 写入数据）到 PC 单一路径，所以 `rd_n` 信号固定为 `1`。
FT2232H FIFO 写入时序如图。

![](./res/fifo_timing.png)

数据的写入控制涉及两个信号：

* `wr_n`：FT2232H 的输入，低电平有效，表示 DATA 上数据就绪；
* `tx_e`：FT2232H 的输出，低电平有效，表示 FIFO 此时可接收数据。

在 CLKOUT 上升沿 `wr_n`、`txe_n` 同时低电平说明数据已经写入 FIFO。

数据采样、FIFO 写入（简化）实现如下：

```verilog
always @(posedge clk) begin
    if (wr_n == 'b0) begin
        if (txe_n == 'b0) begin
            wr_n <= 'b1;
        end;
    end else begin
        if (counter == 0) begin
            reg_data <= capture;
        end
        if (counter == 1) begin
            wr_n <= 'b0;
        end
    end;
    counter <= counter + 1'b1;
end
```

## 连线说明

20K 与 FT2232H 的连接如下表。

| Primer 20K |   FT2232H Pin No.   |  说明    |
| --------   |  ---------------    | ------- |
|  K15       |   16                | ADBUS[0] |
|  K16       |   17                | ADBUS[1] |
|  H16       |   18                | ADBUS[2] |
|  G16       |   19                | ADBUS[3] |
|  P11       |   21                | ADBUS[4] |
|  R11       |   22                | ADBUS[5] |
|  L13       |   23                | ADBUS[6] |
|  K12       |   24                | ADBUS[7] |
|  H13       |   26                | RXF#     |
|  H12       |   27                | TXE#     |
|  T12       |   28                | RD#      |
|  K11       |   29                | WR#      |
|  K13       |   30                | SIWU     |
|  J12       |   32                | CLKOUT (60MHz)|
|  G11       |   33                | OE#      |

8 路探针如下表。

| Primer 20K |   探针 No.   |
| --------   |  -------    |
|  E15       |   0         |
|  F14       |   1         |
|  G14       |   2         |
|  J16       |   3         |
|  F13       |   4         |
|  M15       |   5         |
|  R13       |   6         |
|  R12       |   7         |

调试信号：N7 反映复位状态（复位信号来自 T2，下降沿生效）。

## 使用说明

完成综合、步线之后，直接下载到 SRAM。连接 FT2232H 开发板，打开桌面测试程序，将其中一路探针触碰 3.3V 高电平，
会看到打印出的变化的数据。

![](./res/result.png)

采集到的数据会自动保存到文件，可以用 [PulseView](https://sigrok.org/wiki/PulseView) 查看、分析。这里把探针 6 接到串口上，用串口工具发送了一个字符串“hello 20K!”，PulseView 成功还原：

![](./res/hello_20k.png)

## 注意事项：

1. 下载时，务必断开 FT2232H 开发版以及其它使用了 FTDI 驱动的设备；
2. 务必保证 FPGA 的供电充足，否则可能出现下载失败等情况；
3. 如果还是出现下载失败，重新拔插下载器；
4. 如果发现 FPGA 电路运行状态与预想的不符，建议**重新打开**下载器、重新下载。

## 其它

这个项目代码短小，也算是 [SIPEED Tang Primer 20K](https://en.wiki.sipeed.com/hardware/zh/tang/tang-primer-20k/primer-20k.html)
上的 Hello World。将本项目也可移植到
[SIPEED Tang Nano](https://en.wiki.sipeed.com/hardware/zh/tang/Tang-Nano-Doc/SUMMARY.html)
系列，用以辅助 20K 上复杂项目的开发。
