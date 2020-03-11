#include "themainwidget.h"
#include "ui_themainwidget.h"

TheMainWidget::TheMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TheMainWidget)
{
    ui->setupUi(this);
    initPort();//初始化
    //把歌曲导入播放列表 通过导入歌曲按钮实现 可防止重复导入歌曲
    connect(ui->pushButtonLoadSongs,&QPushButton::clicked,
            [=]()
            {
                QStringList tempList;
                QStringList mp3paths = QFileDialog::getOpenFileNames(this,"open file","../myNewMusic/mp3","MP3(*.mp3)");
                for(int i =0 ; i < mp3paths.size();++i)
                {
                    fi = QFileInfo(mp3paths.at(i));
                    tempList.push_back(fi.fileName());
                }
               if(ui->listWidgetPlayList->count()==0)
               {
                   ui->listWidgetPlayList->addItems(tempList);
                   for(int i = 0 ; i < mp3paths.size();i++)
                   {
                        //加载歌曲到播放列表
                         playerList.addMedia(QUrl::fromLocalFile(mp3paths.at(i)));
                   }
                   ui->toolButtonPlay->setEnabled(true);
                   return;
               }
               int j;
               int num = ui->listWidgetPlayList->count();
               cout << num;
                    for(int i=0;i<mp3paths.size();++i)
                    {
                        for(j = 0 ; j< num;++j)
                        {
                            if(tempList.at(i) == ui->listWidgetPlayList->item(j)->text())
                            {
                                QMessageBox::warning(this,"警告","文件已经导入");
                                break;
                            }
                        }
                        if(j==num)
                        {
                             ui->listWidgetPlayList->addItem(tempList.at(i));
                             playerList.addMedia(QUrl::fromLocalFile(mp3paths.at(i)));
                        }
                    }
            });
    //播放进度.更新滑块。
    connect(&player,&QMediaPlayer::positionChanged,this,&TheMainWidget::updatePosition);
    connect(ui->horizontalSliderPlay,&QSlider::sliderMoved,this,
            [=](int position){
                //设置播放jindu
                player.setPosition(position*1000);
            });
    connect(ui->verticalSliderVoice,&QSlider::sliderPressed,this,&TheMainWidget::slider_vilume_changed);
    connect(ui->verticalSliderVoice,&QSlider::sliderMoved,this,&TheMainWidget::slider_vilume_changed);
    connect(&player,&QMediaPlayer::durationChanged,
            [=]()
            {
                QString pathfile = QString("../myNewMusic/mp3/"+ui->listWidgetPlayList->item(playerList.currentIndex())->text());
                cout <<"PAth = :"<< pathfile;
                readLyric(pathfile);
            });
    connect(btnExit,&QPushButton::clicked,
            [=]()
            {
                dialog->hide();
            });
}
TheMainWidget::~TheMainWidget()
{
//    delete hbox;
//    delete btnExit;
//    delete listLyric;
//    delete dialog;
    delete ui;
}
//初始化一些变量
void TheMainWidget::initPort()
{
    currentIndexofPlayList = 0;
    ui->verticalSliderVoice->hide();
    movie.setFileName(":/image/skin/pig.gif");
    ui->label_2->setMovie(&movie);
    movie.start();
    isPlaying = false;
    ui->toolButtonPlay->setEnabled(false);
    ui->toolButtonNext->setEnabled(false);
    ui->toolButtonPre->setEnabled(false);
    ui->horizontalSliderPlay->setValue(0);
    ui->verticalSliderVoice->setMaximum(100);
    ui->verticalSliderVoice->setMinimum(0);
    ui->verticalSliderVoice->setValue(50);

    //播放器关联播放列表
    playerList.setPlaybackMode(QMediaPlaylist::Loop);
    player.setPlaylist(&playerList);
    player.setVolume(ui->verticalSliderVoice->value());

    //线程初始化
//    myThread = new QThread(this);
//    myshowlyricThread = new MyThreadLyricShow;
//    myshowlyricThread->moveToThread(myThread);
//    myThread->start();

    //歌词显示框初始化
    isDialogopen =false;
    dialog = new QDialog;

    dialog->hide();
    dialog->setModal(false);
    dialog->setFixedSize(800,600);
    btnExit = new QPushButton("关闭",dialog);
    btnExit->setFixedSize(100,50);
    listLyric = new QListWidget(dialog);

    hbox = new QHBoxLayout(dialog);
    hbox->addWidget(listLyric);
    hbox->addWidget(btnExit);
    dialog->setWindowFlags(Qt::FramelessWindowHint);//去除页面边框
    QPalette pl = dialog->palette();
    pl.setColor(QPalette::Window, QColor(0, 0, 0, 200));    //   设置背景颜色为黑色，如果不设置默认为白色
    dialog->setPalette(pl);
    dialog->setAutoFillBackground(true);
    dialog->setWindowOpacity(0.7);     //   背景和元素都设置透明效果
    dialog->setStyleSheet("");
}
//双击列表，播放歌曲
void TheMainWidget::on_listWidgetPlayList_itemDoubleClicked(QListWidgetItem *item)
{
    //获取当前歌曲在播放列表位置
    for(int i= 0; i< ui->listWidgetPlayList->count();i++)
    {
        if(ui->listWidgetPlayList->item(i)==item)
        {
            currentIndexofPlayList = i;
            cout << currentIndexofPlayList;
        }
    }
    playerList.setCurrentIndex(currentIndexofPlayList);
   player.play();
   isPlaying = true;
   ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_pause.png"));
  // ui->labelShowsong->setText(QString("正在播放：%1").arg(item->text()));
   ui->toolButtonNext->setEnabled(true);
   ui->toolButtonPre->setEnabled(true);
}
//下一曲
void TheMainWidget::on_toolButtonNext_clicked()
{

//    if(currentIndexofPlayList+1==ui->listWidgetPlayList->count())
//    {
//        currentIndexofPlayList = 0;
//    }else
//    {
//         currentIndexofPlayList++;
//    }

//    playerList.setCurrentIndex(currentIndexofPlayList);
    playerList.next();
    player.play();
    isPlaying = true;
    //需设置定时器来制造播放暂停切换状态的间隔时间
    ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_play.png"));
    ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_pause.png"));
}
//上一曲
void TheMainWidget::on_toolButtonPre_clicked()
{
//    if(currentIndexofPlayList-1==-1)
//    {
//        currentIndexofPlayList = ui->listWidgetPlayList->count()-1;
//    }else
//    {
//         currentIndexofPlayList--;
//    }
    playerList.previous();
  //  playerList.setCurrentIndex(currentIndexofPlayList);
    player.play();

    isPlaying = true;
    ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_play.png"));
    ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_pause.png"));
   // ui->labelShowsong->setText(QString("正在播放：%1").arg(ui->listWidgetPlayList->item(currentIndexofPlayList)->text()));

}
//播放及暂停按钮
void TheMainWidget::on_toolButtonPlay_clicked()
{
    if(isPlaying == false)
    {
        player.play();
        isPlaying = true;
        ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_pause.png"));
    }else
    {
        isPlaying = false;
        player.pause();
        ui->toolButtonPlay->setIcon(QIcon(":/image/trayMenu/menu_play.png"));
    }

    ui->toolButtonNext->setEnabled(true);
    ui->toolButtonPre->setEnabled(true);
}
//设置时间显示(播放时间/歌曲总时间)
void TheMainWidget::settime(qint64 playtime)
{
    qint64 m1,s1,m2,s2,currentTime;
    playtime /= 1000;  //获得的时间是以毫秒为单位的
   // cout << playtime;
    m1 = playtime/60;
    s1 = playtime-m1*60;
     //把int型转化为string类型后再设置为label的text
    currentTime =  player.position();
    currentTime/=1000;
    m2 = currentTime/60;
    s2=currentTime-m2*60;
    ui->labelPlayTime->setText(QString("%1:%2/%3:%4").arg(m2,2,10,QLatin1Char('0'))
                                                        .arg(s2,2,10,QLatin1Char('0'))
                                                          .arg(m1,2,10,QLatin1Char('0'))
                                                        .arg(s1,2,10,QLatin1Char('0')));
}
//播放音乐时，更新滑块，更新时间显示 更新歌曲显示信息 更新当前列表播放位置
void TheMainWidget::updatePosition(qint64 position)
{
    if(this->isVisible())
    {
        //设置滑块范围
        ui->horizontalSliderPlay->setMaximum(player.duration()/1000);
        settime(player.duration());
        ui->labelShowsong->setText(QString("正在播放:%1").arg(ui->listWidgetPlayList->item(playerList.currentIndex())->text()));
        ui->listWidgetPlayList->setCurrentItem(ui->listWidgetPlayList->item(playerList.currentIndex()));
        ui->horizontalSliderPlay->setMinimum(0);
   //     cout << position;
        ui->horizontalSliderPlay->setValue(position/1000);
        qint64 previous = 0;
        qint64 later = 0;
               //keys()方法返回lrc_map列表
//            foreach (qint64 value, lrc_map.keys()) {
//                   if (position >= value) {
//                       previous = value;
//                   } else {
//                       later = value;
//                       break;
//                   }
//               }
//               // 达到最后一行,将later设置为歌曲总时间的值
//               if (later == 0)
//                   later = player.duration();

//               // 获取当前时间所对应的歌词内容
//               QString current_lrc = lrc_map.value(previous);
//               QListWidgetItem cuitem(current_lrc);


               for(int i = 0; i<listLyric->count();i++)
               {
                   if(position/1000 == lrc_map.key(listLyric->item(i)->text())/1000)
                   {
                       listLyric->setCurrentRow(i);
                       break;
                   }
               }
    }

}
//音量控制
void TheMainWidget::slider_vilume_changed()
{
    player.setVolume(ui->verticalSliderVoice->value());
}
//音量点击
void TheMainWidget::on_toolButtonVoice_clicked()
{
    if(ui->verticalSliderVoice->isHidden())
    {
      ui->verticalSliderVoice->show();
    }else
    {
      ui->verticalSliderVoice->hide();
    }
}
//读取歌词
void TheMainWidget::readLyric(QString source_file_name)
{
    //防止关闭窗口的时候读取歌词，会出错。，所以只有程序运行才能读取歌词。
    if(this->isVisible())
    {
        lrc_map.clear();
        listLyric->clear();
        if(source_file_name.isEmpty())
            return;
        QString file_name = source_file_name;
        QString lrc_file_name = file_name.remove(file_name.length()-3,3) + "lrc";//把音频文件的后缀改成lrc后缀
        cout <<lrc_file_name;
        // 打开歌词文件
        QFile file(lrc_file_name);
        if (!file.open(QIODevice::ReadOnly)) {

            return ;
        }
       // 将歌词按行分解为歌词列表
        //这个是时间标签的格式[00:05.54]
        //正则表达式d{2}表示匹配2个数字
        QRegExp rx("\\[\\d{2}:\\d{2}\\.\\d{2}\\]");
        QTextStream in(&file);
        in.setCodec("GBK");
        while(!in.atEnd())
        {
            QString oneline = in.readLine();
             QString temp = oneline;
            temp.replace(rx, "");//用空字符串替换正则表达式中所匹配的地方,这样就获得了歌词文本
            // 然后依次获取当前行中的所有时间标签，并分别与歌词文本存入QMap中
            //indexIn()为返回第一个匹配的位置，如果返回为-1，则表示没有匹配成功
            //正常情况下pos后面应该对应的是歌词文件
            int pos = rx.indexIn(oneline, 0);
            while (pos != -1) { //表示匹配成功
                QString cap = rx.cap(0);//返回第0个表达式匹配的内容
                // 将时间标签转换为时间数值，以毫秒为单位
                QRegExp regexp;
                regexp.setPattern("\\d{2}(?=:)");
                regexp.indexIn(cap);
                int minute = regexp.cap(0).toInt();
                regexp.setPattern("\\d{2}(?=\\.)");
                regexp.indexIn(cap);
                int second = regexp.cap(0).toInt();
                regexp.setPattern("\\d{2}(?=\\])");
                regexp.indexIn(cap);
                int millisecond = regexp.cap(0).toInt();
                qint64 totalTime = minute * 60000 + second * 1000 + millisecond * 10;
                // 插入到lrc_map中
                lrc_map.insert(totalTime, temp);
                pos += rx.matchedLength();
                pos = rx.indexIn(oneline, pos);//匹配全部
                listLyric->addItem(temp);
                cout << totalTime <<":"<<temp;
            }
        }
        // 如果lrc_map为空
        if (lrc_map.isEmpty()) {

            return;
        }
    }
}
//歌词显示按钮
void TheMainWidget::on_toolButtonPlay_2_clicked()
{
    if(isDialogopen==false)
    {
        cout << "exec";
        dialog->show();
        isDialogopen = true;
        dialog->exec();

    }else
    {
        dialog->show();
    }

}
