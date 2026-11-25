# VirtualHere Qt 0.1.1

[English](README.md) | [Deutsch](README.de.md) | [Русский](README.ru.md)

**An unofficial Qt 6 frontend for the VirtualHere USB Client running as a systemd service on Linux.**

This independent project is not affiliated with, endorsed by, or supported by VirtualHere.

## Features

- View USB servers and devices, connect and disconnect devices.
- KDE Plasma system tray integration, autostart and service status.
- No root privileges required for the graphical frontend; uses the official `-t` API.
- Auto-Use for selected devices; Quit closes only the frontend.
- English, German and Russian interface; English by default.

## Platform and Compatibility

Arch Linux x86_64, KDE Plasma, Wayland, Qt 6 (package built against Qt 6.12), qt6-wayland and systemd. Other environments have not been tested.

## Installation and Update

The installation package is not yet publicly available. Before updating, select **Quit** from the tray menu. After installation, launch **VirtualHere** again from the Plasma application menu.

```bash
sudo pacman -U ./virtualhere-0.1.1-1-x86_64.pkg.tar.zst
```

If the service is already running, the frontend connects automatically. On a fresh installation:

```bash
sudo systemctl enable --now virtualhere.service
```

## Building from Source

Once the source archive is published, extract it and run the following in the directory containing PKGBUILD as a regular user:

```bash
sudo pacman -S --needed base-devel
makepkg -si
```

## File Layout

```
/usr/
├── bin/virtualhere
├── lib/
│   ├── virtualhere/vhclientx86_64
│   └── systemd/system/virtualhere.service
└── share/
    ├── applications/virtualhere.desktop
    ├── icons/hicolor/128x128/apps/virtualhere.png
    ├── doc/virtualhere/README.ru.md
    └── licenses/virtualhere/
        ├── LICENSE
        └── NOTICE
```

The service stores connection settings and Auto-Use preferences in `/var/lib/virtualhere/vhclient.conf`. Language and window geometry: `~/.config/virtualhere/settings.ini`. Autostart: `~/.config/autostart/virtualhere.desktop`.

## License and Third-Party Components

The daemon `vhclientx86_64` is the original, unmodified client provided by VirtualHere. This project provides the Qt frontend, Arch Linux packaging and systemd integration. The 0BSD license applies only to the frontend's own source code. The original client and icon remain subject to their respective licensing terms. Redistribution rights are still under review.

[Official VirtualHere website](https://www.virtualhere.com/)
