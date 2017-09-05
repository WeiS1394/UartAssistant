串口调试助手UartAssistant_v0版本说明：
课题任务编写，Windows7+Qt4.6环境下编写
功能：
	1.串口1~9的选择，波特率，数据位，停止位，校验位，数据流控的设置。
	2.十六进制发送和显示。
	3.状态栏显示发送和接收数量的计数。
	4.打开文件和发送文件。
	5.保存内容到文件。
	
说明：有6个Windows下串口通信的API函数库（qextserialbase.cpp和qextserialbase.h，qextserialport.cpp和qextserialport.h，win_qextserialport.cpp和win_qextserialport.h），其它的为界面编写函数。只有简单的串口的打开，关闭，发送数据功能。