#include "server.hpp"

#include <QCoreApplication>

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication application(argc, argv);

    if (argc != 2) {
        std::cerr << "You have to pass the address of the cdn server as parameter!" << std::endl;
        return 1;
    }

    Server  server(argv[1]); // @Warning construction this object start the server

    return application.exec();
}
