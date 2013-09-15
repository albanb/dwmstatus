pkgname=dwmstbar-git
_pkgname=dwmstbar
pkgver=0.1
pkgrel=1
pkgdesc="A status bar for dwm written in C"
url="http://github.com/albanb/dwmstatus.git"
arch=('any')
license=('none')
depends=('libx11')
makedepends=('git')
provides=('dwmstatus')
source=("$_pkgname::git://github.com/albanb/dwmstatus.git")
sha256sums=('SKIP')

pkgver() {
  cd "${srcdir}/$_pkgname"
  echo "0.1"
}

prepare() {
  cd "${srcdir}/$_pkgname"
  cp ../../dwmstbar.h .
}

build() {
  cd "${srcdir}/$_pkgname"
  make
}

package() {
  cd "${srcdir}/$_pkgname"
  make PREFIX=/usr DESTDIR="${pkgdir}" install
}
