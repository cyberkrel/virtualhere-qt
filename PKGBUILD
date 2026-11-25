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
  'SOURCE-REVIEW.md'
  'SHA256SUMS'
)
sha256sums=(
  '688c9174f004526c56e922335eb96ee69f7ef0682a7e3d8554fa6ddcd80399b4'
  '11e723e90c166ec62cd544b169ef857f66dc11c61325f678952e66acbdc84f90'
  '10a86c285e5d42847f41fd7e0cbac1bd57db345500e6f162e60ad2b2bee07e6a'
  'e9f95809c1bfcf38869f3cd4493dc60b7d8ec0c922c4b7d876afc624a3184c06'
  '0f9cc5ed5f843e72ba00ec4ee947a6afba64df527f244b28de5eee33265e6d6f'
)

verify() {
  cd "$startdir"
  sha256sum --check SHA256SUMS
}

prepare() {
  local project="$srcdir/$pkgname-$pkgver"
  mkdir -p "$project"
  cp -a "$startdir/frontend" "$startdir/i18n" "$startdir/packaging" "$startdir/assets" "$project/"
  for file in CMakeLists.txt LICENSE NOTICE SOURCE-REVIEW.md; do
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
  install -Dm644 "$project/SOURCE-REVIEW.md" "$pkgdir/usr/share/doc/virtualhere/SOURCE-REVIEW.md"
  install -Dm644 "$project/LICENSE" "$pkgdir/usr/share/licenses/virtualhere/LICENSE"
  install -Dm644 "$project/NOTICE" "$pkgdir/usr/share/licenses/virtualhere/NOTICE"
}

