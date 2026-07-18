#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "networkclient.h"
#include <QQmlContext>
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    NetworkClient client;
    engine.rootContext()->setContextProperty("networkClient", &client);

    engine.loadFromModule("test_test", "Main");

    return QCoreApplication::exec();
}
