#pragma once

#include <QCoreApplication>
#include <QList>
#include <QString>

enum class DeviceStatus { Available, Connected, Busy, Unavailable };
enum class DeviceKind { Generic, Printer, MassStorage, Webcam };

struct Device {
    QString address;
    QString identity;
    QString name;
    QString autoUse;
    QString owner;
    DeviceKind kind = DeviceKind::Generic;
    DeviceStatus status = DeviceStatus::Unavailable;
    bool operator==(const Device &) const = default;
};

struct Server {
    QString name;
    QString endpoint;
    QList<Device> devices;
    bool operator==(const Server &) const = default;
};

struct ClientState {
    bool online = false;
    QList<Server> servers;
    QString detail;

    const Device *find(const QString &address) const;
    bool operator==(const ClientState &) const = default;
};

struct ClientResult {
    ClientState state;
    QString error;
    QString detail;
};

class VirtualHereClient {
    Q_DECLARE_TR_FUNCTIONS(VirtualHereClient)

public:
    explicit VirtualHereClient(QString executable);
    static QString defaultExecutable();
    static ClientState parseState(const QByteArray &xml);
    ClientState state() const;
    ClientResult changeConnection(const Device &requested, bool connect) const;

private:
    struct CommandResult {
        bool succeeded;
        QByteArray output;
        QString detail;
    };
    CommandResult execute(const QString &command) const;

    QString executable;
};
