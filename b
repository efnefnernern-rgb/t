echo "gui-apps/quickshell ~amd64" >> /etc/portage/package.accept_keywords/quickshell
emerge --ask gui-apps/quickshell gui-apps/qt6ct dev-qt/qtsvg:6 \
  dev-qt/qtmultimedia:6 dev-qt/qt5compat:6 kde-frameworks/kirigami \
  kde-apps/kdialog kde-frameworks/syntax-highlighting \
  x11-themes/breeze-icons kde-plasma/plasma-integration \
  dev-libs/jemalloc
