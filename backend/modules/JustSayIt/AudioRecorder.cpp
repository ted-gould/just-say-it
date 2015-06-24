#include "AudioRecorder.h"
#include <QUrl>

AudioRecorder::AudioRecorder(QObject *parent) :
    QObject(parent),
    m_text(""),
    m_recorder(this)
{
    QAudioEncoderSettings settings;
    settings.setSampleRate(8096);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    settings.setChannelCount(1);
    settings.setCodec("audio/PCM");

    m_recorder.setEncodingSettings(settings);
    m_recorder.setContainerFormat("wav");
    m_recorder.setOutputLocation(QUrl::fromLocalFile("tmp.wav"));
    m_recorder.setAudioInput(m_recorder.defaultAudioInput());

    connect(&m_recorder, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SLOT(onStateChanged(QMediaRecorder::State)));
}

AudioRecorder::~AudioRecorder() {

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
}

