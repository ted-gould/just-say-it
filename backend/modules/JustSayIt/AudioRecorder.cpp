#include "AudioRecorder.h"
#include <QUrl>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QFile>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
    , m_text("")
    , m_recorder(this)
    , m_reply(NULL)
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
    if (m_reply != NULL) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = NULL;
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

    if (m_reply != NULL) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = NULL;
    }

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

    m_reply = m_qnam->post(request, multipart);
    multipart->setParent(m_reply);

    connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void
AudioRecorder::onFinished () {
    Q_EMIT stateChanged();

    if (m_reply->error() == QNetworkReply::NoError) {
//       QVariant map(m_reply->readAll());
//       m_text = map.toMap()["document"].toList()[0].toMap()["content"].toString();
        m_text = m_reply->readAll();
    } else {
        m_text = "Error from webservice:\n";
        m_text += m_reply->readAll();
    }

    Q_EMIT textChanged();
}
