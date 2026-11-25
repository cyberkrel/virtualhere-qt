# Source code — VirtualHere Qt 0.1.1

VirtualHere Qt is an unofficial Qt 6 frontend for the VirtualHere USB Console Client on Arch Linux and KDE Plasma. It communicates with the original client through its official command interface.

This repository provides the frontend source code, translations, and Linux packaging configuration for public review.

The original VirtualHere executable and application icon are not included. The first release containing these components will only be published after permission from the original developer has been obtained.

## Build on Arch Linux

Obtain the two original files from the official links in [third-party components](assets/README.md) and save them under `assets/` using the specified filenames. The build checks their SHA-256 hashes. If upstream has updated the files, obtain the matching version from the developer; do not bypass verification.

From the directory containing `PKGBUILD`, as a regular user:

```bash
sudo pacman -S --needed base-devel
makepkg -si
```

This builds a local package only. Redistribution of the original components remains pending permission.

The frontend is licensed under [0BSD](LICENSE). See [NOTICE](NOTICE) for third-party attribution.

[Project overview](https://github.com/cyberkrel/virtualhere-qt) · [Official VirtualHere website](https://www.virtualhere.com/)
