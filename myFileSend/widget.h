#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QHostAddress>
#include <QDebug>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QMessageBox>
#include <QByteArray>
#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>

namespace Ui {
class Widget;
}
enum{
    SENDFILE = 0X02, //发送文件
    SENDTEXT = 0X03 //发送文本
};
typedef struct DataPackage_t{
    char head;  //包头
    char type;  //发送类型：文本/文件
    int  len;   //文本/文件 长度
    int  filenamelen; //文件名长度
    char tail;  //包尾
}DATAPA;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    //初始化一些参数
    void initPara();
    //获取当前所有IP
    QString getLocalAllAddress();
private slots:
    void on_btnConnect_clicked();
    //是否作为服务器
    void on_checkBoxisServer_stateChanged(int arg1);
    //关闭连接,不是关闭服务器。
    void on_btnClose_clicked();

    //作为服务器 处理新连接
    void checkNewConnection();
    //处理readyread信号
    void checkReadyread();
    //处理disconnected信号
    void checkClientDisconnected();
    //发送文本
    void on_btnSend_clicked();
    //开始服务器
    void on_pushButtonStart_clicked();

    void on_btnOpen_clicked();
    //选中文件发送 固定最多显示10个文件
    void on_listWidget_doubleClicked(const QModelIndex &index);

private:
    Ui::Widget *ui;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QString ipstr;
    qint16 port;
    bool isServer; //是否作为服务器
    bool isStart; //正在启动
    int  fileCount; //待发送列表
    bool isSendFile;    //是否在发送文件
    QFile writeFile;   //写文件
    int readSizeSum;  //接受的数据
    int writesizeSum;  //写入的数据
    int readFileLength;  //文件长度
};

#endif // WIDGET_H
