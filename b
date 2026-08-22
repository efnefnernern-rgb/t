echo "gui-apps/quickshell ~amd64" >> /etc/portage/package.accept_keywords/quickshell
emerge --ask gui-apps/quickshell gui-apps/qt6ct dev-qt/qtsvg:6 \
  dev-qt/qtmultimedia:6 dev-qt/qt5compat:6 kde-frameworks/kirigami \
  kde-apps/kdialog kde-frameworks/syntax-highlighting \
  x11-themes/breeze-icons kde-plasma/plasma-integration \
  dev-libs/jemallocCOMMON_FLAGS="-O2 -march=native -pipe"
CFLAGS="${COMMON_FLAGS}"
CXXFLAGS="${COMMON_FLAGS}"
FCFLAGS="${COMMON_FLAGS}"
FFLAGS="${COMMON_FLAGS}"
 
# 16 потоков (6P+8E+2LP) — компилируем на самой машине, оставим запас:
MAKEOPTS="-j12"
 
USE="pipewire -pulseaudio elogind -systemd icu X -gnome -kde"
 
VIDEO_CARDS="intel"
INPUT_DEVICES="libinput"
 
ACCEPT_LICENSE="*"
GRUB_PLATFORMS="efi-64"
