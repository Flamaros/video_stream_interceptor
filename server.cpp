#include "server.hpp"

#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>

#include <iostream>

constexpr int https_port = 443;    // 443 is the default https port @TODO we may want something more customizable
constexpr int http_port = 80;

constexpr int port_to_use = http_port;

Server::Server(QString cnd_address, QObject* parent /* = nullptr */)
    : QObject(parent)
    , m_cdn_address(cnd_address)
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

/*    connect(m_cdn_socket, SIGNAL(hostFound()), this, SLOT(cdn_found()));
    connect(m_cdn_socket, SIGNAL(connected()), this, SLOT(cdn_connected()));
    connect(m_cdn_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(cdn_connection_error(QAbstractSocket::SocketError)));

    m_cdn_socket->connectToHost(m_cdn_address, port_to_use);*/
}

Server::~Server()
{
    delete m_cdn_socket;
    delete m_server;
}

void Server::new_connection()
{
    QByteArray      received_request_data;
    QByteArray      received_reply_data;
    QByteArray      received_reply_body;
    HTTP_Header     client_request_header;
    HTTP_Header     cdn_reply_header;
    QTime           timer;
    Content_Type    content_type;
    QString         content_type_text;

    // need to grab the socket
    QTcpSocket* client_socket = m_server->nextPendingConnection();

    m_cdn_socket->connectToHost(m_cdn_address, port_to_use); // @Warning we start to connect to the CDN as soon as possible to win a little amount of time (due to asynchronous operation)

    read_everything(client_socket, received_request_data, client_request_header);

    if (client_request_header.url.endsWith(".m3u8")) {  // @TODO Wait the day when reflection on enum will be available to generate the text instead of hard code it
        content_type = Content_Type::manifest;
        content_type_text = "[MANIFEST]";
    } else if (client_request_header.url.endsWith(".ts")) {
        content_type = Content_Type::segment;
        content_type_text = "[SEGMENT]";
    }

    if (content_type == Content_Type::manifest) {
        std::cout << "\033[31m[TRACK SWITCH]\033[0m" << std::endl;
    }

    std::cout << "\033[31m[IN]" << qPrintable(content_type_text) << "\033[0m https://localhost" << qPrintable(client_request_header.url.toString()) << std::endl;
    qDebug() << QString(received_request_data);

    // @Warning we should fix the request before sending it to the CDN
    received_request_data.replace(QByteArray("Host: localhost"), QString("Host: " + m_cdn_address).toUtf8());

    // Forward the request in a synchronous way
    if (m_cdn_socket->waitForConnected(5 * 1000)) { // @TODO Do something better this is not robust. cf Qt doc : Note: This function may fail randomly on Windows. Consider using the event loop and the connected() signal if your software will run on Windows.
        timer.start();

        m_cdn_socket->write(received_request_data);
        m_cdn_socket->waitForBytesWritten(-1);

        // Wait for the reply of the CDN
        read_everything(m_cdn_socket, received_reply_data, cdn_reply_header);

        std::cout << "\033[31m[OUT]" << qPrintable(content_type_text) << "\033[0m https://localhost" << qPrintable(client_request_header.url.toString()) << " (" << timer.elapsed() << "ms)" << std::endl;
        qDebug() << QString(received_reply_data);

        // @Warning we should fix absolute urls that can be in the response
        if (content_type == Content_Type::manifest) {
            received_reply_data.replace(m_cdn_address, "localhost");
        }

        // Forward the reply to the client
        client_socket->write(received_reply_data); // @TODO @SpeedUp we have to check how this is implemented, does Qt do some intermediate copies before writting on the OS socket?
        client_socket->waitForBytesWritten(-1);
    }
    else {
        qDebug() << "CDN connection failed!";
    }

    // @TODO @SpeedUp reopening sockets for everysingle request is really slow
    client_socket->close();
    delete client_socket;  // @Warning From the doc: The socket is created as a child of the server, which means that it is automatically deleted when the QTcpServer object is destroyed. It is still a good idea to delete the object explicitly when you are done with it, to avoid wasting memory.

    m_cdn_socket->close();
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
void Server::read_everything(QTcpSocket* socket, QByteArray& request, Server::HTTP_Header& header)
{
    int from = 0;
    int header_size = 0;
    constexpr int header_delimiter_length = sizeof("\r\n\r\n") - 1; // @Warning -1 because there is a '\0'

    while ((header_size = request.indexOf("\r\n\r\n", from)) == -1)
    {
        from = std::max(0, request.size() - header_delimiter_length);  // @Warning - header_delimiter_length be sure that CLRFCLRF can be found if truncated between two chunk of data, else we can get an infinite loop
        socket->waitForReadyRead(-1);
        request += socket->readAll();
    }

    header_size += header_delimiter_length;

    // @Warning we send the size as the buffer can be bigger and already containing some data of the content
    parse_http_header(request, header_size, header);

    while (request.size() - header_size < header.content_length) {
        socket->waitForReadyRead(-1);
        request += socket->readAll();
    }
}

void Server::parse_http_header(const QByteArray& http_header, int header_size, Server::HTTP_Header& header)
{
    // @TODO This can be much more improved
    // We can use a proper parser with a Tokenizer and then
    // build the AST
    // At least we use QStringRef here to avoid allocations and copies

    QVector<QStringRef> slices;

    header.original_data = http_header.mid(0, header_size);   // @Speed by doing this we will parse only what we want, nothing more

    slices = header.original_data.splitRef("\r\n");
    for (const QStringRef& slice : slices) {
        QVector<QStringRef> pair_variable_value;

        pair_variable_value = slice.split(':');
        if (pair_variable_value.size() == 2)
        {
            if (pair_variable_value[0] == "Content-Length") {
                header.content_length = pair_variable_value[1].toInt(); // @TODO normally we should retrieve and check the ok parameter
            } else if (pair_variable_value[0] == "Host") {
                header.host = pair_variable_value[1].trimmed(); // @Warning we have to trim the string as it can have spaces before the value
            }
        }
    }

    slices = header.original_data.splitRef(' ');
    if (slices.size() >= 2
        && slices[0] == "GET") {
        header.url = slices[1];
    }
}
