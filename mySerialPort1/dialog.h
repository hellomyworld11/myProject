#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QSettings>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void getCurrentConfig();
    void setSerialPortConfig();
    void SetTheDefaultConfig();
    void saveTheDefaultConfig();
    char ConvertHexChar(char ch);
    void StringToHex(QString str, QByteArray &senddata);
private slots:
    void on_pushButtonOpen_clicked();
    void checkRead();

    void on_pushButtonSend_clicked();

private:
    Ui::Dialog *ui;
    QString currentPortName;  //当前串口
    QString currentBaud;
    QString currentStopbit;
    QString currentDatabit;
    QString currentParity;

    QSerialPort mSerialPort;
    bool mIsOpen;
 //   QSettings mySet;

};

#endif // DIALOG_H
