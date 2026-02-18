#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSettings>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QRandomGenerator>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QScrollArea>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QMutex>
#include <functional>
#include "av1tab.h"
#include "x265tab.h"
#include "x264tab.h"
#include "vp9tab.h"
#include "combinetab.h"
#include "trimtab.h"
#include "converter.h"
#include "presets.h"

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void selectFile();
    void browseOutputDirectory();
    void openSettings();
    void updateRecentMenu();
    void onFileSelected(const QString &file);
    void refreshOutputName();
    void detectCrop();
    void startConversion();
    void showInfo(const QString &inputFile);
    void onConversionFinished();
    void showConversionNotification(const QString &outputFile);
    void forceCustomPreset();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUi();
    void createFileSelectionSection();
    void createOutputNameSection();
    void createOutputDirSection();
    void createScalingSection();
    void createOptionsSection();
    void createPresetSection();
    void createCodecTabs();
    void createInfoAndConsoleTabs();
    void createConversionControls();
    void createMenuBar();
    void loadSettings();
    void saveSettings();
    void wireAllSignals();
    QString extractTitle(const QString &inputFile);

    QVBoxLayout *mainLayout = nullptr;
    QPushButton *selectFilesButton = nullptr;
    QLineEdit *selectedFilesBox = nullptr;
    QComboBox *outputNameModeBox = nullptr;
    QLineEdit *outputNameBox = nullptr;
    QCheckBox *overwriteCheck = nullptr;
    QPushButton *outputDirButton = nullptr;
    QLineEdit *outputDirBox = nullptr;
    QDoubleSpinBox *scaleWidthSpin = nullptr;
    QDoubleSpinBox *scaleHeightSpin = nullptr;
    QComboBox *scaleFilterBox = nullptr;
    QComboBox *scaleRangeBox = nullptr;
    QCheckBox *eightBitCheck = nullptr;
    QComboBox *eightBitColorFormatBox = nullptr;
    QCheckBox *tenBitCheck = nullptr;
    QComboBox *colorFormatBox = nullptr;
    QCheckBox *cropCheck = nullptr;
    QPushButton *detectCropButton = nullptr;
    QLineEdit *cropValueBox = nullptr;
    QCheckBox *seekCheck = nullptr;
    QLineEdit *seekHH = nullptr, *seekMM = nullptr, *seekSS = nullptr;
    QCheckBox *timeCheck = nullptr;
    QLineEdit *timeHH = nullptr, *timeMM = nullptr, *timeSS = nullptr;
    QComboBox *frameRateBox = nullptr;
    QLineEdit *customFrameRateBox = nullptr;
    QComboBox *rotationBox = nullptr;
    QCheckBox *preserveMetadataCheck = nullptr;
    QCheckBox *removeChaptersCheck = nullptr;
    QCheckBox *deinterlaceCheck = nullptr;
    QCheckBox *deblockCheck = nullptr;
    QCheckBox *normalizeAudioCheck = nullptr;
    QCheckBox *denoiseCheck = nullptr;
    QCheckBox *toneMapCheck = nullptr;
    QCheckBox *superSharpCheck = nullptr;
    QComboBox *presetCombo = nullptr;
    QCheckBox *videoSpeedCheck = nullptr;
    QComboBox *videoSpeedCombo = nullptr;
    QCheckBox *audioSpeedCheck = nullptr;
    QComboBox *audioSpeedCombo = nullptr;

    QTabWidget *codecTabs = nullptr;
    Av1Tab *av1Tab = nullptr;
    X265Tab *x265Tab = nullptr;
    X264Tab *x264Tab = nullptr;
    Vp9Tab *vp9Tab = nullptr;
    CombineTab *combineTab = nullptr;
    TrimTab *trimTab = nullptr;

    QScrollArea *combineScroll = nullptr;
    QScrollArea *trimScroll = nullptr;

    QTextEdit *infoBox = nullptr;
    QTextEdit *logBox = nullptr;
    QPushButton *clearLogButton = nullptr;
    QPushButton *convertButton = nullptr;
    QPushButton *cancelButton = nullptr;
    QProgressBar *conversionProgress = nullptr;

    Converter *converter = nullptr;
    QSettings *settings = nullptr;

    QAction *openAction = nullptr;
    QMenu *recentMenu = nullptr;
    QAction *settingsAction = nullptr;

    QString originalFilename;
    QString cachedMetadataTitle;
    QTimer *debounceTimer = nullptr;
    bool isProcessingInfo = false;
};

#endif // MAINWINDOW_H
