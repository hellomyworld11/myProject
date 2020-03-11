#include "dialog.h"
#include "ui_dialog.h"
#include <QStringList>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QTextCodec>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
  // QTextCodec * BianMa = QTextCodec::codecForName ( "GBK" );
    mIsOpen = false;
    ui->pushButtonSend->setEnabled(false);
    SetTheDefaultConfig();
    //检测当前有效的串口
    QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
    for(int i = 0 ; i< serialPortList.size();i++)
    {
       ui->comboBoxPort->addItem( serialPortList.at(i).portName());
    }
    connect(&mSerialPort,&QSerialPort::readyRead,this,&Dialog::checkRead);
}
Dialog::~Dialog()
{
    delete ui;
}

void Dialog::getCurrentConfig()
{

    currentPortName = ui->comboBoxPort->currentText();
    currentBaud = ui->comboBoxBaud->currentText();
    currentDatabit = ui->comboBoxData->currentText();
    currentParity = ui->comboBoxJiaoyan->currentText();
    currentStopbit = ui->comboBoxStop->currentText();
}

void Dialog::setSerialPortConfig()
{
    mSerialPort.setPortName(currentPortName);
    mSerialPort.setBaudRate(currentBaud.toInt()); //设置波特率
    mSerialPort.setDataBits(static_cast<QSerialPort::DataBits>(currentDatabit.toInt()));
    mSerialPort.setStopBits(static_cast<QSerialPort::StopBits>(currentStopbit.toInt()));
    if(currentParity == "ODD")
    {
        mSerialPort.setParity(QSerialPort::OddParity);

    }else if(currentParity == "EVEN")
    {
        mSerialPort.setParity(QSerialPort::EvenParity);
    }else if(currentParity == "NONE")
    {
        mSerialPort.setParity(QSerialPort::NoParity);
    }else if(currentParity =="SPACE")
    {
        mSerialPort.setParity(QSerialPort::SpaceParity);
    }else if(currentParity =="MARK")
    {
        mSerialPort.setParity(QSerialPort::MarkParity);
    }

}

void Dialog::SetTheDefaultConfig()
{
    QSettings *mySet = new QSettings("./setting.ini",QSettings::IniFormat);
   // mySet.setPath(QSettings::IniFormat,QSettings::UserScope,"./setting.ini");
    ui->comboBoxPort->setCurrentText(mySet->value("portname").toString());
    ui->comboBoxBaud->setCurrentText(mySet->value("baud").toString());
    ui->comboBoxJiaoyan->setCurrentText(mySet->value("parity").toString());
    ui->comboBoxStop->setCurrentText(mySet->value("stopbit").toString());
    ui->comboBoxData->setCurrentText(mySet->value("databit").toString());
    delete mySet;
}

void Dialog::saveTheDefaultConfig()
{
    QSettings *mySet = new QSettings("./setting.ini",QSettings::IniFormat);
 //   mySet.setPath(QSettings::IniFormat,QSettings::UserScope,"./setting.ini");
    mySet->setValue("portname",ui->comboBoxPort->currentText());
    mySet->setValue("baud",ui->comboBoxBaud->currentText());
    mySet->setValue("parity",ui->comboBoxJiaoyan->currentText());
    mySet->setValue("stopbit",ui->comboBoxStop->currentText());
    mySet->setValue("databit",ui->comboBoxData->currentText());
      delete mySet;
}

void Dialog::on_pushButtonOpen_clicked()
{
    saveTheDefaultConfig();
    if(mIsOpen)
    {

        mSerialPort.close();
        ui->pushButtonOpen->setText("打开");
        mIsOpen = false;
        ui->comboBoxBaud->setEnabled(true);
        ui->comboBoxData->setEnabled(true);
        ui->comboBoxJiaoyan->setEnabled(true);
        ui->comboBoxPort->setEnabled(true);
        ui->comboBoxStop->setEnabled(true);
         ui->pushButtonSend->setEnabled(false);
    }else{
        //打开串口
        getCurrentConfig();
        setSerialPortConfig();
       if ( mSerialPort.open(QIODevice::ReadWrite) )
       {
            mIsOpen = true;
            ui->pushButtonOpen->setText("关闭");
            ui->comboBoxBaud->setEnabled(false);
            ui->comboBoxData->setEnabled(false);
            ui->comboBoxJiaoyan->setEnabled(false);
            ui->comboBoxPort->setEnabled(false);
            ui->comboBoxStop->setEnabled(false);
            ui->pushButtonSend->setEnabled(true);
       }
    }
}

void Dialog::checkRead()
{
   if(ui->checkBoxisHexShow ->isChecked()) //16进制
   {
         QByteArray readArr =  mSerialPort.readAll(); //读出字节
         QString hex_data = readArr.toHex().data(); // 将buf里的数据转换为16进制
         hex_data = hex_data.toUpper(); // 转换为大写
         QString hex_str;
         // 将16进制 按两位 两位输出 比如01020304 - 》 01 02 03 04
         for (int i=0; i< hex_data.length(); i+=2)
         {
             QString st = hex_data.mid(i,2);
             hex_str+=st;
             hex_str+=' ';
         }
         ui->textEditRead->append(hex_str);
   }else{
       QByteArray readArr =  mSerialPort.readAll();
       QString str = QString(readArr);
       QString str2 = QString::fromLocal8Bit(readArr); //解码unicode
       ui->textEditRead->append(str2);
   }
}

void Dialog::on_pushButtonSend_clicked()
{
    if(ui->checkBoxisHexSend->isChecked())
    {
        //sendText转为16进制
        if(!ui->textEditWrite->toPlainText().isEmpty())
        {
            QString temp =ui->textEditWrite->toPlainText();
            if(temp.length()%2 !=0)
            {
               temp.insert(temp.length()-1,'0');
            }
   //         ui->textEditWrite->setText(QString(temp));
            QByteArray arr;
            StringToHex(temp,arr);
            mSerialPort.write(arr);
        }

    }else{

       //  mSerialPort.write(ui->textEditWrite->toPlainText().toLocal8Bit());
        //文本模式转unicode
         mSerialPort.write(ui->textEditWrite->toPlainText().toLocal8Bit());
    }

}

//12 34 56 78 ab -> 0x12 0x34 0x56 0x78 0xab 字符串到16进制的转换 在16进制模式发送下只接收字符和数字，不接受汉字
//具体步骤：
/*
1.字符串 -> 对应ascii码 ->对应十进制 - > 两个十进制合成一个十六进制  ->十六进制 ->用一个char保存一位16进制
-》字节发送
*/
void Dialog::StringToHex(QString str, QByteArray &senddata)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length(); //求长度:eg:10
    senddata.resize(len/2);  //5 一个字节能表示一个16进制数即字符串的两位，实际上两位字符是表示一个十六进制数
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1(); //转为ASCII
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);  //字符ascii转对应的整数形式。
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata >= 16) || (lowhexdata >= 16))  //16进制0-15  超过范围则发空
          break;
        else
            hexdata = hexdata*16+lowhexdata;  //两个合成一个16进制的10进制表达方式
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}
//字符转对应整数 ： ‘1’ -> 1  'a' ->10
char Dialog::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
            return ch-0x30;
        else if((ch >= 'A') && (ch <= 'F'))
            return ch-'A'+10;
        else if((ch >= 'a') && (ch <= 'f'))
            return ch-'a'+10;
        else return 16;
}

