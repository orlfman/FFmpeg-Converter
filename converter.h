#ifndef CONVERTER_H
#define CONVERTER_H
#include <QObject>
#include <QProcess>
#include <QPointer>
#include <QMutex>
#include <QStringList>
#include <QProcessEnvironment>
// The Converter class handles the actual video conversion process using FFmpeg
class Converter : public QObject {
    Q_OBJECT
public:
    // Basic constructor, takes a parent if needed
    explicit Converter(QObject *parent = nullptr);
    // Kicks off the conversion with all the necessary params
    void startConversion(const QString &inputFile, const QString &outputDir, const QString &baseName, const QStringList &args, bool twoPass, const QString &extension, const QString &codec, const QString &ffmpegPath, const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment(), bool overwrite = false, const QString& seekTimeStr = "", const QString& outputDurationStr = "", double videoSpeedMult = 1.0);
    // Gets the path to the final output file after conversion
    QString getFinalOutputFile() const;
public slots:
    // Slot to cancel the ongoing conversion
    void cancel();
signals:
    // Signal for logging messages during the process
    void logMessage(const QString &message);
    // Updates the progress bar
    void progressUpdated(int value);
    // Fired when the conversion is done
    void conversionFinished();
private slots:
    // Handles what happens when a process finishes
    void processFinished(QPointer<QProcess> processPtr, int exitCode, QProcess::ExitStatus exitStatus, int pass, const QString &statsFile);
    // Reads output from the process for logging and progress
    void readProcessOutput();
private:
    // Internal method to process the file for a given pass
    void processFile(int pass = 0, const QString &statsFile = "");
    // Gets the duration of the input video for progress calculation
    double getDuration(const QString &inputFile);
    // Finds a unique filename for the output if needed
    QString getUniqueOutputFile(const QString& outputDir, const QString& baseName, const QString& extension);
    // Helper to parse HH:MM:SS to seconds
    double parseTimeToSeconds(const QString& timeStr);
    // List of running processes
    QList<QPointer<QProcess>> processes;
    // Mutex for thread safety with processes
    QMutex processesMutex;
    // Input file path
    QString file;
    // Output directory
    QString outputDir;
    // Base name for output file
    QString baseName;
    // FFmpeg arguments
    QStringList ffmpegArgs;
    // Video duration in seconds
    double duration = 0.0;
    // Current pass number for two-pass encoding
    int currentPass = 0;
    // Final output file path
    QString finalOutputFile;
    // Codec being used
    QString codec;
    // Path to FFmpeg executable
    QString ffmpegPath;
    // Process environment
    QProcessEnvironment processEnv;
    // Flag for overwriting files
    bool overwriteFlag;
    // Expected output duration (adjusted for speed/seek/time)
    double expectedOutputDuration = 0.0;
    // Video speed multiplier
    double videoSpeedMultiplier = 1.0;
    // Seek and duration strings
    QString seekTimeStr;
    QString outputDurationStr;
};
#endif // CONVERTER_H
