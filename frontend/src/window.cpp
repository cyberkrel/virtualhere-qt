#include "window.h"

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSaveFile>
#include <QSet>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QtConcurrentRun>
#include <utility>

namespace {

const auto applicationName = QStringLiteral("VirtualHere");
const auto applicationIcon = QStringLiteral(":/icons/virtualhere.png");
constexpr int deviceRole = Qt::UserRole;

QString autostartFile()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
           + QStringLiteral("/autostart/virtualhere.desktop");
}

QPixmap statusDot(QColor color)
{
    QPixmap result(12, 12);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRectF(1, 1, 10, 10));
    return result;
}

QIcon trayIcon()
{
    QPixmap pixmap(applicationIcon);
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), Qt::white);
    painter.end();
    return QIcon(pixmap);
}

QIcon themedIcon(const QString &name, QStyle::StandardPixmap fallback)
{
    return QIcon::fromTheme(name, QApplication::style()->standardIcon(fallback));
}

QIcon usbDeviceIcon(const Device &device)
{
    switch (device.kind) {
    case DeviceKind::Printer:
        return themedIcon(QStringLiteral("printer"), QStyle::SP_FileDialogDetailedView);
    case DeviceKind::MassStorage:
        return themedIcon(QStringLiteral("drive-removable-media-usb"), QStyle::SP_DriveHDIcon);
    case DeviceKind::Webcam:
        return themedIcon(QStringLiteral("camera-web"), QStyle::SP_ComputerIcon);
    case DeviceKind::Generic:
        return themedIcon(QStringLiteral("device-notifier"), QStyle::SP_DriveHDIcon);
    }
    return {};
}

bool canChange(const Device *device)
{
    return device && (device->status == DeviceStatus::Available || device->status == DeviceStatus::Connected);
}

QString escapeMenuText(QString text)
{
    return text.replace(QStringLiteral("&"), QStringLiteral("&&"));
}

} // namespace

Window::Window(VirtualHereClient client, QWidget *parent)
    : QMainWindow(parent), client(std::move(client)),
      settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("virtualhere"),
               QStringLiteral("settings")), tray(this)
{
    setWindowTitle(applicationName);
    setWindowIcon(QIcon(applicationIcon));
    resize(550, 315);
    restoreGeometry(settings.value(QStringLiteral("window/geometry")).toByteArray());

    auto *contents = new QWidget(this);
    auto *body = new QVBoxLayout(contents);
    auto *status = new QHBoxLayout;
    serviceDot = new QLabel(contents);
    serviceText = new QLabel(contents);
    serviceText->setTextFormat(Qt::PlainText);
    status->addWidget(serviceDot);
    status->addWidget(serviceText, 1);
    body->addLayout(status);

    tree = new QTreeWidget(contents);
    tree->setObjectName(QStringLiteral("devices"));
    tree->setColumnCount(2);
    tree->setAlternatingRowColors(true);
    tree->setUniformRowHeights(true);
    tree->setAllColumnsShowFocus(true);
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    body->addWidget(tree, 1);

    auto *buttons = new QHBoxLayout;
    hideButton = new QPushButton(contents);
    deviceButton = new QPushButton(contents);
    deviceButton->setObjectName(QStringLiteral("deviceAction"));
    buttons->addWidget(hideButton);
    buttons->addStretch();
    buttons->addWidget(deviceButton);
    body->addLayout(buttons);
    setCentralWidget(contents);

    trayMenu = new QMenu(this);
    tray.setIcon(trayIcon());
    tray.setContextMenu(trayMenu);
    connect(&tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::Trigger)
            return;
        if (isVisible() && !isMinimized())
            close();
        else
            openWindow();
    });
    connect(hideButton, &QPushButton::clicked, this, &QWidget::close);
    connect(deviceButton, &QPushButton::clicked, this, [this] { requestChange(selectedAddress()); });
    connect(tree, &QTreeWidget::itemSelectionChanged, this, &Window::updateControls);
    connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
        requestChange(item->data(0, deviceRole).toString());
    });
    connect(&timer, &QTimer::timeout, this, &Window::refresh);
    connect(&watcher, &QFutureWatcher<ClientResult>::finished, this, &Window::commandFinished);
    connect(qApp, &QApplication::aboutToQuit, this, &Window::saveGeometrySetting);

    setLanguage(settings.value(QStringLiteral("language"), QStringLiteral("en")).toString());
}

void Window::start(bool inTray)
{
    tray.show();
    if (!inTray)
        openWindow();
    timer.start(1800);
    refresh();
}

