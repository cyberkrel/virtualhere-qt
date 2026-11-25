#include "client.h"

#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <utility>

namespace {

QString field(const QXmlStreamAttributes &attributes, const char *key)
{
    return attributes.value(QLatin1StringView(key)).toString();
}

int usbNumber(QStringView value)
{
    value = value.trimmed();
    return value.toInt(nullptr, value.startsWith(u"0x", Qt::CaseInsensitive) ? 16 : 10);
}

DeviceKind deviceKind(const QXmlStreamAttributes &attributes, QStringView vendorId,
                      QStringView productId)
{
    // The Samsung M2070 exposes printing on its second USB interface.
    if (usbNumber(vendorId) == 0x04e8 && usbNumber(productId) == 0x3469)
        return DeviceKind::Printer;

    switch (usbNumber(attributes.value(u"firstInterfaceClass"))) {
    case 0x07: return DeviceKind::Printer;
    case 0x08: return DeviceKind::MassStorage;
    case 0x0e: return DeviceKind::Webcam;
    default: return DeviceKind::Generic;
    }
}

Device readDevice(const QXmlStreamAttributes &attributes, const QString &host,
                  const QString &serverSerial)
{
    Device device;
    const QString port = field(attributes, "address").trimmed();
    static const QRegularExpression validHost(QStringLiteral(R"(^[^\s,\x00-\x1f]+$)"));
    static const QRegularExpression validPort(QStringLiteral("^[0-9]+$"));
    if (validHost.match(host).hasMatch() && validPort.match(port).hasMatch())
        device.address = host + QLatin1Char('.') + port;

    const QString vendorId = field(attributes, "idVendor");
    const QString productId = field(attributes, "idProduct");
    device.identity = serverSerial + QLatin1Char('/') + vendorId + QLatin1Char('/') + productId
                      + QLatin1Char('/') + field(attributes, "deviceSerial");
    device.name = field(attributes, "nickname").trimmed();
    if (device.name.isEmpty())
        device.name = field(attributes, "product").trimmed();
    if (device.name.isEmpty())
        device.name = QStringLiteral("USB %1:%2").arg(vendorId, productId);
    device.autoUse = field(attributes, "autoUse");
    device.owner = field(attributes, "boundClientHostname");
    device.kind = deviceKind(attributes, vendorId, productId);

    const int state = field(attributes, "state").toInt();
    const QString own = field(attributes, "connectionUUID");
    const QString bound = field(attributes, "boundConnectionUUID");
    if (state == 3) {
        const bool someoneElse = !own.isEmpty() && !bound.isEmpty() && own != bound;
        device.status = someoneElse ? DeviceStatus::Busy : DeviceStatus::Connected;
    } else if (state == 1) {
        device.status = DeviceStatus::Available;
    } else if (state == 4 || !device.owner.isEmpty()) {
        device.status = DeviceStatus::Busy;
    }
    return device;
}

Server readServer(QXmlStreamReader &reader)
{
    QXmlStreamAttributes connection;
    QList<QXmlStreamAttributes> devices;
    while (reader.readNextStartElement()) {
        if (reader.name() == u"connection")
            connection = reader.attributes();
        else if (reader.name() == u"device")
            devices.append(reader.attributes());
        reader.skipCurrentElement();
    }

    QString host = field(connection, "hostname").trimmed();
    if (host.isEmpty())
        host = field(connection, "host").trimmed();
    if (host.isEmpty())
        host = field(connection, "ip").trimmed();
    QString name = field(connection, "serverName").trimmed();
    const QString serial = field(connection, "serverSerial");
    if (name.isEmpty())
        name = host.isEmpty() ? serial : host;
    const QString port = field(connection, "port");
    Server server{name, port.isEmpty() ? host : host + QLatin1Char(':') + port, {}};
    for (const auto &attributes : std::as_const(devices))
        server.devices.append(readDevice(attributes, host, serial));
    return server;
}

QString autoUseCommand(const Device &device)
{
    if (device.autoUse == u"devicePort")
        return QStringLiteral("AUTO USE DEVICE PORT,") + device.address;
    if (device.autoUse == u"device")
        return QStringLiteral("AUTO USE DEVICE,") + device.address;
    if (device.autoUse == u"port")
        return QStringLiteral("AUTO USE PORT,") + device.address;
    return {};
}

bool hasAutoUse(const Device &device)
{
    return !autoUseCommand(device).isEmpty() || device.autoUse == u"all" || device.autoUse == u"server";
}

const Device *matchingDevice(const ClientState &state, const Device &requested)
{
    const Device *device = state.find(requested.address);
    return device && device->identity == requested.identity ? device : nullptr;
}

} // namespace

