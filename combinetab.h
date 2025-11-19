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

class CombineTab : public QWidget {
    Q_OBJECT
public:
    explicit CombineTab(QWidget *parent = nullptr);

    // Public so main.cpp can call start/stop when Convert/Cancel is pressed
    void startConcatenation();
    void cancelConcatenation();

    // Returns the final output file path (for notification)
    QString getFinalOutputFile() const { return finalOutputFile; }

signals:
    void logMessage(const QString &msg);
    void conversionFinished();

private slots:
    void selectInputDirectory();
    void selectOutputDirectory();
    void populateTable();

private:
    QTemporaryFile *concatTempFile = nullptr;
    QLineEdit *inputDirEdit;
    QLineEdit *outputDirEdit;
    QLineEdit *outputNameEdit;
    QTableWidget *table;

    QStringList videoExtensions = {"*.mp4", "*.mkv", "*.webm", "*.avi", "*.mov", "*.wmv", "*.flv", "*.m4v", "*.ts", "*.m2ts", "*.mpg", "*.mpeg"};

    QString finalOutputFile;

    void createConcatListFile(const QMap<int, QString> &orderMap);
};

#endif // COMBINETAB_H
