pkgname=dwmstatus-git
_pkgname=dwmstatus
pkgver=0.1
pkgrel=1

pkgdesc="A status bar for dwm written in C"
url="http://github.com/albanb/dwmstatus.git"
arch=('any')
license=('MIT')
depends=('libx11')
makedepends=('git')
provides=('dwmstatus')
source=("$_pkgname::gitp://github.com/albanb/dwmstatus.git")
sha256sums=('SKIP')

pkgver() {
  cd "${srcdir}/$_pkgname"
  echo "0.1"
}

build() {
  cd "${srdir}/$_pkgname"
  make
}

package() {
  cd "${srcdir}/$_pkgname"
  make PREFIX=/usr DESTDIR="${pkgdir}" install
}
