#pragma once    // @Warning supported by all compilers from years!!!

#include <QObject>
#include <QAbstractSocket>

// Forward declaration to keep the compilation time as lowest as possible
class QSslSocket;
class QTcpServer;
class QTcpSocket;

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);
    ~Server();

signals:

public slots:
    void new_connection();

    void cdn_found();
    void cdn_connected();
    void cdn_connection_error(QAbstractSocket::SocketError socketError);

private:
    struct HTTP_Header
    {
        HTTP_Header()
            : content_length(0)
        {
        }

        QString     original_data;
        QStringRef  url;
        QStringRef  host;
        int         content_length;
    };

    void read_everything(QTcpSocket* socket, QByteArray& request, HTTP_Header& header); /// Synchronous method that wait completeness of the request
    void parse_http_header(const QByteArray& http_header, int header_size, HTTP_Header& header);

    QTcpServer* m_server;
    QTcpSocket* m_cdn_socket;
};