void Window::openWindow()
{
    setWindowState(windowState() & ~Qt::WindowMinimized);
    show();
    raise();
    activateWindow();
}

void Window::closeEvent(QCloseEvent *event)
{
    saveGeometrySetting();
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        hide();
        event->ignore();
    } else {
        event->accept();
        QCoreApplication::quit();
    }
}

void Window::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange && isMinimized()
        && QSystemTrayIcon::isSystemTrayAvailable()) {
        QTimer::singleShot(0, this, [this] {
            if (isMinimized())
                hide();
        });
    }
}

void Window::saveGeometrySetting()
{
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings.sync();
}

void Window::setLanguage(const QString &code)
{
    QCoreApplication::removeTranslator(&translator);
    language = QStringLiteral("en");
    if ((code == u"de" || code == u"ru")
        && translator.load(QStringLiteral(":/i18n/virtualhere_%1.qm").arg(code))) {
        QCoreApplication::installTranslator(&translator);
        language = code;
    }
    settings.setValue(QStringLiteral("language"), language);
    settings.sync();
    render();
}

void Window::setAutostart(bool enabled)
{
    const QString path = autostartFile();
    bool success;
    if (enabled) {
        const QByteArray entry = "[Desktop Entry]\nType=Application\nName=VirtualHere\n"
                                 "Exec=/usr/bin/virtualhere --tray\n"
                                 "Icon=virtualhere\nTerminal=false\n";
        QSaveFile file(path);
        success = QDir().mkpath(QFileInfo(path).absolutePath()) && file.open(QIODevice::WriteOnly)
                  && file.write(entry) == entry.size() && file.commit();
    } else {
        success = !QFile::exists(path) || QFile::remove(path);
    }
    if (!success)
        showError(tr("Could not update the login startup setting."));
    QTimer::singleShot(0, this, &Window::renderTray);
}

void Window::refresh()
{
    if (running)
        return;
    running = true;
    if (pending) {
        const Operation operation = std::exchange(pending, std::nullopt).value();
        watcher.setFuture(QtConcurrent::run([client = client, operation] {
            return client.changeConnection(operation.device, operation.connect);
        }));
    } else {
        watcher.setFuture(QtConcurrent::run([client = client] {
            return ClientResult{client.state(), {}, {}};
        }));
    }
}

void Window::commandFinished()
{
    running = false;
    const ClientResult result = watcher.result();
    const bool update = initial || snapshot != result.state;
    initial = false;
    snapshot = result.state;
    changing = pending.has_value();
    if (update)
        render();
    else
        updateControls();
    if (!result.error.isEmpty())
        showError(result.error, result.detail);
    if (pending)
        refresh();
}

void Window::requestChange(const QString &address)
{
    const Device *device = snapshot.find(address);
    if (changing || !canChange(device))
        return;
    pending = Operation{*device, device->status == DeviceStatus::Available};
    changing = true;
    updateControls();
    refresh();
}

QString Window::selectedAddress() const
{
    const QTreeWidgetItem *item = tree->currentItem();
    return item ? item->data(0, deviceRole).toString() : QString{};
}

QString Window::deviceStatus(const Device &device) const
{
    switch (device.status) {
    case DeviceStatus::Available: return tr("Available");
    case DeviceStatus::Connected: return tr("Connected");
    case DeviceStatus::Busy: return tr("In use elsewhere");
    default: return tr("Unavailable");
    }
}

void Window::render()
{
    hideButton->setText(tr("Hide to Tray"));
    const QString status = initial ? tr("Checking service…")
        : snapshot.online ? tr("Service running") : tr("Service unavailable");
    serviceText->setText(status);
    serviceText->setToolTip(snapshot.detail);
    serviceDot->setPixmap(statusDot(snapshot.online ? QColor(39, 174, 96) : QColor(135, 142, 149)));
    tray.setToolTip(applicationName + QStringLiteral(" — ") + status);
    renderTree();
    renderTray();
    updateControls();
}

