#include "window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("VirtualHere"));
    application.setApplicationVersion(QStringLiteral("0.1.1"));
    application.setDesktopFileName(QStringLiteral("virtualhere"));
    application.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption tray(QStringLiteral("tray"), QStringLiteral("Start hidden in the tray."));
    const QCommandLineOption backend(QStringLiteral("client"),
        QStringLiteral("Use a specific VirtualHere console client."), QStringLiteral("path"));
    parser.addOptions({tray, backend});
    parser.process(application);

    QDBusConnection bus = QDBusConnection::sessionBus();
    const QString service = QStringLiteral("local.virtualhere.Gui");
    const QString path = QStringLiteral("/local/virtualhere/Gui");
    const bool inTray = parser.isSet(tray);
    if (bus.isConnected() && !bus.registerService(service)) {
        if (!inTray) {
            auto message = QDBusMessage::createMethodCall(service, path, service, QStringLiteral("openWindow"));
            bus.call(message, QDBus::Block, 1500);
        }
        return 0;
    }

    const QString executable = parser.isSet(backend) ? QFileInfo(parser.value(backend)).absoluteFilePath()
                                                    : VirtualHereClient::defaultExecutable();
    Window window{VirtualHereClient(executable)};
    if (bus.isConnected())
        bus.registerObject(path, &window, QDBusConnection::ExportScriptableSlots);
    window.start(inTray);
    return application.exec();
}
