#ifndef TRIMTAB_H
#define TRIMTAB_H

#include <QWidget>
#include <QList>
#include <QPair>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QCheckBox>

class TrimTab : public QWidget
{
    Q_OBJECT
public:
    explicit TrimTab(QWidget *parent = nullptr);
    QList<QPair<qint64, qint64>> getSegments() const { return segments; }
    int getCodecIndex() const;
    void setDefaultCodec(int index);
    bool isLosslessTrim() const;

public slots:
    void setInputFile(const QString &file);
    void stopPreviewPlayer();
    void restartPreviewPlayer();

private:
    QComboBox *codecCombo;
    QLabel *inputFileLabel;
    QVideoWidget *videoWidget;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QSlider *positionSlider;
    QPushButton *playPauseButton;
    QLabel *currentTimeLabel;
    QLabel *durationLabel;
    QLineEdit *startTimeEdit;
    QLineEdit *endTimeEdit;
    QPushButton *setStartButton;
    QPushButton *setEndButton;
    QPushButton *addSegmentButton;
    QTableWidget *segmentsTable;
    QPushButton *removeButton;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *clearButton;
    QComboBox *speedCombo;
    QString currentInputFile;
    QCheckBox *losslessCheck;

    QList<QPair<qint64, qint64>> segments;

    QString formatTime(qint64 ms);
    qint64 parseTime(const QString &str);
    void updateCurrentTime(qint64 pos);
    void updateTable();
};

#endif // TRIMTAB_H
