#include "server.hpp"

#include <QTcpServer>
#include <QSslSocket>

// @TODO
// replace this harcoded url by a read on the console
QString remote_url = "https://bitdash-a.akamaihd.net/content/MI201109210084_1/m3u8s/f08e80da-bf1d-4e3d-8899f0f6155f6efa.m3u8";
QString cdn_hostname = "bitdash-a.akamaihd.net";

constexpr int https_port = 443;    // 443 is the default https port @TODO we may want something more customizable
constexpr int http_port = 80;

constexpr int port_to_use = http_port;

Server::Server(QObject* parent /* = nullptr */)
    : QObject(parent)
{
    m_server = new QTcpServer(this);

    // Starting the server
    connect(m_server, SIGNAL(newConnection()), this, SLOT(new_connection()));

    if (!m_server->listen(QHostAddress::Any, port_to_use)) {
        qDebug() << "Server could not start";
    }
    else {
        qDebug() << "Server started!";
    }

    // Initiate the connection with the cdn
    m_cdn_socket = new QSslSocket();

    connect(m_cdn_socket, SIGNAL(hostFound()), this, SLOT(cdn_found()));
    connect(m_cdn_socket, SIGNAL(connected()), this, SLOT(cdn_connected()));
    connect(m_cdn_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(cdn_connection_error(QAbstractSocket::SocketError)));

    m_cdn_socket->connectToHost(cdn_hostname, port_to_use);
}

Server::~Server()
{
    delete m_cdn_socket;
    delete m_server;
}

void Server::new_connection()
{
    QByteArray  received_request;
    QByteArray  received_reply;

    // need to grab the socket
    QTcpSocket* socket = m_server->nextPendingConnection();

    socket->waitForReadyRead();
    received_request = socket->readAll();

    qInfo() << "[IN] https://localhost";
    qDebug() << received_request;

    // Forward the request in a synchronous way
    m_cdn_socket->write(received_request);
    m_cdn_socket->waitForBytesWritten(-1);

    // Wait for the reply of the CDN
    m_cdn_socket->waitForReadyRead();
    received_reply = m_cdn_socket->readAll();
    qInfo() << "[OUT] https://localhost";
    qDebug() << received_reply;

    // Forward the reply to the client
    m_cdn_socket->write(received_reply);
    m_cdn_socket->waitForBytesWritten(-1);


//    socket->write("Hello client\r\n");
//    socket->flush();

//    socket->waitForBytesWritten(3000);

    socket->close();
    delete socket;  // @Warning From the doc: The socket is created as a child of the server, which means that it is automatically deleted when the QTcpServer object is destroyed. It is still a good idea to delete the object explicitly when you are done with it, to avoid wasting memory.
}

void Server::cdn_found()
{
    qDebug() << "CDN found!";
}

void Server::cdn_connected()
{
    qDebug() << "CDN connected!";
}

void Server::cdn_connection_error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "CDN connection error: " << socketError;
}
