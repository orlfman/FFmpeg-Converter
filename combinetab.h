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

private:
    void smartUpdateExtension();
    QTemporaryFile *concatTempFile = nullptr;
    QLineEdit *inputDirEdit;
    QLineEdit *outputDirEdit;
    QLineEdit *outputNameEdit;
    QTableWidget *table;
    QComboBox *containerCombo = nullptr;
    QStringList videoExtensions = {"*.mp4", "*.mkv", "*.webm", "*.avi", "*.mov", "*.wmv", "*.flv", "*.m4v", "*.ts", "*.m2ts", "*.mpg", "*.mpeg"};

    QString finalOutputFile;

    void createConcatListFile(const QMap<int, QString> &orderMap);
};

#endif // COMBINETAB_H
