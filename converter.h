#ifndef CONVERTER_H
#define CONVERTER_H
#include <QObject>
#include <QProcess>
#include <QPointer>
#include <QMutex>
#include <QStringList>
#include <QProcessEnvironment>
class Converter : public QObject {
    Q_OBJECT
public:
    explicit Converter(QObject *parent = nullptr);
    void startConversion(const QString &inputFile, const QString &outputDir, const QString &baseName, const QStringList &args, bool twoPass, const QString &extension, const QString &codec, const QString &ffmpegPath, const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment(), bool overwrite = false);
    QString getFinalOutputFile() const;
public slots:
    void cancel();
signals:
    void logMessage(const QString &message);
    void progressUpdated(int value);
    void conversionFinished();
private slots:
    void processFinished(QPointer<QProcess> processPtr, int exitCode, QProcess::ExitStatus exitStatus, int pass, const QString &statsFile);
    void readProcessOutput();
private:
    void processFile(int pass = 0, const QString &statsFile = "");
    double getDuration(const QString &inputFile);
    QString getUniqueOutputFile(const QString& outputDir, const QString& baseName, const QString& extension);
    QList<QPointer<QProcess>> processes;
    QMutex processesMutex;
    QString file;
    QString outputDir;
    QString baseName;
    QStringList ffmpegArgs;
    double duration = 0.0;
    int currentPass = 0;
    QString finalOutputFile;
    QString codec;
    QString ffmpegPath; // Store custom FFmpeg path
    QProcessEnvironment processEnv; // Store custom environment
    bool overwriteFlag;
};
#endif // CONVERTER_H
