#include "AudioRecorder.h"
#include <QUrl>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QFile>
#include <QJsonDocument>
#include <QTimerEvent>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
    , m_text("")
    , m_language("en-US")
    , m_recorder(this)
    , m_upload(NULL)
    , m_content(NULL)
{
    m_qnam = new QNetworkAccessManager(this);

    QAudioEncoderSettings settings;
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    settings.setChannelCount(1);
    settings.setSampleRate(16000);
    settings.setCodec("audio/PCM");

    m_recorder.setEncodingSettings(settings);
    m_recorder.setContainerFormat("wav");
    m_recorder.setAudioInput(m_recorder.defaultAudioInput());

    connect(&m_recorder, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SLOT(onStateChanged(QMediaRecorder::State)));
}

AudioRecorder::~AudioRecorder() {
    clearNetwork();
}

void
AudioRecorder::clearNetwork () {
    if (m_upload != NULL) {
        m_upload->abort();
        m_upload->deleteLater();
        m_upload = NULL;
    }
    if (m_content != NULL) {
        m_content->abort();
        m_content->deleteLater();
        m_content = NULL;
    }
}

QString
AudioRecorder::getState() {
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

QString
AudioRecorder::getLanguage()
{
    return m_language;
}

void
AudioRecorder::setLanguage(QString &inlang)
{
    m_language = inlang;
}

void
AudioRecorder::onStateChanged (QMediaRecorder::State state) {
    if (state == QMediaRecorder::StoppedState) {
        clearNetwork();

        /* Goal: */
        /* curl -X POST --form "file=@/tmp/tmp.wav" --form "apikey=7a3505c1-dc73-423c-a095-9ab189563bd9" https://api.idolondemand.com/1/api/async/recognizespeech/v1 */

        QHttpMultiPart * multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        /* Set Key */
        QHttpPart apikey;
        apikey.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"apikey\""));
        apikey.setBody("7a3505c1-dc73-423c-a095-9ab189563bd9");
        multipart->append(apikey);

        /* Set Launguage */
        QHttpPart langpart;
        langpart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"language\""));
        langpart.setBody(m_language.toUtf8());
        multipart->append(langpart);
        qDebug() << "Sending language as:" << m_language.toUtf8();

        /* Attach file */
        QHttpPart audiofile;
        audiofile.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"tmp.wav\""));
        audiofile.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav"));
        m_tempfile->open();
        audiofile.setBodyDevice(&(*m_tempfile));

        multipart->append(audiofile);

        /* Do request */
        QUrl url("https://api.idolondemand.com/1/api/async/recognizespeech/v1");
        QNetworkRequest request(url);

        m_upload = m_qnam->post(request, multipart);
        multipart->setParent(m_upload);

        connect(m_upload, SIGNAL(finished()), this, SLOT(onUploadFinished()));
    }

    Q_EMIT stateChanged();
}

void
AudioRecorder::record() {
    if (m_text != "") {
        m_text = "";
        Q_EMIT textChanged();
    }

    m_tempfile = std::make_shared<QTemporaryFile>();
    m_tempfile->open(); /* Creates the file, but we can't use it with Audio Recorder */
    m_tempfile->close();

    m_recorder.setOutputLocation(QUrl::fromLocalFile(m_tempfile->fileName()));
    m_recorder.setMuted(false);
    m_recorder.record();
}

void
AudioRecorder::stop() {
    if (m_recorder.state() == QMediaRecorder::RecordingState) {
        m_recorder.setMuted(true);
        /* We're putting a second of silence at the end, it seems to really help the detection */
        m_timer.start(1000, Qt::CoarseTimer, this);
    } else if (m_upload != NULL || m_content != NULL) {
        if (m_upload) {
            m_upload->abort();
        }
        if (m_content) {
            m_content->abort();
        }
    }
}

void
AudioRecorder::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_recorder.stop();
        m_timer.stop(); /* Make it a one shot */
    }
}

void
AudioRecorder::onUploadFinished () {
    if (m_upload->error() == QNetworkReply::NoError) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(m_upload->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing feed entries" << error.errorString();
        } else {
            QVariant map = jsonDoc.toVariant();
            QString jobid = map.toMap()["jobID"].toString();
            qDebug() << "JobID:" << jobid;

            QUrl url("https://api.idolondemand.com/1/job/result/" + jobid + "?apikey=7a3505c1-dc73-423c-a095-9ab189563bd9");
            QNetworkRequest request(url);

            m_content = m_qnam->get(request);
            connect(m_content, SIGNAL(finished()), this, SLOT(onContentFinished()));
       }
    } else if (m_upload->error() == QNetworkReply::OperationCanceledError) {
        qDebug("Upload cancelled");
        /* No Op */
    } else {
        m_text = "Error from webservice:\n";
        m_text += m_upload->readAll();

        Q_EMIT textChanged();
    }

    m_upload->deleteLater();
    m_upload = NULL;
    m_tempfile.reset();

    Q_EMIT stateChanged();
}

void
AudioRecorder::onContentFinished () {
    if (m_content->error() == QNetworkReply::NoError) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(m_content->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing feed entries" << error.errorString();
        } else {
            QVariant map = jsonDoc.toVariant();
            QString content = map.toMap()["actions"].toList()[0].toMap()["result"].toMap()["document"].toList()[0].toMap()["content"].toString();

            m_text = content;
       }
    } else if (m_content->error() == QNetworkReply::OperationCanceledError) {
        /* No Op */
        qDebug("Processing cancelled");
    } else {
        m_text = "Error from webservice:\n";
        m_text += m_content->readAll();
    }

    Q_EMIT textChanged();

    m_content->deleteLater();
    m_content = NULL;

    Q_EMIT stateChanged();
}
