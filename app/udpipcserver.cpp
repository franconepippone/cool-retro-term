#include "udpipcserver.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QNetworkDatagram>
#include <QDebug>

UdpIpcServer::UdpIpcServer(QObject *appSettings, quint16 port, QObject *parent)
    : QObject(parent)
    , m_appSettings(appSettings)
{
    if (!m_appSettings) {
        qWarning() << "UDP IPC: appSettings object is null";
        return;
    }

    if (!m_socket.bind(QHostAddress::LocalHost, port, QUdpSocket::ReuseAddressHint)) {
        qWarning() << "UDP IPC: failed to bind port" << port << ":" << m_socket.errorString();
        return;
    }

    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpIpcServer::onReadyRead);
    qDebug() << "UDP IPC: listening on" << port << "(localhost)";
}

void UdpIpcServer::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket.receiveDatagram();
        processMessage(datagram.data());
    }
}

void UdpIpcServer::processMessage(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        qWarning() << "UDP IPC: invalid JSON payload:" << QString::fromUtf8(data).trimmed();
        return;
    }

    QJsonObject rootObject = document.object();

    if (rootObject.contains("profile")) {
        QJsonValue profileValue = rootObject.value("profile");
        if (profileValue.isObject()) {
            applyObject(profileValue.toObject());
        } else if (profileValue.isString()) {
            if (!loadProfileByName(profileValue.toString())) {
                qWarning() << "UDP IPC: profile not found:" << profileValue.toString();
            }
        }
    }
    if (rootObject.contains("settings") && rootObject.value("settings").isObject()) {
        applyObject(rootObject.value("settings").toObject());
    }

    for (auto it = rootObject.constBegin(); it != rootObject.constEnd(); ++it) {
        if (it.key() == "profile" || it.key() == "settings")
            continue;

        QVariant value = jsonValueToVariant(it.value());
        if (value.isValid()) {
            if (!m_appSettings->setProperty(it.key().toLatin1(), value)) {
                qDebug() << "UDP IPC: ignored unknown property" << it.key();
            }
        }
    }
}

bool UdpIpcServer::loadProfileByName(const QString &profileName)
{
    if (!m_appSettings)
        return false;

    QVariant loadedResult;
    bool invoked = QMetaObject::invokeMethod(m_appSettings,
                                             "loadProfileByName",
                                             Q_RETURN_ARG(QVariant, loadedResult),
                                             Q_ARG(QVariant, profileName));
    if (invoked)
        return loadedResult.toBool();

    QVariant profileIndexResult;
    bool found = QMetaObject::invokeMethod(m_appSettings,
                                           "getProfileIndexByName",
                                           Q_RETURN_ARG(QVariant, profileIndexResult),
                                           Q_ARG(QVariant, profileName));
    if (found) {
        int profileIndex = profileIndexResult.toInt();
        if (profileIndex >= 0) {
            QVariant loadResult;
            bool loadInvoked = QMetaObject::invokeMethod(m_appSettings,
                                                          "loadProfile",
                                                          Q_RETURN_ARG(QVariant, loadResult),
                                                          Q_ARG(QVariant, profileIndex));
            return loadInvoked ? loadResult.toBool() : false;
        }
    }

    return false;
}

void UdpIpcServer::applyObject(const QJsonObject &object)
{
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        QVariant value = jsonValueToVariant(it.value());
        if (value.isValid()) {
            if (!m_appSettings->setProperty(it.key().toLatin1(), value)) {
                qDebug() << "UDP IPC: ignored unknown property" << it.key();
            }
        }
    }
}

QVariant UdpIpcServer::jsonValueToVariant(const QJsonValue &value) const
{
    switch (value.type()) {
    case QJsonValue::Bool:
        return value.toBool();
    case QJsonValue::Double:
        return value.toDouble();
    case QJsonValue::String:
        return value.toString();
    case QJsonValue::Array: {
        QVariantList list;
        for (const QJsonValue &entry : value.toArray())
            list << jsonValueToVariant(entry);
        return list;
    }
    default:
        return QVariant();
    }
}
