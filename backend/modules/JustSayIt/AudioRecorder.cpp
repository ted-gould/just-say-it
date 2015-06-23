#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(QObject *parent) :
    QObject(parent),
    m_state("stopped"),
    m_text("")
{

}

AudioRecorder::~AudioRecorder() {

}

void
AudioRecorder::record() {
    m_state = "recording";
    Q_EMIT stateChanged();
}

void
AudioRecorder::stop() {
    m_state = "stopped";
    Q_EMIT stateChanged();
}

