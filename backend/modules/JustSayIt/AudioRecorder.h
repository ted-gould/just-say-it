#ifndef MYTYPE_H
#define MYTYPE_H

#include <QObject>

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

protected:
    QString getText() { return m_text; }
    QString getState() { return m_state; }

    QString m_text;
    QString m_state;
};

#endif // MYTYPE_H

