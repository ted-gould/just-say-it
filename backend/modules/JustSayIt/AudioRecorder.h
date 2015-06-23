#ifndef MYTYPE_H
#define MYTYPE_H

#include <QObject>

class AudioRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString helloWorld READ helloWorld WRITE setHelloWorld NOTIFY helloWorldChanged )

public:
    explicit AudioRecorder (QObject *parent = 0);
    ~AudioRecorder ();

Q_SIGNALS:
    void helloWorldChanged();

protected:
    QString helloWorld() { return m_message; }
    void setHelloWorld(QString msg) { m_message = msg; Q_EMIT helloWorldChanged(); }

    QString m_message;
};

#endif // MYTYPE_H

