# VirtualHere Qt 0.1.1

[English](README.md) | [Deutsch](README.de.md) | [Русский](README.ru.md)

**Inoffizielles Qt-6-Frontend für den VirtualHere USB Client als systemd-Dienst unter Linux.**

Unabhängiges Projekt, nicht mit VirtualHere verbunden oder von VirtualHere unterstützt.

> [!IMPORTANT]
> **Veröffentlichungsstatus — Genehmigung ausstehend**
>
> Die erste öffentliche Version mit dem originalen VirtualHere USB Console Client und Anwendungssymbol wird erst nach ausdrücklicher Genehmigung durch den Originalentwickler veröffentlicht. Bis dahin dient dieses Repository der Projektvorstellung und Dokumentation zur Prüfung. Das Installationspaket bleibt unveröffentlicht.

![“VirtualHere Qt Hauptfenster und Systemleiste”](preview.png)

## Funktionen

- USB-Server und Geräte anzeigen, verbinden und trennen.
- KDE-Plasma-Systemleiste, Autostart und Dienststatus.
- Grafische Oberfläche ohne Root-Rechte; Kommunikation über die offizielle `-t` API.
- Auto-Use für ausgewählte Geräte; Quit beendet nur die Oberfläche.
- English, Deutsch, Русский; Standardsprache Englisch.

## Plattform

Arch Linux x86_64, KDE Plasma, Wayland, Qt 6 (Paket gebaut mit Qt 6.12), qt6-wayland und systemd. Andere Umgebungen sind nicht getestet.

## Installation und Aktualisierung

Das Installationspaket ist noch nicht öffentlich verfügbar. Vor einem Update Quit im Tray wählen, danach VirtualHere im Plasma-Menü öffnen.

```bash
sudo pacman -U ./virtualhere-0.1.1-1-x86_64.pkg.tar.zst
```

Bei laufendem Dienst verbindet sich die Oberfläche automatisch. Auf einem neuen System:

```bash
sudo systemctl enable --now virtualhere.service
```

## Quellcode bauen

Sobald das Quellarchiv verfügbar ist, entpacken und im Verzeichnis mit PKGBUILD als normaler Benutzer ausführen:

```bash
sudo pacman -S --needed base-devel
makepkg -si
```

## Dateistruktur

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

Der Dienst speichert Verbindungseinstellungen und Auto-Use in `/var/lib/virtualhere/vhclient.conf`. Sprache und Fenstergeometrie: `~/.config/virtualhere/settings.ini`. Autostart: `~/.config/autostart/virtualhere.desktop`.

## Lizenz und Drittanbieter

Der Daemon `vhclientx86_64` ist der unveränderte Originalclient des Herstellers. Dieses Projekt stellt die Qt-Oberfläche, Arch-Paketierung und systemd-Integration bereit. 0BSD gilt nur für den eigenen Frontend-Code; Originalclient und Symbol unterliegen ihren jeweiligen Lizenzbedingungen. Die Weiterverbreitung wird noch geprüft.

[Offizielle VirtualHere-Website](https://www.virtualhere.com/)
