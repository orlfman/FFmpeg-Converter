#include "trimtab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QHeaderView>
#include <QFileInfo>
#include <QTableWidgetItem>
#include <algorithm>

TrimTab::TrimTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // Codec selector
    QHBoxLayout *codecLay = new QHBoxLayout();
    codecLay->addWidget(new QLabel("Encoding Codec:"));
    codecCombo = new QComboBox();
    codecCombo->addItems({"AV1", "x265", "VP9"});
    codecLay->addWidget(codecCombo);
    codecLay->addStretch();
    mainLayout->addLayout(codecLay);
    // Input file label
    inputFileLabel = new QLabel("Input file: No file selected");
    mainLayout->addWidget(inputFileLabel);
    // Video player
    videoWidget = new QVideoWidget();
    videoWidget->setMinimumSize(640, 360);
    videoWidget->setStyleSheet("background-color: black;");
    mainLayout->addWidget(videoWidget);
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoWidget);
    // Playback controls
    QHBoxLayout *controls = new QHBoxLayout();
    playPauseButton = new QPushButton("Play");
    controls->addWidget(playPauseButton);
    positionSlider = new QSlider(Qt::Horizontal);
    controls->addWidget(positionSlider);
    currentTimeLabel = new QLabel("00:00:00.000");
    controls->addWidget(currentTimeLabel);
    durationLabel = new QLabel("/ 00:00:00.000");
    controls->addWidget(durationLabel);

    // Playback speed control
    QLabel *speedLabel = new QLabel("Speed:");
    controls->addWidget(speedLabel);
    speedCombo = new QComboBox();
    speedCombo->addItems({"0.25x", "0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "1.75x", "2.0x"});
    speedCombo->setCurrentText("1.0x");
    speedCombo->setFixedWidth(80);
    controls->addWidget(speedCombo);

    controls->addStretch();
    mainLayout->addLayout(controls);
    // Mark start/end and add segment
    QHBoxLayout *marks = new QHBoxLayout();
    marks->addWidget(new QLabel("Start time:"));
    startTimeEdit = new QLineEdit("00:00:00.000");
    marks->addWidget(startTimeEdit);
    setStartButton = new QPushButton("Mark Start");
    marks->addWidget(setStartButton);
    marks->addSpacing(20);
    marks->addWidget(new QLabel("End time:"));
    endTimeEdit = new QLineEdit();
    marks->addWidget(endTimeEdit);
    setEndButton = new QPushButton("Mark End");
    marks->addWidget(setEndButton);
    addSegmentButton = new QPushButton("Add Segment");
    marks->addWidget(addSegmentButton);
    marks->addStretch();
    mainLayout->addLayout(marks);
    // Segments table
    segmentsTable = new QTableWidget(0, 3);
    segmentsTable->setHorizontalHeaderLabels({"Start", "End", "Duration"});
    segmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    segmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    segmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(segmentsTable);
    // Segment buttons
    QHBoxLayout *btns = new QHBoxLayout();
    removeButton = new QPushButton("Remove Selected");
    btns->addWidget(removeButton);
    upButton = new QPushButton("Move Up");
    btns->addWidget(upButton);
    downButton = new QPushButton("Move Down");
    btns->addWidget(downButton);
    clearButton = new QPushButton("Clear All");
    btns->addWidget(clearButton);
    btns->addStretch();
    mainLayout->addLayout(btns);
    mainLayout->addStretch();
    // Connections
    connect(playPauseButton, &QPushButton::clicked, this, [this]() {
        if (player->playbackState() == QMediaPlayer::PlayingState) {
            player->pause();
            playPauseButton->setText("Play");
        } else if (player->playbackState() != QMediaPlayer::StoppedState) {
            player->play();
            playPauseButton->setText("Pause");
        }
    });
    connect(player, &QMediaPlayer::durationChanged, this, [this](qint64 dur) {
        positionSlider->setRange(0, dur);
        durationLabel->setText("/ " + formatTime(dur));
        if (endTimeEdit->text().isEmpty()) {
            endTimeEdit->setText(formatTime(dur));
        }
    });
    connect(player, &QMediaPlayer::positionChanged, this, &TrimTab::updateCurrentTime);
    connect(positionSlider, &QSlider::valueChanged, player, &QMediaPlayer::setPosition);

    // Speed control connection
    connect(speedCombo, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        QString numStr = text.chopped(1); // remove 'x'
    bool ok;
    double rate = numStr.toDouble(&ok);
    if (ok && rate > 0.0) {
        player->setPlaybackRate(rate);
    }
    });

    connect(setStartButton, &QPushButton::clicked, this, [this]() {
        startTimeEdit->setText(formatTime(player->position()));
    });
    connect(setEndButton, &QPushButton::clicked, this, [this]() {
        endTimeEdit->setText(formatTime(player->position()));
    });
    connect(addSegmentButton, &QPushButton::clicked, this, [this]() {
        qint64 start = parseTime(startTimeEdit->text());
        qint64 end = parseTime(endTimeEdit->text());
        if (start < 0 || end < 0 || start >= end || end > player->duration()) {
            QMessageBox::warning(this, "Invalid Segment", "Start must be before end and within video length.");
            return;
        }

        // Test adding the new segment
        QList<QPair<qint64, qint64>> temp = segments;
        temp.append({start, end});

        // Sort by start time
        std::sort(temp.begin(), temp.end(), [](const QPair<qint64, qint64>& a, const QPair<qint64, qint64>& b) {
            return a.first < b.first;
        });

        // Check for any overlap
        bool hasOverlap = false;
        for (int i = 1; i < temp.size(); ++i) {
            if (temp[i].first < temp[i - 1].second) {
                hasOverlap = true;
                break;
            }
        }

        if (hasOverlap) {
            QMessageBox::warning(this, "Invalid Segment", "This segment overlaps with an existing segment and cannot be added.");
            return;
        }

        // No overlap → accept it
        segments = temp;
        updateTable();
        startTimeEdit->setText(formatTime(end));
    });
    connect(removeButton, &QPushButton::clicked, this, [this]() {
        int row = segmentsTable->currentRow();
        if (row >= 0) {
            segments.removeAt(row);
            updateTable();
        }
    });
    connect(upButton, &QPushButton::clicked, this, [this]() {
        int row = segmentsTable->currentRow();
        if (row > 0) {
            segments.swapItemsAt(row, row - 1);
            updateTable();
            segmentsTable->selectRow(row - 1);
        }
    });
    connect(downButton, &QPushButton::clicked, this, [this]() {
        int row = segmentsTable->currentRow();
        if (row >= 0 && row < segments.size() - 1) {
            segments.swapItemsAt(row, row + 1);
            updateTable();
            segmentsTable->selectRow(row + 1);
        }
    });
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        segments.clear();
        updateTable();
    });
}

