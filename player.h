#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItemModel>
#include <QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QWidget
{
    Q_OBJECT

public:
    Player(QWidget *parent = nullptr);
    ~Player();

private:
    Ui::Player *ui;
    QStandardItemModel *model;
    QVideoWidget *widget;
    QMediaPlayer *player;
    QMediaPlaylist *playlist;
    int m_sound = 50;
    qint64 player_duration;
    QMap <QUrl, qint64> map;

private:
    void setButtonStyle();
    void setPlaylist();
    void videoWidget();
    void setSound();
    void setDuration();
    void savePosition();
    void readPosition();
    void saveMap();
    void readMap();

private slots:

    void play();
    void next();
    void pause();
    void stop();
    void addFile();
    void deleteFile();
    void soundChange(QString sound);
    void soundMute();
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void seek(int seconds);
    void updateDurationInfo(qint64 currentInfo);
    void findVideo();
};
#endif // PLAYER_H
