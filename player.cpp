#include "player.h"
#include "ui_player.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QTime>
#include <QSpinBox>
#include <QDataStream>
#include <QDebug>

Player::Player(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Player)
{
    ui->setupUi(this);
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(player);

    this->readMap();
    this->setButtonStyle();
    this->setPlaylist();
    this->videoWidget();
    this->setDuration();

    connect(ui->previous_but, &QToolButton::clicked,
            playlist, &QMediaPlaylist::previous);
    connect(ui->next_but, &QToolButton::clicked,
            this, &Player::next);
    connect(ui->play_but, &QToolButton::clicked,
            this, &Player::play);
    connect(ui->pause_but, &QToolButton::clicked,
            this, &Player::pause);
    connect(ui->stop_but, &QToolButton::clicked,
            this, &Player::stop);
    connect(ui->add_but, &QToolButton::clicked,
            this, &Player::addFile);
    connect(ui->del_but, &QToolButton::clicked,
            this, &Player::deleteFile);
    connect(ui->find_in_dir_button, &QToolButton::clicked,
            this, &Player::findVideo);
}

Player::~Player(){
    this->savePosition();
    this->saveMap();
    delete ui;
    delete model;
    delete playlist;
    delete player;
}

void Player::setButtonStyle(){
    ui->stop_but->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->play_but->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pause_but->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->sound->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    ui->next_but->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->previous_but->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
}
void Player::findVideo()
{
    QString dir_name = lineEdit->text();
    QDir dir;
    // Метод exists() проверяет наличие каталога
      if (dir.exists(dir_name)) {
        start(dir);
      }


}
void start(const QDir& dir){
    qint16 qnFileCount = 0;
     QDir dir( Dir);
     foreach( QFileInfo fi, dir.entryInfoList())
     {
     QString szFileName = fi.absoluteFilePath();
     if( fi.isDir())
     {
     if( fi.fileName()=="." || fi.fileName()=="..")
     continue;

     start( szFileName);
     }
     if( fi.isFile())
     {
     qnFileCount++;
     }
     }
}
void Player::setPlaylist()
{
    model = new QStandardItemModel(this);
    ui->tableView->setModel(model);
    model->setHorizontalHeaderLabels(QStringList()<< tr("File")
                                     << tr("File Path"));
    ui->tableView->hideColumn(1);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    player->setPlaylist(playlist);
    this->setSound();
    playlist->setPlaybackMode(QMediaPlaylist::Loop);


    connect(ui->tableView, &QTableView::doubleClicked,
            [this](const QModelIndex &index){
            playlist->setCurrentIndex(index.row());
            player->play();
            });
    connect(playlist, &QMediaPlaylist::currentIndexChanged,
            [this](int index){
            ui->currentTrack->setText(model->data(model->index(index, 0)).toString());
            });
}

void Player::videoWidget()
{
    widget = new QVideoWidget(ui->widget);
    player->setVideoOutput(widget);
    widget->resize(ui->widget->size());
    widget->show();
}

void Player::addFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open files"),
                                                      QString(),
                                                      tr("Audio Files (*.mp3 *.wav);; "
                                                         "Video files (*.avi)"));
    foreach (QString filePath, files) {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(QDir(filePath).dirName()));
        items.append(new QStandardItem(filePath));
        model->appendRow(items);
        playlist->addMedia(QUrl::fromLocalFile(filePath));
        widget = new QVideoWidget;
    }
}

void Player::deleteFile()
{
    savePosition();
    model->removeRow(ui->tableView->currentIndex().row());
    playlist->removeMedia(ui->tableView->currentIndex().row()+1);
    if(playlist->isEmpty()){
        ui->currentTrack->setText("Current Track");
        player->stop();
    }
    else{
        ui->currentTrack->setText(model->data(model->index(playlist->currentIndex(), 0)).toString());
    }
}

void Player::setSound()
{
    player->setVolume(m_sound);
    ui->spinBox->setValue(50);

    connect(ui->sound, &QToolButton::clicked,
            this, &Player::soundMute);
    connect(ui->spinBox, &QSpinBox::textChanged,
            this, &Player::soundChange);
}

void Player::soundChange(QString sound)
{
    if(sound.toInt()!=0)
        this->m_sound = sound.toInt();
    player->setVolume(sound.toInt());
}

void Player::soundMute()
{
    if(ui->spinBox->value() != 0){
        ui->spinBox->setValue(0);
        player->setMuted(true);
        ui->sound->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    } else {
        ui->spinBox->setValue(this->m_sound);
        player->setMuted(false);
        ui->sound->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }

}

void Player::setDuration()
{
    ui->timeSlider->setRange(0, player->duration()/1000);
    connect(ui->timeSlider, &QSlider::sliderMoved,
            this, &Player::seek);
    connect(player, &QMediaPlayer::durationChanged,
            this, &Player::durationChanged);
    connect(player, &QMediaPlayer::positionChanged,
            this, &Player::positionChanged);
}

void Player::durationChanged(qint64 duration)
{
    player_duration = duration/1000;
    ui->timeSlider->setMaximum(duration/1000);
}

void Player::positionChanged(qint64 progress)
{
    if(!ui->timeSlider->isSliderDown())
        ui->timeSlider->setValue(progress/1000);
    updateDurationInfo(progress/1000);
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || player_duration) {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
                           currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((player_duration / 3600) % 60, (player_duration / 60) % 60,
                         player_duration % 60, (player_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (player_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    ui->time->setText(tStr);
}

void Player::seek(int seconds)
{
    player->setPosition(seconds*1000);
}

void Player::savePosition()
{
    map.insert(playlist->currentMedia().request().url(), player->position());
}

void Player::readPosition()
{
    foreach(QUrl key, map.keys()){
        if(key.toString().compare(playlist->currentMedia().request().url().toString()) == 0){
            qint64 value = map.value(key);
            player->setPosition(value);
        }
    }
}

void Player::saveMap()
{
    QFile f ("position.txt");
    if (!f.open(QIODevice::WriteOnly)) return;
    QDataStream str(&f);
    str << map;
}

void Player::readMap()
{
    QFile f ("position.txt");
    if (!f.open(QIODevice::ReadOnly)) return;
    QDataStream str(&f);
    map.clear();
    str >> map;
    f.remove();
}

void Player::play()
{
    player->play();
    ui->play_but->setEnabled(false);
    ui->pause_but->setEnabled(true);
    ui->stop_but->setEnabled(true);
    readPosition();
}

void Player::next()
{
    savePosition();
    playlist->next();
    player->play();
}

void Player::pause()
{
    savePosition();
    ui->pause_but->setEnabled(false);
    ui->play_but->setEnabled(true);
    player->pause();
}

void Player::stop()
{
    ui->stop_but->setEnabled(false);
    ui->play_but->setEnabled(true);
    map.remove(playlist->currentMedia().request().url());
    player->stop();
}
