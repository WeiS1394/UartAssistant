#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Qtcore/qtextcodec.h>
#include <QDebug>
#include <QMessageBox>
#include <myhelp.h>
#include "qextserialbase.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));  //编码对照，显示中文标题
    setWindowTitle(trUtf8("串口调试助手"));  //设置窗口标题

    ui->sendMsgBtn->setEnabled(false);  //初始化发送按钮不可用
    ui->AutoSend->setEnabled(false);  //初始化定时发送
    ui->SendFileBtn->setEnabled(false);  //初始化发送文件
    DisplayFlag = true;  //初始化接收数据状态
    g_isHexDisplay = false;  //初始化十六进制显示
    g_isHexSend = false;  //初始化十六进制发送

    //串口选择
    ui->portNameComboBox->addItem("COM1");
    ui->portNameComboBox->addItem("COM2");
    ui->portNameComboBox->addItem("COM3");
    ui->portNameComboBox->addItem("COM4");
    ui->portNameComboBox->addItem("COM5");
    ui->portNameComboBox->addItem("COM6");
    ui->portNameComboBox->addItem("COM7");
    ui->portNameComboBox->addItem("COM8");
    ui->portNameComboBox->addItem("COM9");

    myTime = new QTimer();  //  设置一个单发定时器
    myTime->setInterval(1000);  //  自动发送间隔初始化
    connect(myTime, SIGNAL(timeout()), this, SLOT(SlectAutoSend()));  //时间信号和定时发送关联，timeout的意思是当
                                                                                                                //定时器超时时，这个信号会发出
    //进度条
    progressBar = new QProgressBar;
    ui->gridLayout->addWidget(progressBar, 1, 0);
    progressBar->setVisible(false);  //隐藏

    /* 状态栏标签 */
    statusLabel = new QLabel;
    statusLabel->setMinimumSize(280, 20);  //设置标签最小的大小
    statusLabel->setFrameShape(QFrame::WinPanel);  //设置标签形状
    statusLabel->setFrameShadow(QFrame::Sunken);  //设置标签阴影
    ui->statusBar->addWidget(statusLabel);  //在状态栏中添加状态栏标签
    statusLabel->setText(trUtf8("串口停止"));
    statusLabel->setAlignment(Qt::AlignHCenter);

    /* 接收数量标签 */
    receiveLabel = new QLabel;
    receiveLabel->setMinimumSize(100, 20);
    receiveLabel->setFrameShape(QFrame::WinPanel);
    receiveLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(receiveLabel);
    receiveLabel->setAlignment(Qt::AlignHCenter);

    /* 发送数量标签 */
    sendLabel = new QLabel;
    sendLabel->setMinimumSize(100, 20);
    sendLabel->setFrameShape(QFrame::WinPanel);
    sendLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(sendLabel);
    sendLabel->setAlignment(Qt::AlignHCenter);
    updateStateBar(QString(), 0, 0);  //初始化状态栏

    /* 计数器清零 */
    clearCounterButton = new QPushButton;
    clearCounterButton->setMinimumSize(50, 20);
    ui->statusBar->addWidget(clearCounterButton);
    clearCounterButton->setText(trUtf8("清零"));
    connect(clearCounterButton, SIGNAL(released()), this, SLOT(clearCounterPushBtn_released()));

    /* 时间标签 */
    timeLabel = new QLabel;
    timeLabel->setMinimumSize(90, 20);
    timeLabel->setMaximumWidth(90);
    timeLabel->setFrameShape(QFrame::WinPanel);
    timeLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(timeLabel);
    timeLabel->setText(QDate::currentDate().toString("yyyy-MM-dd"));  //更新时间

    mReceiveNum = mSendNum = 0;
    qDebug() << QDir::currentPath();  //调试，打印程序的执行路径
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * 打开串口
 */
void MainWindow::on_openMyComBtn_clicked()
{
    if (ui->openMyComBtn->text() == trUtf8("打开串口")) {
        QString portName = ui->portNameComboBox->currentText();  //获取串口号
        myCom = new Win_QextSerialPort(portName, QextSerialBase::EventDriven);  //打开串口对象，传递参数，在构造函数里对其初始化
        if (false == myCom->open(QIODevice::ReadWrite)) {
            QMessageBox::warning(this, tr("Warning"), tr("串口不存在或已被占用"), QMessageBox::Yes);
            return;
        }

        //波特率
        if (ui->baudRateComboBox->currentIndex() == 0) {
            myCom->setBaudRate(BAUD9600);
        }
        else if (ui->baudRateComboBox->currentIndex() == 1) {
            myCom->setBaudRate(BAUD115200);
        }
        //数据位
        if (ui->dataBitsComboBox->currentText() == "8") {
            myCom->setDataBits(DATA_8);
        }
        else if (ui->dataBitsComboBox->currentText() == "7") {
            myCom->setDataBits(DATA_7);
        }
        //奇偶校验位
        if (ui->parityComboBox->currentText() == "None") {
            myCom->setParity(PAR_NONE);
        }
        else if (ui->parityComboBox->currentText() == "Odd") {
            myCom->setParity(PAR_ODD);
        }
        else if (ui->parityComboBox->currentText() == "Even") {
            myCom->setParity(PAR_EVEN);
        }
        //停止位
        if (ui->stopBitsComboBox->currentText() == "1") {
            myCom->setStopBits(STOP_1);
        }
        else if (ui->stopBitsComboBox->currentText() == "2") {
            myCom->setStopBits(STOP_2);
        }
        //数据流
        if (ui->comboBox->currentText() == "Hardware") {
            myCom->setFlowControl(FLOW_HARDWARE);
        }
        else if (ui->comboBox->currentText() == "Software") {
            myCom->setFlowControl(FLOW_XONXOFF);
        }
        else if (ui->comboBox->currentText() == "None") {
            myCom->setFlowControl(FLOW_OFF);
        }

        connect(myCom, SIGNAL(readyRead()), this, SLOT(readMyCom()));  //关联，串口缓冲区有信号时，读取串口

        updateStateBar(ui->portNameComboBox->currentText() + trUtf8(" 已开启") +
                       ui->baudRateComboBox->currentText() + "bps," +
                       ui->dataBitsComboBox->currentText() + "," +
                       ui->stopBitsComboBox->currentText() + "," +
                       ui->parityComboBox->currentText() + "," +
                       ui->comboBox->currentText(),
                       QVariant(QVariant::Int), QVariant(QVariant::Int));

        ui->openMyComBtn->setText(trUtf8("关闭串口"));
        ui->sendMsgBtn->setEnabled(true);
        ui->portNameComboBox->setEnabled(false);
        ui->baudRateComboBox->setEnabled(false);
        ui->dataBitsComboBox->setEnabled(false);
        ui->parityComboBox->setEnabled(false);
        ui->stopBitsComboBox->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->AutoSend->setEnabled(true);
        ui->SendFileBtn->setEnabled(true);
    } else {
        myCom->close();
        ui->openMyComBtn->setText(trUtf8("打开串口"));
        updateStateBar(trUtf8("串口停止"), QVariant(QVariant::Int), QVariant(QVariant::Int));
        ui->sendMsgBtn->setEnabled(false);
        ui->portNameComboBox->setEnabled(true);
        ui->baudRateComboBox->setEnabled(true);
        ui->dataBitsComboBox->setEnabled(true);
        ui->parityComboBox->setEnabled(true);
        ui->stopBitsComboBox->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->AutoSend->setEnabled(false);
        ui->SendFileBtn->setEnabled(false);
    }
}

/*
 * 读取串口，打印数据
 */
void MainWindow::readMyCom()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray temp = myCom->readAll();  //读取串口缓冲区的数据
    if (temp.length() <= 0)
        return;
    QString strDisplay;

    if (g_isHexDisplay) {  //是否十六进制显示
        QString str = temp.toHex().data();
        str = str.toUpper();
        for ( int i = 0; i < str.length(); i += 2) {  //以空格方式打印
            QString st = str.mid(i, 2);
            strDisplay += st;
            strDisplay += " ";
        }
    }
    else {
        strDisplay = QString(temp);
    }

    if (DisplayFlag) {  // 是否停止打印
        ui->textBrowser->insertPlainText(strDisplay);  //读取到的数据打印在窗口中
        updateStateBar(QString(), temp.size(), QVariant(QVariant::Int));
    }
}

/*
 * 发送按钮
 */
void MainWindow::on_sendMsgBtn_clicked()
{
    MainWindow::WorkSendMsg();  //发送数据函数
}

/*
 * 发送数据
 */
void MainWindow::WorkSendMsg()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    if (!(myCom->isOpen())) {
        return;
    }

    QString sdata = ui->sendMsgLineEdit->displayText();  //要发送的数据

    if (sdata.isEmpty())
        return;

    QByteArray sendBuf = "";

    if (g_isHexSend) {
        sendBuf = QByteArray::fromHex(sdata.toLatin1().data());
    } else {
        sendBuf = sdata.toAscii();
    }

    myCom->write(sendBuf);
    updateStateBar(QString(), QVariant(QVariant::Int), sendBuf.size());
}


/*
 * 十六进制显示, QCheckBox控件槽函数
 */
void MainWindow::on_HEXDisplay_toggled(bool checked)
{
    g_isHexDisplay = checked;
}

/*
 * 十六进制发送
 */
void MainWindow::on_HEXSend_toggled(bool checked)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));

    QString str = ui->sendMsgLineEdit->displayText();
    if (!str.isEmpty()) {
        if (checked) {
            ui->sendMsgLineEdit->setText(str.toAscii().toHex());
        }
        else {
            QByteArray buf = myHelper::StringtoHex(str);
            ui->sendMsgLineEdit->setText(buf);
        }
    }
    g_isHexSend = checked;
}

/*
 * 获取定时时间
 */
void MainWindow::on_AutoSend_toggled(bool checked)
{
    if (checked) {
        int time = ui->AutoSendTime->text().toInt();
        myTime->start(time);
        ui->sendMsgBtn->setEnabled(false);
        ui->SendFileBtn->setEnabled(false);
    }
    else {
        myTime->stop();
        ui->sendMsgBtn->setEnabled(true);
        ui->SendFileBtn->setEnabled(true);
    }
}

/*
 * 定时发送
 */
void MainWindow::SlectAutoSend()
{
    MainWindow::WorkSendMsg();
}

/*
 * 打开文件
 */
void MainWindow::on_OpenFileBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开"), "", tr("所有文件(*.*)"));
    QFile file(fileName);
    QString fileDisplay;

    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        ui->textBrowser->insertPlainText(trUtf8("打开文件的大小：%1字节，下面显示全部内容").arg(file.size()));
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            if (g_isHexDisplay) {
                QString str = textStream.readAll().toAscii().toHex();
                str = str.toUpper();
                for (int i  = 0; i < str.length(); i += 2) {
                    QString st = str.mid(i, 2);
                    fileDisplay += st;
                    fileDisplay += " ";
                }
            }
            else {
                fileDisplay = textStream.readLine();
            }
            ui->textBrowser->append(fileDisplay);
        }
        ui->FileLineEdit->setText(fileName);
        ui->textBrowser->append(trUtf8("\n文件打开结束\n"));
    }
    else {
        QMessageBox::information(NULL, NULL, "open file error!");
    }
}

/*
 * 发送文件
 */
