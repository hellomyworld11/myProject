#ifndef MYTHREADLYRICSHOW_H
#define MYTHREADLYRICSHOW_H

#include <QObject>

class MyThreadLyricShow : public QObject
{
    Q_OBJECT
public:
    explicit MyThreadLyricShow(QObject *parent = 0);
    void showLyric(qint64 currT);
signals:

public slots:


private:


};

#endif // MYTHREADLYRICSHOW_H
