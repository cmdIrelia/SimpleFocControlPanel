#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/favicon.ico"));

    serialPort1 = new QSerialPort();
    connect(serialPort1,SIGNAL(readyRead()),this, SLOT(on_serialport_data_ready()));

    //Log窗口只读
    ui->plainTextEdit_logWnd->setReadOnly(true);
    this->logWnd = ui->plainTextEdit_logWnd;
    this->recvWnd = ui->plainTextEdit_recvWnd;

    //遍历串口
    foreach( const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        LogWnd_WriteLine(QString("serial port system loction: %1").arg(info.systemLocation()));
        LogWnd_WriteLine(QString("serial port name: %1").arg(info.portName()));
        ui->lineEdit_serialPortName->setText(info.portName());
#ifdef _WIN32
        ui->lineEdit_serialPortName->setText(info.portName());
#else
        ui->lineEdit_serialPortName->setText(info.systemLocation());
#endif
    }

    LogWnd_WriteLine("Sys Start.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LogWnd_WriteLine(QPlainTextEdit *_logWnd, QString line)
{
    //移动光标到最后一行
    _logWnd->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor);
    //文本数量过大就清空
    if(_logWnd->toPlainText().size()>1025*8)
    {
        _logWnd->clear();
    }
    //获取时间
    QDateTime curDateTime=QDateTime::currentDateTime();
    QString time = curDateTime.toString ("hh:mm:ss");
    //插入新的一行文本
    _logWnd->insertPlainText("["+time+"] "+line+'\n');
    //滚动条滚动到底部
    QScrollBar *scrollbar = _logWnd->verticalScrollBar();
    if(scrollbar)
    {
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void MainWindow::LogWnd_WriteLine(QString line)
{
    //移动光标到最后一行
    this->logWnd->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor);
    //文本数量过大就清空
    if(this->logWnd->toPlainText().size()>1025*8)
    {
        this->logWnd->clear();
    }
    //获取时间
    QDateTime curDateTime=QDateTime::currentDateTime();
    QString time = curDateTime.toString ("hh:mm:ss");
    //插入新的一行文本
    this->logWnd->insertPlainText("["+time+"] "+line+'\n');
    //滚动条滚动到底部
    QScrollBar *scrollbar = this->logWnd->verticalScrollBar();
    if(scrollbar)
    {
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void MainWindow::on_pushButton_openSerialPort_clicked()
{
    if(serialPort1->isOpen())
    {
        serialPort1->close();
        LogWnd_WriteLine("Serial Port Closed.");
        return;
    }
    serialPort1->setPortName(ui->lineEdit_serialPortName->text());
    if(!serialPort1->open(QIODevice::ReadWrite))
    {
        LogWnd_WriteLine("Failed to Serial Port: "+QString(ui->lineEdit_serialPortName->text()));
        return;
    }
    serialPort1->setBaudRate(115200);
    serialPort1->setParity(QSerialPort::NoParity);
    serialPort1->setStopBits(QSerialPort::OneStop);
    serialPort1->setFlowControl(QSerialPort::NoFlowControl);
    serialPort1->setDataBits(QSerialPort::Data8);
    LogWnd_WriteLine("Serial Port start OK.");
}

void MainWindow::on_pushButton_position_clicked()
{
    if(serialPort1->isOpen())
    {
        QByteArray qbaSerialPortData("MC2\n");
        serialPort1->write(qbaSerialPortData,qbaSerialPortData.size());
        serialPort1->flush();
        LogWnd_WriteLine("Position Mode Set, MC2.");
    }
    else
    {
        LogWnd_WriteLine("SerialPort not opened yet.");
    }
}

void MainWindow::on_pushButton_velocity_clicked()
{
    if(serialPort1->isOpen())
    {
        QByteArray qbaSerialPortData("MC1\n");
        serialPort1->write(qbaSerialPortData,qbaSerialPortData.size());
        serialPort1->flush();
        LogWnd_WriteLine("Velocity Mode Set, MC1.");
    }
    else
    {
        LogWnd_WriteLine("SerialPort not opened yet.");
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    QString cmd;
    cmd = ui->lineEdit_cmdPrefix->text() + QString::number(value/10.f) + '\n';  //value在UI上被放大了10倍
    if(serialPort1->isOpen() && ui->checkBox_slidBarEnable->isChecked())    //判断串口打开，以及有效性选中
    {
        QByteArray qbaSerialPortData(cmd.toUtf8());
        serialPort1->write(qbaSerialPortData,qbaSerialPortData.size());
        serialPort1->flush();
        LogWnd_WriteLine(cmd.left(cmd.length()-1)); //减一去掉末尾的换行符
    }
}

void MainWindow::on_lineEdit_cmdline_returnPressed()
{
    QString cmd;
    cmd = ui->lineEdit_cmdline->text() + '\n';
    if(serialPort1->isOpen())
    {
        QByteArray qbaSerialPortData(cmd.toUtf8());
        serialPort1->write(qbaSerialPortData,qbaSerialPortData.size());
        serialPort1->flush();
        LogWnd_WriteLine(cmd.left(cmd.length()-1)); //减一去掉末尾的换行符
    }
}

void MainWindow::on_serialport_data_ready()
{
    QByteArray buf;
    buf = serialPort1->readAll();
    if(!buf.isEmpty())
    {
        for(int i = 0; i< buf.size(); i++)
        {
            if(buf.at(i)!='\r') //以换行符为分隔
            {
                recvBuff.append(buf.at(i));
            }
            else
            {
                QString s = ParseMonitorValue(recvBuff);
                LogWnd_WriteLine(recvWnd,s.remove(0,1)); //不将接收到的'\n'放入buf中
                recvBuff.clear();
            }
        }
    }
}

QString MainWindow::ParseMonitorValue(QString str)
{
    QStringList sl = str.split('\t');
    if(sl.length()==3)
    {
        float speed = sl[1].toFloat()/3.1415926*180.f;
        float angle = sl[2].toFloat()/3.1415926*180.f;
        str += QString("\t%1\t%2").arg(QString::number(speed)).arg(QString::number(angle));
    }
    return str;
}
