# Contributor: elcerdo <georges.wbush@laposte.net>
maintainer=elcerdo
pkgname=supershooter
pkgver=0.2.3
pkgrel=2
pkgdesc="a shoot them up game with a lot of bullets"
arch=(i686 x86_64)
url="http://sd-12155.dedibox.fr:5001/SuperShooter"
license=('GPL')
depends=(sdl sdl_mixer sdl_image boost)
makedepends=(cmake)
source=(http://github.com/elcerdo/${pkgname}/tarball/${pkgver})
md5sums=('e1b9c38838d9620f43dacf5ca59c0c04')

build() {
  cd "${srcdir}/$(ls ${srcdir} | grep "elcerdo")"

  cmake . \
  -DCMAKE_INSTALL_PREFIX:FILEPATH=/usr \
  -DCMAKE_BUILD_TYPE:STRING=Release

  make || return 1
  make DESTDIR="$pkgdir/" install
}

# vim:set ts=2 sw=2 et:
