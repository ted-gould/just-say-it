#include "AudioRecorder.h"
#include <QUrl>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QFile>
#include <QJsonDocument>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
    , m_text("")
    , m_recorder(this)
    , m_upload(NULL)
    , m_content(NULL)
{
    m_qnam = new QNetworkAccessManager(this);

    QAudioEncoderSettings settings;
    settings.setSampleRate(8096);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    settings.setChannelCount(1);
    settings.setCodec("audio/PCM");

    m_recorder.setEncodingSettings(settings);
    m_recorder.setContainerFormat("wav");
    m_recorder.setOutputLocation(QUrl::fromLocalFile("/tmp/tmp.wav"));
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

void
AudioRecorder::onStateChanged (QMediaRecorder::State) {
    Q_EMIT stateChanged();
}

void
AudioRecorder::record() {
    m_recorder.record();
}

void
AudioRecorder::stop() {
    m_recorder.stop();
    qDebug() << "Recording at:" << m_recorder.actualLocation();

    clearNetwork();

    /* Goal: */
    /* curl -X POST --form "file=@/tmp/tmp.wav" --form "apikey=7a3505c1-dc73-423c-a095-9ab189563bd9" https://api.idolondemand.com/1/api/async/recognizespeech/v1 */

    QHttpMultiPart * multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    /* Set Key */
    QHttpPart apikey;
    apikey.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"apikey\""));
    apikey.setBody("7a3505c1-dc73-423c-a095-9ab189563bd9");
    multipart->append(apikey);

    /* Attach file */
    QHttpPart audiofile;
    audiofile.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"tmp.wav\""));
    audiofile.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav"));
    QFile * file = new QFile("/tmp/tmp.wav");
    file->open(QIODevice::ReadOnly);
    audiofile.setBodyDevice(file);
    file->setParent(multipart);

    multipart->append(audiofile);

    /* Do request */
    QUrl url("https://api.idolondemand.com/1/api/async/recognizespeech/v1");
    QNetworkRequest request(url);

    m_upload = m_qnam->post(request, multipart);
    multipart->setParent(m_upload);

    connect(m_upload, SIGNAL(finished()), this, SLOT(onUploadFinished()));

    Q_EMIT stateChanged();
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
    } else {
        m_text = "Error from webservice:\n";
        m_text += m_upload->readAll();

        Q_EMIT textChanged();
    }

    m_upload->deleteLater();
    m_upload = NULL;

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

            m_text = "Recognized Text: ";
            m_text += content;
       }
    } else {
        m_text = "Error from webservice:\n";
        m_text += m_content->readAll();
    }

    Q_EMIT textChanged();

    m_content->deleteLater();
    m_content = NULL;

    Q_EMIT stateChanged();
}
