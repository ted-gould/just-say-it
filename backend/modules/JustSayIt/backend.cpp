#include <QtQml>
#include <QtQml/QQmlContext>
#include "backend.h"
#include "AudioRecorder.h"


void BackendPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("JustSayIt"));

    qmlRegisterType<AudioRecorder>(uri, 1, 0, "AudioRecorder");
}

void BackendPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}

