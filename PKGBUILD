pkgname=virtualhere
pkgver=0.1.1
pkgrel=1
pkgdesc='Qt 6 tray interface for the VirtualHere USB client'
arch=('x86_64')
url='https://github.com/cyberkrel/virtualhere-qt'
license=('0BSD' 'LicenseRef-VirtualHere')
depends=('gcc-libs' 'glibc' 'qt6-wayland' 'systemd' 'qt6-base>=6.5')
makedepends=('cmake' 'ninja' 'qt6-base' 'qt6-tools')
options=('!strip' '!debug')
source=(
  'CMakeLists.txt'
  'LICENSE'
  'NOTICE'
  'README.md'
  'README.de.md'
  'README.ru.md'
  'SHA256SUMS'
)
sha256sums=(
  '688c9174f004526c56e922335eb96ee69f7ef0682a7e3d8554fa6ddcd80399b4'
  '11e723e90c166ec62cd544b169ef857f66dc11c61325f678952e66acbdc84f90'
  'fdfc27e9fc17555b2d23e732b5fcf4373cfddbb452c349a521934529c5037f39'
  '30df457d9e0e5d87be6993afa78644fe1f4056f73a442af72ac55bc75cb3b3fc'
  '7c2012259205d2d349372c8a06c8448caa2224a502d22ae3a6439f9bb3dbdede'
  '2bf93abeeb354dc825b264b98da5d44f2ec258abb5cde0d4679b77442bf2e34e'
  'ed18f9b927d5a287c75f89090cc17b517088b369cd2c248067361406f6972c18'
)

verify() {
  cd "$startdir"
  sha256sum --check SHA256SUMS
}

prepare() {
  local project="$srcdir/$pkgname-$pkgver"
  mkdir -p "$project"
  cp -a "$startdir/frontend" "$startdir/i18n" "$startdir/packaging" "$startdir/assets" "$project/"
  for file in CMakeLists.txt LICENSE NOTICE README.md README.de.md README.ru.md; do
    install -Dm644 "$srcdir/$file" "$project/$file"
  done
}

build() {
  cmake -S "$srcdir/$pkgname-$pkgver" -B "$srcdir/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build "$srcdir/build"
}

package() {
  local qt_version
  qt_version=$(/usr/lib/qt6/bin/qtpaths --qt-version)
  depends=("${depends[@]/qt6-base>=6.5/qt6-base>=${qt_version:-6.5}}")
  local project="$srcdir/$pkgname-$pkgver"
  DESTDIR="$pkgdir" cmake --install "$srcdir/build"
  strip --strip-unneeded "$pkgdir/usr/bin/virtualhere"
  install -Dm755 "$project/assets/vhclientx86_64" "$pkgdir/usr/lib/virtualhere/vhclientx86_64"
  install -Dm644 "$project/packaging/virtualhere.service" "$pkgdir/usr/lib/systemd/system/virtualhere.service"
  install -Dm644 "$project/packaging/virtualhere.desktop" "$pkgdir/usr/share/applications/virtualhere.desktop"
  install -Dm644 "$project/assets/virtualhere.png" "$pkgdir/usr/share/icons/hicolor/128x128/apps/virtualhere.png"
  for file in README.md README.de.md README.ru.md; do
    install -Dm644 "$project/$file" "$pkgdir/usr/share/doc/virtualhere/$file"
  done
  install -Dm644 "$project/LICENSE" "$pkgdir/usr/share/licenses/virtualhere/LICENSE"
  install -Dm644 "$project/NOTICE" "$pkgdir/usr/share/licenses/virtualhere/NOTICE"
}