void MainWindow::on_SendFileBtn_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QFile file(ui->FileLineEdit->text());
    int num;
    ui->textBrowser->clear();
    ui->textBrowser->insertPlainText(trUtf8("文件大小：%1字节，请稍后...\n\n").arg(file.size()));

    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QTextStream textSend(&file);
        QString sendString = textSend.readAll();
        num = sendString.length();
        QByteArray buf = sendString.toAscii();
        myCom->write(buf);
        updateStateBar(QString(), QVariant(QVariant::Int), buf.size());
    }
    progressBar->setVisible(true);  //显示进度条
    progressBar->setRange(0, num);
    for (int i = 1; i < num; i++) {
        progressBar->setValue(i);
    }
    progressBar->setVisible(false);

}

/*
 * 停止接收数据
 */
void MainWindow::on_ReceiveStopBtn_clicked()
{
    if (ui->ReceiveStopBtn->text() == tr("停止")) {
        ui->ReceiveStopBtn->setText(trUtf8("继续"));
        DisplayFlag = false;
    } else {
        ui->ReceiveStopBtn->setText(trUtf8("停止"));
        DisplayFlag = true;
    }
}

/*
 * 保存窗口内容到文件中
 */
void MainWindow::on_ReceiveSaveBtn_clicked()
{
    SaveAs();
}

/*
 *  保存数据
 */
bool MainWindow::SaveAs()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GNK"));
    QString curFile;
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), curFile, tr("TXT文档(*.txt);;所有文件(*.*)"));

    if (fileName.isEmpty()) {
        return false;
    }

    if (!(fileName.endsWith(".txt", Qt::CaseInsensitive))) {
        fileName += ".txt";
    }

    QFile file(fileName);

    if(!file.open(QFile::WriteOnly | QFile::Unbuffered)) {
        QMessageBox::critical(this, tr("critical"), tr("无法写入数据！"), QMessageBox::Yes);
        return false;
    } else {
        QTextStream out(&file);
        out <<ui->textBrowser->toPlainText();
        return true;
    }
}

/*
 * 清除窗口 按钮
 */
void MainWindow::on_ReceiveClearBtn_clicked()
{
    ui->textBrowser->clear();
}

/*
 * 帮助 按钮
 */
void MainWindow::on_HelpBtn_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GNK"));
    QFile file("1.txt");  //打开文件
    QString fileDisplay;
    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            fileDisplay = textStream.readLine();
            ui->textBrowser->append(fileDisplay);
        }
    }
    else {
        QMessageBox::information(NULL, NULL, "open help error");
    }
}

/*
 * 缩小Control区域
 */
void MainWindow::on_DownControlBtn_clicked()
{
    if (ui->DownControlBtn->text() == tr("-")) {
        ui->groupBox_3->setVisible(false);  //隐藏控制区
        ui->DownControlBtn->setText(tr("■"));
    }
    else {
        ui->groupBox_3->setVisible(true);  //显示控制区
        ui->DownControlBtn->setText(tr("-"));
    }
}

/*
 * 扩展 按钮
 */
void MainWindow::on_ExpandBtn_clicked()
{
    QMessageBox::information(NULL, NULL, "open Expand error");
}

/*
 * 光标显示在最后
 */
void MainWindow::on_textBrowser_textChanged()
{
    ui->textBrowser->moveCursor(QTextCursor::End);
}

/*
 * 状态栏更新
 */
void MainWindow::updateStateBar(QString state, QVariant inNum, QVariant outNum)
{
    if (!state.isNull()) {
        statusLabel->setText(state);
    }
    if (!inNum.isNull()) {
        if (inNum.toInt() == 0) {
            mReceiveNum = 0;
        }
        else {
            mReceiveNum += inNum.toInt();
        }
        receiveLabel->setText(trUtf8("接收：") + QString::number(mReceiveNum));
    }

    if (!outNum.isNull()) {
        if (outNum.toInt() == 0) {
            mSendNum = 0;
        }
        else {
            mSendNum += outNum.toInt();
        }
        sendLabel->setText(trUtf8("发送：") + QString::number(mSendNum));
    }
}


/*
 * 状态栏清零
 */
void MainWindow::clearCounterPushBtn_released()
{
    updateStateBar(QString(), 0, 0);
}
