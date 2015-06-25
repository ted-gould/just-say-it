#ifndef MYTYPE_H
#define MYTYPE_H

#include <QObject>
#include <QAudioRecorder>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AudioRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString text READ getText NOTIFY textChanged )
    Q_PROPERTY( QString state READ getState NOTIFY stateChanged )

public:
    explicit AudioRecorder (QObject *parent = 0);
    ~AudioRecorder ();

    Q_INVOKABLE void record ();
    Q_INVOKABLE void stop ();

Q_SIGNALS:
    void textChanged();
    void stateChanged();

protected slots:
    void onStateChanged (QMediaRecorder::State);
    void onUploadFinished ();
    void onContentFinished ();

protected:
    QString getText() { return m_text; }
    QString getState() {
        if (m_recorder.state() == QMediaRecorder::StoppedState) {
            if (m_upload != NULL && m_upload->isRunning())
                return "uploading";
            else if (m_content != NULL && m_content->isRunning())
                return "processing";
            else
                return "stopped";
        } else {
            return "recording";
        }
    }

    QString m_text;
    QAudioRecorder m_recorder;
    QNetworkAccessManager * m_qnam;
    QNetworkReply * m_upload;
    QNetworkReply * m_content;

private:
    void clearNetwork ();
};

#endif // MYTYPE_H