void Window::renderTree()
{
    const QSignalBlocker blocker(tree);
    const QString selected = selectedAddress();
    QSet<QString> collapsed;
    for (int index = 0; index < tree->topLevelItemCount(); ++index) {
        auto *item = tree->topLevelItem(index);
        if (!item->isExpanded())
            collapsed.insert(item->toolTip(0));
    }
    tree->clear();
    tree->setHeaderLabels({tr("Device"), tr("Status")});
    QTreeWidgetItem *selection = nullptr;
    QTreeWidgetItem *first = nullptr;
    for (const Server &server : std::as_const(snapshot.servers)) {
        auto *hub = new QTreeWidgetItem(tree, {server.name});
        QFont font = hub->font(0);
        font.setBold(true);
        hub->setFont(0, font);
        hub->setToolTip(0, server.endpoint);
        hub->setIcon(0, themedIcon(QStringLiteral("network-server"), QStyle::SP_DriveNetIcon));
        for (const Device &device : server.devices) {
            auto *item = new QTreeWidgetItem(hub, {device.name, deviceStatus(device)});
            item->setData(0, deviceRole, device.address);
            item->setIcon(0, usbDeviceIcon(device));
            const QColor color = device.status == DeviceStatus::Connected ? QColor(39, 174, 96)
                : device.status == DeviceStatus::Busy ? QColor(230, 146, 48) : QColor(135, 142, 149);
            item->setIcon(1, QIcon(statusDot(color)));
            item->setToolTip(1, device.owner);
            if (!first && canChange(snapshot.find(device.address)))
                first = item;
            if (!selected.isEmpty() && device.address == selected)
                selection = item;
        }
        hub->setExpanded(!collapsed.contains(server.endpoint));
    }
    if (snapshot.servers.isEmpty() && snapshot.online) {
        auto *empty = new QTreeWidgetItem(tree, {tr("Waiting for a USB server…")});
        empty->setFlags(Qt::NoItemFlags);
    }
    tree->setCurrentItem(selection ? selection : first);
}

void Window::renderTray()
{
    delete languageMenu;
    trayMenu->clear();
    const int serverCount = snapshot.servers.size();
    if (!initial && (!snapshot.online || serverCount == 0)) {
        trayMenu->addAction(snapshot.online ? tr("Waiting for a USB server…")
                                           : tr("Service unavailable"))->setEnabled(false);
    }
    for (const Server &server : std::as_const(snapshot.servers)) {
        trayMenu->addSection(escapeMenuText(server.name));
        for (const Device &device : server.devices) {
            const QString name = escapeMenuText(device.name);
            const QString label = device.status == DeviceStatus::Connected ? tr("Disconnect %1").arg(name)
                : device.status == DeviceStatus::Available ? tr("Connect %1").arg(name)
                : tr("%1 (%2)").arg(name, deviceStatus(device));
            auto *action = trayMenu->addAction(usbDeviceIcon(device), label, this,
                                              [this, address = device.address] { requestChange(address); });
            action->setData(device.address);
            action->setEnabled(!changing && canChange(snapshot.find(device.address)));
        }
    }
    trayMenu->addSeparator();
    trayMenu->addAction(tr("Open Window"), this, &Window::openWindow);
    languageMenu = trayMenu->addMenu(tr("Language"));
    auto *selection = new QActionGroup(languageMenu);
    selection->setExclusive(true);
    const QList<QPair<QString, QString>> options{
        {QStringLiteral("en"), QStringLiteral("English")},
        {QStringLiteral("de"), QStringLiteral("Deutsch")},
        {QStringLiteral("ru"), QStringLiteral("Русский")}};
    for (const auto &[code, name] : options) {
        auto *action = languageMenu->addAction(name);
        selection->addAction(action);
        action->setCheckable(true);
        action->setChecked(language == code);
        connect(action, &QAction::triggered, this, [this, code] {
            QTimer::singleShot(0, this, [this, code] { setLanguage(code); });
        });
    }
    auto *autostart = trayMenu->addAction(tr("Start at Login"));
    autostart->setCheckable(true);
    autostart->setChecked(QFile::exists(autostartFile()));
    connect(autostart, &QAction::triggered, this, &Window::setAutostart);
    trayMenu->addSeparator();
    trayMenu->addAction(tr("Quit"), qApp, &QCoreApplication::quit);
}

void Window::updateControls()
{
    const Device *device = snapshot.find(selectedAddress());
    deviceButton->setText(changing ? tr("Working…")
                          : device && device->status == DeviceStatus::Connected ? tr("Disconnect")
                                                                               : tr("Connect"));
    deviceButton->setEnabled(!changing && canChange(device));
    for (QAction *action : trayMenu->actions()) {
        if (action->data().isValid())
            action->setEnabled(!changing && canChange(snapshot.find(action->data().toString())));
    }
}

void Window::showError(const QString &message, const QString &detail)
{
    const QString text = detail.isEmpty() ? message : message + QStringLiteral("\n\n") + detail;
    if (isVisible()) {
        auto *dialog = new QMessageBox(QMessageBox::Warning, applicationName, text, QMessageBox::Ok, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setTextFormat(Qt::PlainText);
        dialog->open();
    } else {
        tray.showMessage(applicationName, text, QSystemTrayIcon::Warning);
    }
}
