#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    initPara();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initPara()
{
    // ipstr = getLocalAllAddress();
     port = 10086;
  //  qDebug() << ipstr;
 //    ui->lineEditOwnPort->setText("10086");
 //    ui->lineEditOwnIP->setText("127.0.0.1");
  //   ui->lineEditOtherPort->setText("10086");
  //   ui->lineEditOtherIP->setText("127.0.0.1");
  //ui->lineEditOwnPort->setText(QString("%1").arg(port));

     isServer = false;  //默认作为客户端
     isStart = false;   //正在执行
     fileCount = 0;   //文件数量
     ui->checkBoxisServer->setChecked(false);
     ui->pushButtonStart->setEnabled(false);
     isSendFile = false;
     readSizeSum = 0;
     writesizeSum = 0;
     tcpServer = NULL;
     tcpSocket = NULL;
     ui->progressBarSend->setValue(0);
     ui->progressBarSend->setMaximum(100);
     ui->progressBarSend->setMinimum(0);
}

QString Widget::getLocalAllAddress()
{
    QList <QHostAddress> list = QNetworkInterface::allAddresses();
    qDebug() << list;
        foreach(QHostAddress address,list)
        {
            if(address.protocol() == QAbstractSocket::IPv4Protocol)
            {
                //我们使用IPv4地址
                return address.toString();。
            }
        }
        return "0.0.0.0";//不加这个会警告。。。。
 }

void Widget::on_btnConnect_clicked()
{
     isStart = true;
     ui->btnConnect->setEnabled(false);
     QHostAddress ipother(ui->lineEditOtherIP->text());
     qint16 portother= ui->lineEditOtherPort->text().toShort();
     tcpSocket = new QTcpSocket(this);
     tcpSocket->connectToHost(ipother,portother);

     connect(tcpSocket,&QTcpSocket::readyRead,this,&Widget::checkReadyread);
     connect(tcpSocket,&QTcpSocket::disconnected,this,&Widget::checkClientDisconnected );
}

void Widget::on_checkBoxisServer_stateChanged(int arg1)
{
   //  qDebug() << "----";
    if(isStart == true)
    {
        if(isServer)
        {
            ui->checkBoxisServer->setChecked(true);
        }else{

             ui->checkBoxisServer->setChecked(false);
        }
        return ;
    }
   if(2 == arg1)//作为服务器
   {
       if(tcpServer!=NULL)
       {
           return;
       }
       isServer = true;
       ui->btnConnect->setEnabled(false);  
       ui->pushButtonStart->setEnabled(true);
   }else if(0 == arg1)
   {
   //    qDebug() << "0";
       isServer = false;
       ui->btnConnect->setEnabled(true);   
       ui->pushButtonStart->setEnabled(false);
   }
}

void Widget::on_btnClose_clicked()
{

    if(isServer)
    {
         isStart = false;
        if( tcpSocket == NULL)
        {
            return;
        }
        tcpSocket->disconnectFromHost();
    //    tcpSocket->close();
    //    tcpSocket = NULL;
      //  ui->checkBoxisServer->setCheckable(true);

    }else{ //作为客户端
        if(tcpSocket == NULL)
        {
            return;
        }
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
         ui->btnConnect->setEnabled(true);
        delete tcpSocket;
        tcpSocket = NULL;
         isStart = false;
    }
}

void Widget::checkNewConnection()
{
    qDebug()<< "new connect";
    tcpSocket = tcpServer->nextPendingConnection();
    QHostAddress ipclient =  tcpSocket->peerAddress();
    qint16 portClient = tcpSocket->peerPort();
    QMessageBox::question(this,"提示",QString("新的连接:%1:%2").arg(ipclient.toString()).arg(portClient),QMessageBox::Ok);
    connect(tcpSocket,&QTcpSocket::readyRead,this, &Widget::checkReadyread);
    connect(tcpSocket,&QTcpSocket::disconnected,this, &Widget::checkClientDisconnected);
}

