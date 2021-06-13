#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>

/**
  * Creates the TCP socket and manages client connections
  */
// TCP port used to listen for connections
#define STKPCONNECT_PORT 40303
// Network protocol, currently always 1
#define STKPCONNECT_PROTOCOL 1
// Feature revision, every time we add a new feature or bug fix, this should be incremented so that clients can know how old the plugin is
#define STKPCONNECT_VERSION 2020

#define STKPCONNECT_STRINGIFY(s) __STKPCONNECT_STRINGIFY(s)
#define __STKPCONNECT_STRINGIFY(s) #s

#define STKPCONNECT_PORT_STR STKPCONNECT_STRINGIFY(STKPCONNECT_PORT)
#define STKPCONNECT_PROTOCOL_STR STKPCONNECT_STRINGIFY(STKPCONNECT_PROTOCOL)
#define STKPCONNECT_VERSION_STR STKPCONNECT_STRINGIFY(STKPCONNECT_VERSION)

class TcpClient;
class DataRefProvider;

class TcpServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)

public:
    TcpServer(QObject *parent=nullptr, DataRefProvider *refProvider=nullptr);
    ~TcpServer();
    int clientCount() const;
    void setDataRefProvider(DataRefProvider *refProvider);

signals:
    void setFlightLoopInterval(float newInterval);
    void clientCountChanged(int clientCount);

public slots:
    void clientConnected();
    void clientDiscoed(TcpClient *client);
    void disconnectClients(); // Call before destroying
    void stkpconnectWarning(QString message); // Send warning message to clients

private:
    QTcpServer server;
    QList<TcpClient *> m_clientConnections;
    DataRefProvider *m_refProvider;
};

#endif // TCPSERVER_H
