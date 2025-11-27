#ifndef COMBINETAB_H
#define COMBINETAB_H
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QSpinBox>
#include <QHeaderView>
#include <QMap>
#include <QFileInfo>
#include <QStringList>
#include <QTemporaryFile>
#include <QProcess>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

class CombineTab : public QWidget {
    Q_OBJECT
public:
    explicit CombineTab(QWidget *parent = nullptr);
    void startConcatenation();
    void cancelConcatenation();
    QString getFinalOutputFile() const;

signals:
    void logMessage(const QString &msg);
    void conversionFinished();
    void progressUpdated(int value);

private slots:
    void selectInputDirectory();
    void selectOutputDirectory();
    void populateTable();
    void smartUpdateExtension();
    void updateCodecOptions();
    void onReencodeToggled(bool checked);
    void onStdoutReady();
    void onStderrReady();

private:
    struct StreamInfo {
        QString videoCodec, audioCodec;
        int width = 0, height = 0;
        QString fps;
        QString pixFmt;
        QString sar;
        double duration = 0.0;
        bool hasAudio = false;
        int audioSampleRate = 0;
    };
    StreamInfo probeFile(const QString& file, const QString& ffprobePath);
    bool checkAllFilesCompatible(const QList<StreamInfo>& streams, StreamInfo& common);
    void createConcatListFile(const QMap<int, QString> &orderMap);

    QTemporaryFile *concatTempFile = nullptr;
    QProcess *concatProcess = nullptr;
    QLineEdit *inputDirEdit = nullptr;
    QLineEdit *outputDirEdit = nullptr;
    QLineEdit *outputNameEdit = nullptr;
    QTableWidget *table = nullptr;
    QComboBox *containerCombo = nullptr;
    QCheckBox *reencodeCheck = nullptr;
    QComboBox *targetCodecCombo = nullptr;
    QLabel *codecLabel = nullptr;
    QLineEdit *searchBox = nullptr;
    QStringList videoExtensions = {"*.mp4", "*.mkv", "*.webm", "*.avi", "*.mov", "*.wmv", "*.flv", "*.m4v", "*.ts", "*.m2ts", "*.mpg", "*.mpeg"};
    QString finalOutputFile;
    double totalDuration = 0.0;
};
#endif // COMBINETAB_H
