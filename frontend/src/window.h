#pragma once

#include "client.h"

#include <QFutureWatcher>
#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTranslator>
#include <optional>

class QLabel;
class QMenu;
class QPushButton;
class QTreeWidget;

class Window : public QMainWindow {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "local.virtualhere.Gui")

public:
    explicit Window(VirtualHereClient client, QWidget *parent = nullptr);
    void start(bool inTray);

public slots:
    Q_SCRIPTABLE void openWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    struct Operation {
        Device device;
        bool connect;
    };

    void setLanguage(const QString &code);
    void showError(const QString &message, const QString &detail = {});
    void saveGeometrySetting();
    void setAutostart(bool enabled);
    void refresh();
    void commandFinished();
    void requestChange(const QString &address);
    void render();
    void renderTree();
    void renderTray();
    void updateControls();
    QString selectedAddress() const;
    QString deviceStatus(const Device &device) const;

    VirtualHereClient client;
    ClientState snapshot;
    QSettings settings;
    QTranslator translator;
    QString language;
    QTimer timer;
    QFutureWatcher<ClientResult> watcher;
    std::optional<Operation> pending;
    bool initial = true;
    bool running = false;
    bool changing = false;
    QTreeWidget *tree = nullptr;
    QLabel *serviceText = nullptr;
    QLabel *serviceDot = nullptr;
    QPushButton *hideButton = nullptr;
    QPushButton *deviceButton = nullptr;
    QMenu *trayMenu = nullptr;
    QMenu *languageMenu = nullptr;
    QSystemTrayIcon tray;
};