const Device *ClientState::find(const QString &address) const
{
    if (address.isEmpty())
        return nullptr;
    const Device *match = nullptr;
    for (const Server &server : servers) {
        for (const Device &device : server.devices) {
            if (device.address != address)
                continue;
            if (match)
                return nullptr;
            match = &device;
        }
    }
    return match;
}

VirtualHereClient::VirtualHereClient(QString executable) : executable(std::move(executable)) {}

QString VirtualHereClient::defaultExecutable()
{
    const QString installed = QStandardPaths::findExecutable(
        QStringLiteral("vhclientx86_64"), {QStringLiteral("/usr/local/bin"), QStringLiteral("/usr/bin"),
                                          QStringLiteral("/usr/local/sbin"), QStringLiteral("/usr/sbin")});
    return installed.isEmpty() ? QStringLiteral("/usr/lib/virtualhere/vhclientx86_64") : installed;
}

ClientState VirtualHereClient::parseState(const QByteArray &xml)
{
    ClientState result;
    QXmlStreamReader reader(xml);
    if (!reader.readNextStartElement() || reader.name() != u"state") {
        result.detail = QString::fromUtf8(xml).trimmed().left(300);
        return result;
    }
    while (reader.readNextStartElement()) {
        if (reader.name() == u"server")
            result.servers.append(readServer(reader));
        else
            reader.skipCurrentElement();
    }
    while (!reader.atEnd())
        reader.readNext();
    if (reader.hasError())
        return {false, {}, reader.errorString()};
    result.online = true;
    return result;
}

VirtualHereClient::CommandResult VirtualHereClient::execute(const QString &command) const
{
    QProcess process;
    process.start(executable, {QStringLiteral("-t"), command});
    if (!process.waitForStarted(1500))
        return {false, {}, process.errorString()};
    if (process.state() != QProcess::NotRunning && !process.waitForFinished(6500)
        && process.state() != QProcess::NotRunning) {
        const QString detail = process.errorString();
        process.kill();
        process.waitForFinished(500);
        return {false, {}, detail};
    }
    QByteArray output = process.readAllStandardOutput().trimmed();
    QString detail = QString::fromUtf8(output.isEmpty() ? process.readAllStandardError() : output)
                         .simplified().left(300);
    const bool succeeded = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    return {succeeded, std::move(output), std::move(detail)};
}

ClientState VirtualHereClient::state() const
{
    const auto result = execute(QStringLiteral("GET CLIENT STATE"));
    return result.succeeded ? parseState(result.output) : ClientState{false, {}, result.detail};
}

ClientResult VirtualHereClient::changeConnection(const Device &requested, bool connect) const
{
    ClientState current = state();
    if (!current.online)
        return {current, tr("The VirtualHere service is unavailable."), current.detail};
    const Device *device = matchingDevice(current, requested);
    if (!device)
        return {current, tr("The device address changed or is ambiguous."), {}};

    if (connect) {
        if (device->status == DeviceStatus::Available) {
            const auto result = execute(QStringLiteral("USE,") + device->address);
            current = state();
            device = matchingDevice(current, requested);
            if (!device || device->status != DeviceStatus::Connected)
                return {current, tr("Could not connect the device."), result.detail};
        }
        if (device->status != DeviceStatus::Connected)
            return {current, tr("The device is no longer available."), {}};
        if (hasAutoUse(*device))
            return {current, {}, {}};
        if (device->autoUse != u"not-set")
            return {current, tr("Connected, but the client did not report an Auto-Use setting."), {}};

        const auto result = execute(QStringLiteral("AUTO USE DEVICE PORT,") + device->address);
        current = state();
        device = matchingDevice(current, requested);
        if (!device || !hasAutoUse(*device))
            return {current, tr("Connected, but Auto-Use could not be saved."), result.detail};
        return {current, {}, {}};
    }

    if (device->status != DeviceStatus::Connected)
        return {current, {}, {}};
    if (device->autoUse == u"all" || device->autoUse == u"server")
        return {current, tr("Turn off the server-wide Auto-Use rule before disconnecting this device."), {}};

    const QString rule = autoUseCommand(*device);
    if (!rule.isEmpty()) {
        const auto result = execute(rule);
        current = state();
        if (!current.online)
            return {current, tr("Could not verify that Auto-Use was disabled."), result.detail};
        device = matchingDevice(current, requested);
        if (!device)
            return {current, {}, {}};
        if (device->autoUse != u"not-set")
            return {current, tr("Could not disable Auto-Use for this device."), result.detail};
        if (device->status != DeviceStatus::Connected)
            return {current, {}, {}};
    }

    const auto result = execute(QStringLiteral("STOP USING,") + device->address);
    current = state();
    device = matchingDevice(current, requested);
    if (!current.online || (device && device->status == DeviceStatus::Connected))
        return {current, tr("Could not disconnect the device."), result.detail};
    return {current, {}, {}};
}
