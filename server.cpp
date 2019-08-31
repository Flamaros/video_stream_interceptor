#include "server.hpp"

#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>

// @TODO
// replace this harcoded url by a read on the console

QString remote_url = "http://mnmedias.api.telequebec.tv/m3u8/29880.m3u8";
//QString remote_url = "https://bitdash-a.akamaihd.net/content/MI201109210084_1/m3u8s/f08e80da-bf1d-4e3d-8899-f0f6155f6efa.m3u8";
QString cdn_hostname = "mnmedias.api.telequebec.tv";

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
    m_cdn_socket = new QTcpSocket();

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

    received_request = read_entiere_header(socket);

    qInfo() << "[IN] https://localhost";
    qDebug() << received_request;

    // @Warning we should fix the request before sending it to the CDN
    received_request.replace(QByteArray("Host: localhost"), QString("Host: " + cdn_hostname).toUtf8());

    // Forward the request in a synchronous way
    m_cdn_socket->write(received_request);
    m_cdn_socket->waitForBytesWritten(-1);

    // Wait for the reply of the CDN
    received_reply = read_entiere_reply(m_cdn_socket);
    qInfo() << "[OUT] https://localhost";
    qDebug() << received_reply;

    // Forward the reply to the client
    m_cdn_socket->write(received_reply);
    m_cdn_socket->waitForBytesWritten(-1);

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

// @TODO should be unit tested
QByteArray Server::read_entiere_header(QTcpSocket* socket)
{
    QByteArray  request;
    int         from = 0;

    socket->waitForReadyRead();
    while (request.indexOf("\r\n\r\n", from) == -1)
    {
        from = std::max(0, request.size() - 2);  // @Warning - 2 be sure that CLRFCLRF can be found if truncated between two chunk of data, else we can get an infinite loop
        request += socket->readAll();
    }
    return request;
}

// @TODO should be unit tested
QByteArray Server::read_entiere_reply(QTcpSocket *socket)
{
    // @TODO @SpeedUp all of this can be optimized by a lot
    // by using cached buffer to avoid allocation
    QByteArray  header_data;
    QByteArray  reply_data;
    HTTP_Header header;

    header_data = read_entiere_header(socket);
    header = parse_http_header(header_data);

    while (reply_data.size() < header.content_length) {
        reply_data += socket->readAll();
    }

    return header_data + reply_data;    // @TODO @SpeedUp Rewrite the API, returning a buffer like this makes copies, the client of this API should be able to send those 2 buffers individually and avoid this unecessary concatanation
}

Server::HTTP_Header Server::parse_http_header(const QString& http_header)
{
    // @TODO This can be much more improved
    // We can use a proper parser with a Tokenizer and then
    // build the AST
    // A good parser will not do much allocations but use
    // string views instead to avoid copies

    HTTP_Header         header;
    QVector<QStringRef> slices;

    slices = http_header.splitRef("\r\n");
    for (const QStringRef& slice : slices) {
        QVector<QStringRef> pair_variable_value;

        pair_variable_value = slice.split(':');
        if (pair_variable_value.size() == 2)
        {
            if (pair_variable_value[0] == "Content-Length") {
                header.content_length = pair_variable_value[1].toInt(); // @TODO normally we should retrieve and check the ok parameter
            }
        }
    }

    return header;
}