void Widget::checkReadyread()
{

      //  QByteArray arr = tcpSocket->readAll();
      //  ui->textEdit->setText(QString(arr));
    if(isSendFile == false)
    {
        DATAPA recvPackageFirt;
        memset(&recvPackageFirt,0,sizeof(recvPackageFirt));
        int ret = tcpSocket->read((char *)&recvPackageFirt,sizeof(recvPackageFirt));

        qDebug() << "headret:"<<ret;
        if(recvPackageFirt.head == 0x06 && recvPackageFirt.tail == 0x10)
        {
            if(recvPackageFirt.type == SENDTEXT)
            {
                char readBuf[1024];
                QString retString;
                int readSum = 0;
                while(readSum < recvPackageFirt.len)
                {
                     memset(readBuf,0,sizeof(readBuf));
                     int ret =  tcpSocket->read(readBuf,sizeof(readBuf));
                     readSum+=ret;
                     retString+=QString(readBuf);
                }
                ui->textEdit->setPlainText(retString);
                return;
            }else if(recvPackageFirt.type == SENDFILE)
            {
                isSendFile = true;
               int filenamelen = recvPackageFirt.filenamelen;
               char fileName[1024] ={0};
               int fileret =  tcpSocket->read(fileName,filenamelen);
               qDebug() << "fileret:"<<fileret;
               qDebug() <<"filename:"<< fileName;
               QString filepath("../");
                QString filename = filepath+QString(fileName);

                //获取接受的文件长度
                readFileLength = recvPackageFirt.len;
                qDebug() << "readFileLength:"<<readFileLength;
                writeFile.setFileName(filename);
                if (!writeFile.open(QIODevice::WriteOnly))
                    return;
                qDebug() << "open success";
                QMessageBox::question(this,"警告","正准备接收文件...",QMessageBox::Ok);
            }
        }
    }


    char readBuf[1024];
    while(readSizeSum < readFileLength)
    {
        memset(readBuf,0,sizeof(readBuf));
        int ret = tcpSocket->read(readBuf,sizeof(readBuf));
        if(ret == 0)
        {
            break;
        }
        qDebug() << "readsize:"<<ret;
        if(ret == -1)
        {
            QMessageBox::question(this,"error","read error",QMessageBox::Ok);
            writeFile.close();
            return ;
        }
        readSizeSum += ret;
        qDebug() << "readSizeSum:"<<readSizeSum ;
        int retwrite =  writeFile.write(readBuf,ret);
        qDebug() << "writesize:"<<retwrite;
          writesizeSum += retwrite;
        qDebug() <<"writesizeSum:"<< writesizeSum;
        qDebug() << "write success:";
        double num = readSizeSum/readFileLength;
        ui->progressBarSend->setValue( num*100);
    }

    if(readSizeSum <readFileLength )
    {
        return;
    }
    isSendFile = false;
    QMessageBox::question(this,"提示","文件接收成功",QMessageBox::Ok);

     qDebug() << "=====writt end====";
    writeFile.close();
    readSizeSum = 0;
    writesizeSum = 0;
    readFileLength = 0;

}

void Widget::checkClientDisconnected()
{
    if(isServer)
    {
        if(tcpSocket == NULL)
        {
            return;
        }
        tcpSocket->close();
        tcpSocket = NULL;
        isStart = false;
        QMessageBox::question(this,"提示","断开成功",QMessageBox::Ok);
    }else{
       ui->btnConnect->setEnabled(true);
       isStart = false;
       QMessageBox::question(this,"提示","断开成功",QMessageBox::Ok);
    }
}

void Widget::on_btnSend_clicked()
{
    // qDebug() << ui->textEdit->toPlainText().toUtf8().length();

    if(isServer)
    {
         if(tcpServer == NULL || tcpSocket == NULL)
         {
             return;
         }
      //  tcpSocket->write(ui->textEdit->toPlainText().toUtf8());
         DATAPA packagefirst;
         memset(&packagefirst,0,sizeof(packagefirst));
         packagefirst.head = 0x06;
         packagefirst.type = SENDTEXT;
         packagefirst.tail = 0x10;
         qDebug() << ui->textEdit->toPlainText().toUtf8().length();
         packagefirst.len = ui->textEdit->toPlainText().toUtf8().length();
         tcpSocket->write((char *)&packagefirst,sizeof(packagefirst));
         tcpSocket->write(ui->textEdit->toPlainText().toUtf8().data(),packagefirst.len);
    }else{
        if(tcpSocket == NULL)
        {
            return;
        }
        DATAPA packagefirst;
        memset(&packagefirst,0,sizeof(packagefirst));
        qDebug() << sizeof(DATAPA);
        packagefirst.head = 0x06;
        packagefirst.type = SENDTEXT;
        packagefirst.tail = 0x10;
        qDebug() << ui->textEdit->toPlainText().toUtf8().length();
        packagefirst.len = ui->textEdit->toPlainText().toUtf8().length();
        tcpSocket->write((char *)&packagefirst,sizeof(packagefirst));
        tcpSocket->write(ui->textEdit->toPlainText().toUtf8().data(),packagefirst.len);

    }
}

