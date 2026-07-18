#include <QCoreApplication>
#include <QTcpSocket>
#include "database.h"
#include "requesthandler.h"
#include "networkserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<QTcpSocket*>("QTcpSocket*");

    Database db("database.json");
    RequestHandler handler(&db);
    NetworkServer server(&handler);

    server.start(12345);

    return app.exec();
}