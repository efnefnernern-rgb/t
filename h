echo "gui-wm/niri ~amd64" >> /etc/portage/package.accept_keywords/niri
echo "gui-apps/awww ~amd64" >> /etc/portage/package.accept_keywords/awww
echo "gui-apps/xwayland-satellite ~amd64" >> /etc/portage/package.accept_keywords/xwayland-satellite

emerge --ask gui-wm/niri gui-apps/awww sys-devel/bc sys-apps/coreutils \
  gui-apps/cliphist net-misc/curl net-misc/wget sys-apps/ripgrep \
  app-misc/jq dev-lang/python x11-misc/xdg-user-dirs x11-misc/xdg-utils \
  net-misc/rsync dev-vcs/git gui-apps/wl-clipboard x11-libs/libnotify \
  app-portage/pacman-contrib-replacement-not-needed \
  gui-apps/wlsunset sys-apps/xdg-desktop-portal \
  sys-apps/xdg-desktop-portal-gtk sys-apps/xdg-desktop-portal-gnome \
  sys-auth/polkit sys-auth/polkit-kde-agent \
  net-misc/networkmanager gnome-base/gnome-keyring \
  gnome-base/nautilus x11-terms/kitty app-shells/fish \
  gui-apps/xwayland-satellite
