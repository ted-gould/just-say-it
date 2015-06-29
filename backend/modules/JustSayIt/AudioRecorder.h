#ifndef MYTYPE_H
#define MYTYPE_H

#include <QObject>
#include <QAudioRecorder>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBasicTimer>
#include <QTemporaryFile>

class AudioRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString text READ getText NOTIFY textChanged )
    Q_PROPERTY( QString state READ getState NOTIFY stateChanged )
    Q_PROPERTY( QString language READ getLanguage WRITE setLanguage )

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
    QString getState();
    QString getLanguage();
    void setLanguage(QString &inlang);

    void timerEvent (QTimerEvent *) Q_DECL_OVERRIDE;

    QString m_text;
    QString m_language;
    QAudioRecorder m_recorder;
    QNetworkAccessManager * m_qnam;
    QNetworkReply * m_upload;
    QNetworkReply * m_content;
    QBasicTimer m_timer;
    QTemporaryFile * m_tempfile;

private:
    void clearNetwork ();
};

#endif // MYTYPE_H

