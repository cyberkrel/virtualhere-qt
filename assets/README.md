# Original VirtualHere Components

This directory is reserved for the original VirtualHere components required to build VirtualHere Qt 0.1.1.

## VirtualHere USB Console Client

- Version: 6.0.3
- Release date: 1 October 2026
- Architecture: Linux x86_64
- Filename: `vhclientx86_64`
- Modification: None

**SHA-1**

`a5e9a489c811357a6eeda3213c172b9e80beab20`

**SHA-256**

`d66d41a12e1137ce3adec1f851987a2bfb3cc62ca96a280646abcd6d9c4ec554`

Official download:

https://www.virtualhere.com/sites/default/files/usbclient/vhclientx86_64

Official checksums:

https://www.virtualhere.com/sites/default/files/usbclient/SHA1SUM

The client's SHA-1 checksum was verified against the manufacturer's published checksum on 25 November 2026.

## Original Application Icon

The original VirtualHere application icon is used with permission from the original developer. A monochrome adaptation is used for KDE Plasma system tray integration.

## Preparing the Build

Place both original files in this directory:

```
assets/
├── README.md
├── vhclientx86_64
└── virtualhere.png
```

The original icon must be obtained separately if it is not already available locally.

From the project root, verify the source files:

```bash
makepkg --verifysource
```

If the upstream files have changed, obtain the matching versions or deliberately update the package and its checksums. Do not bypass checksum verification.

## Licensing

The original components are not covered by the frontend's 0BSD license.

Redistribution permission for the Arch Linux installation package and permission to use the application graphics were granted on 25 November 2026.

See [NOTICE](../NOTICE) for details.

Official website: https://www.virtualhere.com/