//开始运行服务器主动监听， 而客户端则需要手动connect
void Widget::on_pushButtonStart_clicked()
{    
    if(isStart == true)
    {
       // QMessageBox::question(this,"warn","is starting",QMessageBox::Ok);
        int ret = QMessageBox::question(this,"警告","确认要关闭服务器吗?",QMessageBox::Ok,QMessageBox::Cancel);
        if(ret == QMessageBox::Ok)
        {
                on_btnClose_clicked();
                tcpServer->close();
                delete tcpServer;
                tcpServer = NULL;
                 ui->pushButtonStart->setText("开启服务器");
                 ui->btnConnect->setEnabled(true);
                return;
        }else{
            return;
        }
    }
    isStart = true;

    if(isServer)
    {    
        if(tcpServer == NULL)
        {
            tcpServer = new QTcpServer(this);
            ui->btnConnect->setEnabled(false);

            tcpServer->listen(QHostAddress::Any,ui->lineEditOwnPort->text().toShort());
            ui->pushButtonStart->setText("StopServer");
            connect(tcpServer,&QTcpServer::newConnection,this,&Widget::checkNewConnection );
        }
    }else{       
    }
}

void Widget::on_btnOpen_clicked()
{
     if(fileCount == 10)
     {
         return;
     }
    QString openFilePath = QFileDialog::getOpenFileName(this,tr("open file"),"./","TXT(*.txt);;Image(*.png *.jpg *.bmp);;C/C++(*.c *.cpp);;ALL(*.*)");
    if(openFilePath ==NULL )
    {
        return;
    }
    QListWidgetItem *newitem = new QListWidgetItem;
    newitem->setText(openFilePath);
    ui->listWidget->insertItem(fileCount++, newitem);
}

void Widget::on_listWidget_doubleClicked(const QModelIndex &index)
{
    QFile file(ui->listWidget->currentItem()->text());
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::question(this,"error","open file error",QMessageBox::Ok);
        qDebug() << "打开文件失败";
        return;
    }
    QFileInfo fileinfo(ui->listWidget->currentItem()->text());

    DATAPA packagefirst;
    memset(&packagefirst,0,sizeof(packagefirst));
    packagefirst.head = 0x06;
    packagefirst.type = SENDFILE;
    packagefirst.tail = 0x10;
    int fileLength = file.size();
    qDebug() <<"file size:"<< fileLength;
    packagefirst.len = fileLength;
    char fileNamebuf[1024] ={0};
    strcpy(fileNamebuf, fileinfo.fileName().toStdString().data());
  //const char fileNamebuf[1024] = fileinfo.fileName().toStdString().data();
   qDebug() << "filename:"<<fileNamebuf;
   packagefirst.filenamelen = strlen(fileNamebuf);

    //发送文件大小过去

   int ret = tcpSocket->write((char *)&packagefirst,sizeof(packagefirst));
   if(-1 == ret)
   {
       file.close();
       QMessageBox::question(this,"error","send file error",QMessageBox::Ok);
       return;
   }
    qDebug() << "headsize:"<<ret;

    //发送文件名

    int fileret = tcpSocket->write(fileNamebuf,strlen(fileNamebuf));
    qDebug() << "fileret:"<<fileret;

    //发送文件内容
   int sendFileSize = 0;
   int readFilesize = 0;
   char readFileBuf[1024]={0};
   while(sendFileSize < fileLength)
   {
       memset(readFileBuf,0,sizeof(readFileBuf));
       int ret = file.read(readFileBuf,sizeof(readFileBuf));
       qDebug() <<"read ret:"<<ret;
       readFilesize+=ret;
       qDebug() << "readFilesize:"<<readFilesize;
        int retsend = tcpSocket->write(readFileBuf,ret);
        qDebug() << "retsend:"<<retsend;
        sendFileSize+=retsend;
       qDebug() << "sendFilesize :" <<sendFileSize;
       double num = sendFileSize/fileLength;
       ui->progressBarSend->setValue(num*100);
   }

    QMessageBox::question(this,"提示","文件传输完成",QMessageBox::Ok);
    ui->listWidget->takeItem(index.row());
    qDebug() <<"send over";
    file.close();
}
