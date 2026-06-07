#ifndef UDP_IPC_SERVER_H
#define UDP_IPC_SERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonValue>

class UdpIpcServer : public QObject {
    Q_OBJECT
public:
    explicit UdpIpcServer(QObject *appSettings, quint16 port = 55432, QObject *parent = nullptr);

private slots:
    void onReadyRead();

private:
    QUdpSocket m_socket;
    QObject *m_appSettings;

    void processMessage(const QByteArray &data);
    void applyObject(const QJsonObject &object);
    QVariant jsonValueToVariant(const QJsonValue &value) const;
};

#endif // UDP_IPC_SERVER_H
