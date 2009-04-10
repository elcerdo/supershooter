# Contributor: elcerdo <georges.wbush@laposte.net>
maintainer=elcerdo
pkgname=supershooter
pkgver=0.2.2
pkgrel=1
pkgdesc="a shoot them up game with a lot of bullets"
arch=(x686 x86_64)
url="http://sd-12155.dedibox.fr:5001/SuperShooter"
license=('GPL')
depends=(sdl sdl_mixer sdl_image boost)
makedepends=(cmake)
source=(http://github.com/elcerdo/${pkgname}/tarball/${pkgver})
md5sums=('192f60a4740931867daa7015632c8445')

build() {
  cd "${srcdir}/$(ls ${srcdir} | grep "elcerdo")"

  cmake . \
  -DCMAKE_INSTALL_PREFIX:FILEPATH=/usr \
  -DCMAKE_BUILD_TYPE:STRING=Release

  make || return 1
  make DESTDIR="$pkgdir/" install
}

# vim:set ts=2 sw=2 et:
