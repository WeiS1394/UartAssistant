#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <win_qextserialport.h>
#include <QtGui>
#include <QProgressBar>

class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Win_QextSerialPort *myCom;//声明串口对象
    QProgressBar *progressBar;  //进度条

    bool g_isHexSend;  //是否十六进制发送
    bool g_isHexDisplay; //是否十六进制显示
    bool DisplayFlag;  //是否停止显示
    bool SaveAs();  //另存为
    QTimer *myTime;  //QTimer类型存放时间
    /* 状态栏 */
    QLabel *statusLabel;  //状态栏标签
    QLabel *receiveLabel;  //接收
    QLabel *sendLabel;  //发送
    QPushButton *clearCounterButton;  //清零
    QLabel *timeLabel;  //时间
    quint64 mReceiveNum;
    quint64 mSendNum;

private slots:
    void on_openMyComBtn_clicked();  //打开串口 按钮
    void readMyCom();  //读取串口数据
    void on_sendMsgBtn_clicked();  //发送 按钮
    void WorkSendMsg();  //发送数据
    void SlectAutoSend();  //定时发送

    /* 状态栏 */
    void updateStateBar(QString state, QVariant inNum, QVariant outNum);
    void clearCounterPushBtn_released();

    void on_HEXDisplay_toggled(bool checked);  //十六进制显示
    void on_HEXSend_toggled(bool checked);  //十六进制发送
    void on_AutoSend_toggled(bool checked);  //定时发送 按钮
    void on_OpenFileBtn_clicked();  //打开文件
    void on_SendFileBtn_clicked();  //发送文件
    void on_ReceiveStopBtn_clicked();  //停止 按钮
    void on_ReceiveSaveBtn_clicked();  //保存窗口
    void on_ReceiveClearBtn_clicked();  //清除窗口
    void on_HelpBtn_clicked();  //帮助信息
    void on_DownControlBtn_clicked(); //缩小Control区域
    void on_ExpandBtn_clicked();  //扩展
    void on_textBrowser_textChanged();
};

#endif // MAINWINDOW_H
