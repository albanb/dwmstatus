pkgname=dwmstatusi-git
_pkgname=dwmstatus
pkgver=0.1
pkgver(){
  cd $_pkgname
  git describe --tags | sed 's/-/./g'
}
pkgrel=1

pkgdesc="A status bar for dwm written in C"
#url="http://dwm.suckless.org"
arch=('i686' 'x86_64')
#license=('MIT')
#options=(zipman)
#depends=('libx11' 'libxinerama')
makedepends=('git')
#install=dwm.install
provides=('dwmstatus')
conflicts=('dwmstatus')
epoch=1
source=( dwm.desktop
    "$_pkgname::git+http://git.suckless.org/dwm#tag=6.0")

build() {
  cd $_pkgname
  make X11INC=/usr/include/X11 X11LIB=/usr/lib/X11
}

package() {
  make -C $_pkgname PREFIX=/usr DESTDIR=$pkgdir install
#  install -m644 -D $_pkgname/LICENSE $pkgdir/usr/share/licenses/$pkgname/LICENSE
#  install -m644 -D $_pkgname/README $pkgdir/usr/share/doc/$pkgname/README
#  install -m644 -D $srcdir/dwm.desktop $pkgdir/usr/share/xsessions/dwm.desktop
}

# vim:set ts=2 sw=2 et:
md5sums=('939f403a71b6e85261d09fc3412269ee'
         'SKIP')
