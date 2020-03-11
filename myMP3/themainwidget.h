#ifndef THEMAINWIDGET_H
#define THEMAINWIDGET_H
#include <QWidget>
#include <QFrame>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMediaPlaylist>
#include <QTime>
#include <QAbstractSlider>
#include <QMovie>
#include <QFileInfo>
#include <QStringList>
#include <QThread>
#include <QDialog>
#include <QPushButton>
#include <QMap>
#include <QTextCodec>
#include <QFile>
#include <QStringList>
#include <QHBoxLayout>
#include <QAbstractSlider>
#include <QPalette>
#include "mythreadlyricshow.h"
#define cout qDebug()<<"["<<__LINE__<<"]"
namespace Ui {
class TheMainWidget;
}

class TheMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TheMainWidget(QWidget *parent = 0);
    ~TheMainWidget();
    void initPort();
    //设置时间显示
    void settime(qint64 playtime);
private slots:
    //更新滑块进度
    void updatePosition(qint64 position);

    void slider_vilume_changed();//音量控制
    //列表双击播放歌曲
    void on_listWidgetPlayList_itemDoubleClicked(QListWidgetItem *item);
    //下一首
    void on_toolButtonNext_clicked();
    //上一首
    void on_toolButtonPre_clicked();
    //播放和暂停
    void on_toolButtonPlay_clicked();
    //声音按钮
    void on_toolButtonVoice_clicked();
    //读取歌词
    void readLyric(QString source_file_name);
    void on_toolButtonPlay_2_clicked();

private:
    Ui::TheMainWidget *ui;
    QMediaPlayer player;      //播放器
    QMediaPlaylist playerList;   //播放列表
    int currentIndexofPlayList;  //当前播放的下标
   // enum WhatDO{PRE,NEXT,PAUSE}; //未用到
    bool isPlaying; //播放的标志
    qint64 currentSongTime; //当前歌曲总时间
    QMovie movie;   //播放动画
    QFileInfo fi;  //读取歌曲文件信息，获取其名，再显示到列表上，从而不会显示绝对路径
    QThread *myThread; //普通线程
  //  MyThreadLyricShow *myshowlyricThread; //
    QMap<qint64, QString> lrc_map; //用来存放歌词
    QDialog *dialog; //
    QPushButton *btnExit;//返回按钮
    QListWidget *listLyric;
    QHBoxLayout *hbox;
    bool isDialogopen;


};

#endif // THEMAINWIDGET_H
