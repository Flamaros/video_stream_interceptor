#pragma once    // @Warning supported by all compilers from years!!!

#include <QObject>
#include <QAbstractSocket>

// Forward declaration to keep the compilation time as lowest as possible
class QTcpServer;
class QSslSocket;

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
    QTcpServer* m_server;
    QSslSocket* m_cdn_socket;
};
