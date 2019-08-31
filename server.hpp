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
    QByteArray  read_entiere_header(QTcpSocket* socket); /// Synchronous method that wait completeness of the request
    QByteArray  read_entiere_reply(QTcpSocket* socket);

    struct HTTP_Header
    {
        HTTP_Header()
            : content_length(0)
        {
        }

        QString host;
        int     content_length;
    };

    HTTP_Header parse_http_header(const QString& http_header);

    QTcpServer* m_server;
    QTcpSocket* m_cdn_socket;
};
