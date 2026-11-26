# VirtualHere Qt 0.1.1

[English](README.md) | [Deutsch](README.de.md) | [Русский](README.ru.md)

**Неофициальный интерфейс Qt 6 для VirtualHere USB Client, работающего как служба systemd в Linux.**

Этот независимый проект не связан с VirtualHere, не имеет официального одобрения и не поддерживается производителем.

> [!NOTE]
> **Знакомство с VirtualHere — USB over IP**
>
> VirtualHere позволяет использовать USB-устройства через локальную сеть, Интернет или облако так, словно они подключены непосредственно к компьютеру. USB Server предоставляет доступ к физическим устройствам, а USB Client делает их доступными существующим приложениям и драйверам. Оригинальный продукт поддерживает широкий спектр серверных платформ и клиенты для Windows, macOS и Linux.
>
> **[Официальный сайт VirtualHere](https://www.virtualhere.com/)** · [Программное обеспечение USB Server](https://www.virtualhere.com/usb_server_software) · [Программное обеспечение USB Client](https://www.virtualhere.com/usb_client_software)
>
> **Релиз v0.1.1**
>
> 25 ноября 2026 года разработчик оригинальной программы предоставил разрешение на распространение неизменённого клиента Linux x86_64 в составе пакета Arch Linux, использование оригинальной иконки приложения и её монохромной версии для системного трея, а также использование названия VirtualHere Qt.

![VirtualHere Qt — основное окно и системный трей](preview.png)

## Возможности

- Просмотр USB-серверов и устройств, подключение и отключение устройств.
- Интеграция с системным треем KDE Plasma, автозапуск и состояние службы.
- Графический интерфейс без root-прав; использование официального API `-t`.
- Auto-Use для выбранных устройств; Quit завершает только интерфейс.
- Английский, немецкий и русский языки; по умолчанию английский.

## Платформа и совместимость

Arch Linux x86_64, KDE Plasma, Wayland, Qt 6 (пакет собран под Qt 6.12), qt6-wayland и systemd. Другие окружения не тестировались.

## Установка и обновление

Скачайте установочный пакет в разделе [Releases](https://github.com/cyberkrel/virtualhere-qt/releases). Перед обновлением выберите **Quit** в меню трея. После установки снова запустите **VirtualHere** из меню приложений Plasma.

```bash
sudo pacman -U ./virtualhere-0.1.1-1-x86_64.pkg.tar.zst
```

Установка не активирует и не запускает службу systemd автоматически. Если служба уже работает, интерфейс подключится автоматически. При первой установке активируйте и запустите службу вручную:

```bash
sudo systemctl enable --now virtualhere.service
```

Проверьте состояние службы командой `systemctl status virtualhere.service`. Запускайте VirtualHere из меню приложений от обычного пользователя.

## Оригинальный клиент VirtualHere

Пакет содержит неизменённый VirtualHere USB Console Client **6.0.3** для Linux x86_64 (1 октября 2026 года).

- SHA-1: `a5e9a489c811357a6eeda3213c172b9e80beab20`
- SHA-256: `d66d41a12e1137ce3adec1f851987a2bfb3cc62ca96a280646abcd6d9c4ec554`

Контрольная сумма SHA-1 была сверена с опубликованной производителем суммой 25 ноября 2026 года. См. [официальные загрузки](https://www.virtualhere.com/usb_client_software) и [официальные контрольные суммы](https://www.virtualhere.com/sites/default/files/usbclient/SHA1SUM).

Для ручного обновления клиента скачайте новый файл `vhclientx86_64` у производителя и сверьте его контрольную сумму с соответствующей официальной суммой. Затем остановите службу, сохраните резервную копию существующего исполняемого файла вне каталога пакета, установите проверенный файл и снова запустите службу:

```bash
sudo systemctl stop virtualhere.service
sudo cp -a /usr/lib/virtualhere/vhclientx86_64 /root/vhclientx86_64.backup
sudo install -m755 vhclientx86_64 /usr/lib/virtualhere/vhclientx86_64
sudo systemctl start virtualhere.service
```

Остановка службы прерывает USB-подключения. Ручная замена изменяет файл, управляемый Pacman, который может быть перезаписан при обновлении пакета. Совместимость с более новыми версиями оригинального клиента не гарантируется; предпочтительна пересборка пакета.

## Сборка из исходного кода

Клонируйте репозиторий или распакуйте архив исходников. Получите оригинальные компоненты согласно [assets/README.md](assets/README.md), затем выполните следующие команды в каталоге с PKGBUILD от обычного пользователя:

```bash
sudo pacman -S --needed base-devel
makepkg --verifysource
makepkg -si
```

## Структура пакета

```
/usr/
├── bin/virtualhere
├── lib/
│   ├── virtualhere/vhclientx86_64
│   └── systemd/system/virtualhere.service
└── share/
    ├── applications/virtualhere.desktop
    ├── icons/hicolor/128x128/apps/virtualhere.png
    ├── doc/virtualhere/
    │   ├── README.md
    │   ├── README.de.md
    │   └── README.ru.md
    └── licenses/virtualhere/
        ├── LICENSE
        └── NOTICE
```

Служба хранит настройки подключения и Auto-Use в `/var/lib/virtualhere/vhclient.conf`. Язык и геометрия окна: `~/.config/virtualhere/settings.ini`. Автозапуск: `~/.config/autostart/virtualhere.desktop`.

## Лицензия и сторонние компоненты

Демон `vhclientx86_64` — неизменённый оригинальный клиент VirtualHere. Наш проект предоставляет интерфейс Qt, упаковку для Arch Linux и интеграцию systemd. Лицензия 0BSD распространяется только на собственный исходный код интерфейса. Оригинальный клиент и иконка подчиняются своим условиям лицензирования. Разрешение на распространение в составе пакета Arch Linux, использование графики и названия проекта предоставлено 25 ноября 2026 года. См. [NOTICE](NOTICE).

[Официальный сайт VirtualHere](https://www.virtualhere.com/)
