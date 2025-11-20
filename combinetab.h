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

private slots:
    void selectInputDirectory();
    void selectOutputDirectory();
    void populateTable();
    void smartUpdateExtension();
    void updateCodecOptions();
    void onReencodeToggled(bool checked);

private:
    bool checkAllFilesCompatible(const QMap<int, QString> &orderMap, QString &videoCodec, QString &audioCodec);
    void createConcatListFile(const QMap<int, QString> &orderMap);

    QTemporaryFile *concatTempFile = nullptr;
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
};

#endif // COMBINETAB_H
