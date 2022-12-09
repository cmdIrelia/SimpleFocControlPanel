#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QPlainTextEdit>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_openSerialPort_clicked();

    void on_pushButton_position_clicked();

    void on_pushButton_velocity_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_cmdline_returnPressed();

    void on_serialport_data_ready();

    QString ParseMonitorValue(QString str);

private:
    Ui::MainWindow *ui;

    QSerialPort *serialPort1;

    void LogWnd_WriteLine(QPlainTextEdit *_logWnd, QString line);
    void LogWnd_WriteLine(QString line);
    QPlainTextEdit *logWnd;
    QPlainTextEdit *recvWnd;

    QByteArray recvBuff;
};
#endif // MAINWINDOW_H
