#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString getDefaultOutputDir() const;
    int getDefaultCodecTab() const;
    QString getFFmpegPath() const;
    bool getNotifyOnFinish() const;
    QString getSvtAv1Path() const;

private slots:
    void browseOutputDirectory();
    void browseFFmpegPath();
    void browseSvtAv1Path();

private:
    void loadSettings();

    QComboBox *defaultCodecComboBox = nullptr;
    QLineEdit *ffmpegPathLineEdit = nullptr;
    QPushButton *ffmpegPathButton = nullptr;
    QLineEdit *svtAv1PathLineEdit = nullptr;
    QPushButton *svtAv1PathButton = nullptr;
    QLineEdit *defaultOutputDirLineEdit = nullptr;
    QPushButton *defaultOutputDirButton = nullptr;
    QCheckBox *notifyOnFinishCheck = nullptr;
};

#endif
