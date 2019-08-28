#include "server.hpp"

#include <QCoreApplication>

int main(int argc, char** argv)
{
    QCoreApplication application(argc, argv);

    Server  server; // @Warning construction this object start the server

    return application.exec();
}
