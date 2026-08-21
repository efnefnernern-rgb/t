COMMON_FLAGS="-O2 -march=native -pipe"
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