int TrimTab::getCodecIndex() const
{
    return codecCombo->currentIndex();
}

void TrimTab::setInputFile(const QString &file)
{
    inputFileLabel->setText("Input file: " + (file.isEmpty() ? "No file selected" : QFileInfo(file).fileName()));
    player->stop();
    if (file.isEmpty()) {
        positionSlider->setRange(0, 0);
        currentTimeLabel->setText("00:00:00.000");
        durationLabel->setText("/ 00:00:00.000");
        startTimeEdit->clear();
        endTimeEdit->clear();
        segments.clear();
        updateTable();
        speedCombo->setCurrentText("1.0x");
        currentInputFile.clear();
        return;
    }
    currentInputFile = file;
    player->setSource(QUrl::fromLocalFile(file));
    player->setPlaybackRate(1.0);
    player->pause();
    playPauseButton->setText("Play");
    startTimeEdit->setText("00:00:00.000");
    endTimeEdit->clear();
    segments.clear();
    updateTable();
    speedCombo->setCurrentText("1.0x");
}

QString TrimTab::formatTime(qint64 ms)
{
    if (ms <= 0) return "00:00:00.000";
    qint64 secs = ms / 1000;
    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = secs % 60;
    int mss = ms % 1000;
    return QString("%1:%2:%3.%4")
    .arg(h, 2, 10, QChar('0'))
    .arg(m, 2, 10, QChar('0'))
    .arg(s, 2, 10, QChar('0'))
    .arg(mss, 3, 10, QChar('0'));
}

qint64 TrimTab::parseTime(const QString &str)
{
    QRegularExpression re(R"(^(\d+):(\d+):(\d+)\.(\d{3})$)");
    auto match = re.match(str.trimmed());
    if (match.hasMatch()) {
        int h = match.captured(1).toInt();
        int m = match.captured(2).toInt();
        int s = match.captured(3).toInt();
        int ms = match.captured(4).toInt();
        if (m < 60 && s < 60) {
            return ((qint64)h * 3600 + m * 60 + s) * 1000 + ms;
        }
    }
    return -1;
}

void TrimTab::updateCurrentTime(qint64 pos)
{
    currentTimeLabel->setText(formatTime(pos));
    if (!positionSlider->isSliderDown()) {
        positionSlider->setValue(pos);
    }
}

void TrimTab::updateTable()
{
    segmentsTable->setRowCount(segments.size());
    for (int i = 0; i < segments.size(); ++i) {
        qint64 dur = segments[i].second - segments[i].first;
        segmentsTable->setItem(i, 0, new QTableWidgetItem(formatTime(segments[i].first)));
        segmentsTable->setItem(i, 1, new QTableWidgetItem(formatTime(segments[i].second)));
        segmentsTable->setItem(i, 2, new QTableWidgetItem(formatTime(dur)));
    }
}

void TrimTab::setDefaultCodec(int index)
{
    codecCombo->setCurrentIndex(index);
}

void TrimTab::stopPreviewPlayer()
{
    player->stop();
    player->setSource(QUrl());
}

void TrimTab::restartPreviewPlayer()
{
    if (currentInputFile.isEmpty()) return;
    player->stop();
    player->setSource(QUrl::fromLocalFile(currentInputFile));
    QString speedText = speedCombo->currentText();
    bool ok;
    double rate = speedText.chopped(1).toDouble(&ok);
    if (ok && rate > 0.0) {
        player->setPlaybackRate(rate);
    } else {
        player->setPlaybackRate(1.0);
        speedCombo->setCurrentText("1.0x");
    }
    player->pause();
    playPauseButton->setText("Play");
}
