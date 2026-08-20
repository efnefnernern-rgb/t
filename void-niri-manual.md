# Void Linux + niri на ASUS Zenbook 14 OLED (Core Ultra 9 285H) — полный мануал

Конфигурация, под которую написан мануал:
- Dual-boot с Windows, которая **не трогается и не форматируется**. Secure Boot и BitLocker выключены.
- 200 ГБ неразмеченного места под Void.
- Файловая система `ext4`, без шифрования.
- glibc, ядро `linux-mainline`, GRUB (увидит Windows через os-prober).
- Wi-Fi Intel BE301, есть Ethernet как запасной вариант.
- Экран 2880×1800 @120Hz OLED.
- Аудио: PipeWire+WirePlumber как основа, deadbeef может забирать ALSA-устройство монопольно "по требованию" (не постоянно) — схема ниже.
- niri + Waybar + dunst + kitty + KDE-приложения (Dolphin/Ark/Okular/Gwenview/Kate/Spectacle/KCalc) + xdg-desktop-portal-kde.
- greetd + regreet (графический greeter со списком пользователей).
- "Пуск" — грид иконок с категориями (nwg-drawer) + быстрый поиск (fuzzel).
- Крышка/простой: если играет музыка — только гасим экран (без сна); если музыки нет — уходим в сон. Простой 3 мин — просто гасить экран (без блокировки).
- power-profiles-daemon, Bluetooth, скриншоты+запись экрана, cliphist, Flatpak, Firefox.
- Переключаемые темы одной командой/меню.

Работайте по разделам по порядку. Команды с `#` в начале строки — это подсказка что выполняется от root (у вас уже есть root-шелл в проблемных местах, `sudo` не всегда нужен — уточняется по контексту).

---

## Часть 0. Что нужно знать заранее

- **BE301** — это Intel Wi-Fi 7 чип. Его прошивка добавлена в linux-firmware относительно недавно и требует свежего ядра (реально нужно 6.11+, часто рекомендуют самое новое стабильное) — поэтому выбор `linux-mainline` у вас правильный и по сути обязательный для этого Wi-Fi. Если после установки Wi-Fi не заработает сразу — не паникуйте, ниже есть отдельный раздел с диагностикой, и у вас есть Ethernet как подстраховка.
- **Core Ultra 9 285H** — это Panther Lake... нет, извините, 285H — это Arrow Lake-H (мобильный), графика Intel Arc (Xe-LPG, поколение до Xe2/Lunar Lake, но платформа очень свежая). Для нормальной работы нужен свежий mesa — версия из свежего Void обычно достаточна, но `linux-mainline` дополнительно подчистит проблемы с планировщиком/энергопотреблением на уровне ядра.
- **niri** официально пакетирован в Void (`xbps-install -S niri`), но версии в бинарном репозитории могут отставать от самых свежих релизов на GitHub — это нормально, стабильная работа важнее bleeding edge.
- Некоторые пакеты (например, `regreet`, возможно отдельные модули) могут не быть в бинарном репозитории Void на момент вашей установки. Для них дан способ через `xbps-src` (сборка из шаблона) как запасной путь — структура точно такая же, как для niri.
- Перед установкой **любого** пакета полезно проверить, что он вообще есть в репах:
  ```
  xbps-query -Rs <имя_пакета>
  ```
  Если пусто — либо неправильное имя (ищите похожие: `xbps-query -Rs <часть_имени>`), либо пакета нет и нужно собирать из исходников.

---

## Часть 1. Подготовка Windows и BIOS

### 1.1 Бэкап (обязательно)
Прежде чем лезть в разметку диска — сделайте бэкап важных данных с Windows (внешний диск/облако). Риск случайно задеть не тот раздел никогда не равен нулю.

### 1.2 Убедитесь, что 200 ГБ свободны и это не просто "видно в проводнике"
В Windows: `Win+X` → «Управление дисками» (Disk Management). Убедитесь, что нужные 200 ГБ отображаются как **"Не распределена" (Unallocated)**, а не как раздел с файловой системой, который вы просто считаете свободным. Раз вы говорите, что место уже выделено — отлично, это самый безопасный вариант (не нужно ужимать том Windows заново, риска для данных Windows нет вообще, мы просто создадим новые разделы в неразмеченной области).

### 1.3 Отключите Fast Startup в Windows
Это важно для dual-boot с общим EFI: Fast Startup держит диск Windows в состоянии гибернации, из-за чего Linux может не видеть/повредить NTFS-раздел, а os-prober может глючить.
- Панель управления → Электропитание → «Действия кнопок питания» → «Изменение параметров, которые сейчас недоступны» → снять галку «Включить быстрый запуск».
- Заодно отключите обычную гибернацию, если не пользуетесь: `powercfg /hibernate off` (от админа в PowerShell).

### 1.4 BIOS/UEFI (ASUS)
Перезагрузка → `F2`/`Del` при старте для входа в BIOS.
- Secure Boot уже выключен — ок, оставляем выключенным (у вас нет задачи держать его включённым, а без него сильно проще: не нужны shim/MOK для сторонних модулей типа `iwlwifi`-патчей или будущего nvidia).
- Убедитесь, что режим загрузки — **UEFI**, не CSM/Legacy.
- Отключите **Fast Boot** в самом BIOS (не путать с Windows Fast Startup) — иначе Live-флешка может не появиться в списке загрузки.
- Проверьте, что в `Boot Priority` есть возможность выбрать загрузку с USB (F8/Esc обычно вызывает Boot Menu на ASUS).

---

## Часть 2. Загрузочная флешка Void Linux

На любом компьютере (можно на этом же ноуте из-под Windows):

1. Скачайте **glibc**-образ (не musl) `void-live-x86_64-<дата>.iso` с https://voidlinux.org/download/ (раздел "Live images", вариант GLIBC).
2. Запишите на флешку (от 4 ГБ):
   - Windows: **Rufus** (режим **DD/ISO image write**, не «ISO mode с преобразованием» — Void ISO гибридный, пишем как есть).
   - Linux/др. ноут: `sudo dd if=void-live-x86_64-YYYYMMDD.iso of=/dev/sdX bs=4M status=progress oflag=sync` (`/dev/sdX` — сама флешка целиком, не раздел).

Загрузитесь с флешки (Boot Menu на Zenbook — обычно `Esc` или `F8` сразу после включения питания). Выберите вход через UEFI (в имени пункта меню должно быть "UEFI").

---

## Часть 3. Live-окружение: сеть и подготовка

После загрузки Live-образа вы попадёте в консоль под root (пароль не нужен) с уже настроенным `runit`.

### 3.1 Раскладка консоли (временно, для ввода WPA-пароля с кириллицей не нужно — пароли обычно латиницей)
```
kbd_mode
loadkeys ru
```
(не обязательно, только если пароль Wi-Fi с кириллицей)

### 3.2 Сеть — Ethernet (проще всего)
Если есть провод — обычно поднимается сам через `dhcpcd`. Проверить:
```
ip a
ping -c3 1.1.1.1
```

### 3.3 Сеть — Wi-Fi BE301 из Live-образа
Live-ISO использует ядро текущей версии Void (не mainline), так что поддержка BE301 **может отсутствовать в Live-окружении** — это нормально, важна поддержка в финальной системе (там будет mainline-ядро). Если Wi-Fi не завёлся в Live — используйте Ethernet только для установки, Wi-Fi настроим после первой загрузки в новую систему с mainline-ядром. Если Wi-Fi всё же виден:
```
wpa_cli
> add_network
0
> set_network 0 ssid "ИмяСети"
OK
> set_network 0 psk "пароль"
OK
> enable_network 0
OK
> quit
dhcpcd wlan0
```
(Интерфейс может называться иначе — смотрите `ip a` до этого).

### 3.4 Часовой пояс live-сессии (не обязательно, просто для удобства часов при установке)
Пропускаем — настроим в chroot.

---

## Часть 4. Разметка диска

### 4.1 Определяем диск
```
lsblk
```
Найдите ваш NVMe-диск, например `/dev/nvme0n1`. Найдите там существующий Windows EFI-раздел (обычно `nvme0n1p1`, тип `EFI System`, размер 100–300 МБ, уже смонтирован в Windows на `\EFI`) и убедитесь, что видите неразмеченную область ~200 ГБ (`lsblk` покажет её как "пусто" между/после Windows-разделов, либо `parted` покажет `Free Space`).

**Никогда не форматируйте и не удаляйте существующие NTFS-разделы и существующий EFI-раздел Windows.** Мы только добавляем новые разделы в свободное место.

### 4.2 Смотрим текущую таблицу разделов
```
parted /dev/nvme0n1 print
```
Запишите себе: номер существующего EFI-раздела (например `1`) и точные границы свободного места (например "начинается на 512GB, заканчивается на 712GB").

### 4.3 Создаём разделы в свободной области
Проверьте объём ОЗУ, чтобы решить размер swap:
```
free -h
```
Правило: если хотите **гибернацию** (suspend-to-disk) в будущем — swap ≥ размер ОЗУ. Вам сейчас гибернация не нужна (только screen-off/suspend по RAM), поэтому берём умеренный swap — 16 ГБ с запасом под zswap/случаи нехватки памяти. Если ОЗУ на ноуте 32 ГБ и вы позже захотите гибернацию — просто пересоздадите swap побольше, это не критично сейчас.

Используем `parted` в интерактивном режиме (замените `1` на реальный номер вашей свободной области, если partad покажет иначе — ориентируйтесь на вывод из 4.2):
```
parted /dev/nvme0n1
```
Внутри `parted` (значения "512GB"/"528GB"/"712GB" — ПОДСТАВЬТЕ свои реальные границы из шага 4.2!):
```
(parted) unit GB
(parted) mkpart primary linux-swap 512GB 528GB
(parted) mkpart primary ext4 528GB 712GB
(parted) print
(parted) quit
```
Это создаст два новых раздела в свободном месте: swap (16GB) и root (остальное ~184GB).

Проверьте:
```
lsblk
```
Вы должны увидеть что-то вроде `nvme0n1p1` (Windows EFI), `nvme0n1p2`..`nvme0n1pN` (Windows-разделы, не трогаем), и новые `nvme0n1p(N+1)` (swap) и `nvme0n1p(N+2)` (root). Дальше в примерах буду использовать условные `nvme0n1p5` (swap) и `nvme0n1p6` (root) — **замените на ваши реальные номера**.

### 4.4 Форматируем новые разделы (только новые! EFI и NTFS Windows не трогаем)
```
mkswap /dev/nvme0n1p5
mkfs.ext4 -L void-root /dev/nvme0n1p6
```

### 4.5 Монтируем
```
mount /dev/nvme0n1p6 /mnt
swapon /dev/nvme0n1p5
mkdir -p /mnt/boot/efi
mount /dev/nvme0n1p1 /mnt/boot/efi
```
Раздел `nvme0n1p1` — это **существующий** EFI Windows, мы просто монтируем его (НЕ форматируем) — так GRUB туда допишет свою запись рядом с записью Windows Boot Manager, ничего не удаляя.

---

## Часть 5. Установка базовой системы (ручной chroot-метод)

Этот метод даёт полный контроль (важно для аккуратного dual-boot и mainline-ядра) в отличие от текстового установщика.

### 5.1 Базовые пакеты
```
mkdir -p /mnt/var/db/xbps/keys
cp /var/db/xbps/keys/*.plist /mnt/var/db/xbps/keys/
xbps-install -S -R https://repo-default.voidlinux.org/current -r /mnt base-system grub grub-x86_64-efi
```
Если во время установки спросит про подтверждение (`Do you want to continue? [Y/n]`) — жмите `Y`. Дождитесь конца (займёт время — качается вся база).

### 5.2 fstab
Узнайте UUID разделов:
```
blkid
```
Создайте `/mnt/etc/fstab`:
```
cat > /mnt/etc/fstab << 'EOF'
UUID=<UUID_root>   /            ext4    defaults,noatime  0 1
UUID=<UUID_efi>    /boot/efi    vfat    defaults,umask=0077  0 2
UUID=<UUID_swap>   swap         swap    defaults          0 0
EOF
```
Замените `<UUID_root>`, `<UUID_efi>`, `<UUID_swap>` на реальные значения из `blkid` (для NTFS-разделов Windows ничего добавлять не нужно — их монтирует сама Windows, Linux их трогать не будет).

### 5.3 Заходим в chroot
```
mount --rbind /dev /mnt/dev
mount --rbind /proc /mnt/proc
mount --rbind /sys /mnt/sys
mount --make-rslave /mnt/dev
mount --make-rslave /mnt/proc
mount --make-rslave /mnt/sys
chroot /mnt /bin/bash
```

Дальше все команды — уже **внутри новой системы**.

### 5.4 Репозитории и синхронизация
```
xbps-install -Su xbps
xbps-install -Su
```

### 5.5 Ядро — сразу linux-mainline
```
xbps-install -S linux-mainline linux-mainline-headers linux-firmware
```
`linux-firmware` подтянет большинство прошивок; конкретно для BE301 может понадобиться самая свежая версия — проверим отдельно в Части 7, если Wi-Fi не заведётся с первого раза.

### 5.6 Хост, локаль, часовой пояс, клавиатура
```
echo "zenbook" > /etc/hostname

echo "ru_RU.UTF-8 UTF-8" >> /etc/default/libc-locales
echo "en_US.UTF-8 UTF-8" >> /etc/default/libc-locales
xbps-reconfigure -f glibc-locales

ln -sf /usr/share/zoneinfo/Europe/Moscow /etc/localtime

cat > /etc/rc.conf << 'EOF'
KEYMAP="ru"
TIMEZONE="Europe/Moscow"
HARDWARECLOCK="UTC"
EOF
```
Раскладка `ru` тут — это раскладка **консоли** (TTY), для графической сессии раскладку/переключение Alt+Shift настроим отдельно в niri (Часть 10).

### 5.7 fstab-модули, dracut (initramfs)
```
xbps-reconfigure -f linux-mainline*
```
(если ругнётся что "уже сконфигурирован" — это нормально, просто пересоберёт initramfs с учётом текущего /etc/fstab и модулей).

### 5.8 Пароль root и создание пользователя
```
passwd
useradd -m -G wheel,video,audio,input,plugdev,kvm,seat -s /bin/bash youruser
passwd youruser
```
Замените `youruser` на желаемое имя пользователя (латиницей, без пробелов).

### 5.9 sudo
```
xbps-install -S sudo
visudo
```
Раскомментируйте строку:
```
%wheel ALL=(ALL:ALL) ALL
```

### 5.10 Сетевые сервисы (база, реальная сеть Wi-Fi/Bluetooth настроим позже в части про DE)
```
xbps-install -S NetworkManager
ln -s /etc/sv/NetworkManager /var/service
ln -s /etc/sv/dbus /var/service
```

---

## Часть 6. GRUB — установка с учётом Windows

### 6.1 os-prober
```
xbps-install -S os-prober ntfs-3g
```
`os-prober` нужен, чтобы GRUB нашёл Windows Boot Manager. `ntfs-3g` — чтобы os-prober смог прочитать NTFS-раздел Windows при сканировании.

### 6.2 Включаем поиск других ОС в конфиге GRUB
```
sed -i 's/^#GRUB_DISABLE_OS_PROBER=false/GRUB_DISABLE_OS_PROBER=false/' /etc/default/grub
grep -q '^GRUB_DISABLE_OS_PROBER' /etc/default/grub || echo 'GRUB_DISABLE_OS_PROBER=false' >> /etc/default/grub
```

### 6.3 Устанавливаем GRUB в EFI (в тот же общий ESP, что и Windows)
```
grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=void --recheck
```
Это **дописывает** запись загрузки Void в NVRAM и в существующий ESP, никак не трогая запись Windows Boot Manager.

### 6.4 Генерируем grub.cfg
```
grub-mkconfig -o /boot/grub/grub.cfg
```
В выводе должно появиться что-то вроде `Found Windows Boot Manager on /dev/nvme0n1p1`. Если не появилось — не паникуйте, доустановим позже (иногда os-prober капризничает из-за Fast Startup — вы его уже отключили в 1.3, это обычно и есть причина, если не сработает).

### 6.5 Проверка порядка загрузки
```
efibootmgr -v
```
Убедитесь, что есть и запись `void`, и запись Windows Boot Manager. При желании поменять, какая грузится по умолчанию — `efibootmgr -o <порядок через запятую>`. Но так как ставим GRUB — можно просто выбирать ОС в меню GRUB при каждой загрузке (там будет и Void, и пункт Windows, если os-prober его нашёл).

---

## Часть 7. Первая загрузка

Выходим из chroot и перезагружаемся:
```
exit
umount -R /mnt
swapoff -a
reboot
```
Выньте флешку. Должно появиться меню GRUB с пунктами Void Linux и Windows Boot Manager. Загружаемся в Void, логинимся под вашим пользователем.

### 7.1 Проверка ядра
```
uname -r
```
Должно показать mainline-версию (что-то вроде `6.1x.x`, актуальную на дату сборки).

### 7.2 Сеть — Ethernet
```
sv status NetworkManager
nmcli device status
```

### 7.3 Wi-Fi BE301 — диагностика, если не завёлся сразу
```
lspci -nnk | grep -A3 -i network
dmesg | grep -i iwlwifi
```
Если видите строки вида `Direct firmware load for iwlwifi-... failed` или `no suitable firmware found` — значит в `linux-firmware` из репозитория ещё нет нужного файла прошивки для BE301 (для совсем свежих чипов Intel это бывает). Решение — доставить свежую прошивку вручную:
```
xbps-install -Su linux-firmware
```
Если после `-Su` всё ещё не хватает файла — скачайте конкретный файл с https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/tree/ (в dmesg будет написано точное имя недостающего файла, например `iwlwifi-ty-a0-gf-a0-...ucode` — ищите по этому имени в дереве репозитория, ветка `main`) и положите в `/lib/firmware/`, затем:
```
modprobe -r iwlwifi
modprobe iwlwifi
```
Через Ethernet это можно делать сразу после первой загрузки — Wi-Fi после этого можно настроить через `nmcli` или `nmtui`, либо графическим апплетом позже (Часть 12).

### 7.4 Обновление системы целиком
```
sudo xbps-install -Su
```

---

## Часть 8. Графика (Intel Arc / Xe-LPG в Core Ultra 9 285H)

```
sudo xbps-install -S mesa mesa-dri mesa-vulkan-intel intel-video-accel vulkan-loader
```
- `mesa-dri` — обязателен для Wayland-сессий без полноценного DE (без него у niri бывают чёрные экраны/проблемы со стартом на некоторых конфигурациях).
- `intel-video-accel` — VA-API для аппаратного декодирования видео (важно для энергопотребления при просмотре видео на OLED).

### 8.1 Переменные окружения для niri (создадим позже вместе с конфигом сессии), но держите в уме:
- `MOZ_ENABLE_WAYLAND=1` — для Firefox под Wayland нативно.
- Явно указывать `LIBVA_DRIVER_NAME=iHD` не обязательно, современный `intel-video-accel` определяется сам, но если декодирование видео не даёт эффекта — пропишем.

### 8.2 Экран 2880×1800 @120Hz
niri настраивает вывод в конфиге `~/.config/niri/config.kdl` (сделаем в Части 10). Забегая вперёд — типичная секция:
```kdl
output "eDP-1" {
    mode "2880x1800@120.000"
    scale 1.5
}
```
Точное имя выхода (`eDP-1` или другое) и точную строку `mode` узнаем командой `niri msg outputs` после первого запуска сессии — впишем финальные значения тогда же.

---

## Часть 9. Аудио — PipeWire + ALSA, deadbeef с эксклюзивным выводом

Ставим PipeWire как основной аудиосервер (современный стандарт, не голый PulseAudio):
```
sudo xbps-install -S pipewire wireplumber alsa-pipewire pavucontrol alsa-utils sof-firmware
```

### 9.1 Включаем PipeWire как замену ALSA/Pulse на уровне сессии
```
mkdir -p ~/.config/pipewire ~/.config/alsa
mkdir -p /etc/pipewire/pipewire.conf.d
sudo ln -sf /usr/share/examples/wireplumber/10-wireplumber.conf /etc/pipewire/pipewire.conf.d/
```
Создайте `~/.asoundrc`, чтобы обычные ALSA-приложения по умолчанию шли через PipeWire (кроме deadbeef, для него — отдельно):
```
cat > ~/.asoundrc << 'EOF'
pcm.!default {
    type pipewire
}
ctl.!default {
    type pipewire
}
EOF
```
Сервисы PipeWire в Void запускаются как **user**-сервисы через `runit`/systemd-user не нужны — PipeWire в Void обычно стартует через сокет-активацию `dbus`. Убедитесь, что автозапуск прописан в сессии niri (Часть 10.5, `spawn-at-startup "pipewire"` и `"wireplumber"`).

### 9.2 Схема "deadbeef с эксклюзивным ALSA, но без вечных конфликтов"
Вы выбрали: **эксклюзивный ALSA-выход для deadbeef, когда сами этого хотите**, но с возможностью быстро переключить его обратно на PipeWire, чтобы не блокировать системный звук постоянно. Реализуем как переключаемый профиль вывода в самом deadbeef — это ровно то, для чего в deadbeef есть выбор аудио-плагина в настройках, никаких хаков с ALSA `hw:` reservation не нужно:

```
sudo xbps-install -S deadbeef deadbeef-plugins
```

В deadbeef: `Настройки → Плеер → Вывод звука`:
- **Профиль "Чисто, эксклюзивно"**: Output plugin = `ALSA` (не «ALSA (pipewire)», не Pulse/JACK), устройство = ваш USB DAC (появится в списке по имени устройства после подключения, например `hw:CARD=DAC,DEV=0`). Это открывает устройство напрямую в обход PipeWire — во время воспроизведения этим профилем ваш USB DAC будет занят deadbeef и недоступен другим приложениям (то, что вы просили — "чистый эксклюзивный выход когда хочу").
- **Профиль "Обычный, через систему"**: Output plugin = `PipeWire` (в списке плагинов deadbeef он называется именно так, если пакет `deadbeef-plugins` установлен) — тогда deadbeef ведёт себя как любое другое приложение и не мешает остальному звуку.

Переключение между профилями — это буквально одна смена значения в выпадающем списке "Output plugin" в настройках deadbeef (Ctrl+P). Чтобы не лезть в настройки каждый раз, можно повесить это на переключение через два сохранённых `.dbdb`-конфига, но для начала проще всего — просто дважды кликнуть по selector'у, deadbeef применяет изменение вывода на лету без перезапуска.

**Важный нюанс:** пока deadbeef держит USB DAC монопольно через чистый ALSA-плагин, PipeWire попытается достучаться до этого же устройства и не сможет (вы услышите тишину или ошибку у других приложений) — это ожидаемое поведение эксклюзивного режима, вы сами это запросили. Как только вернёте deadbeef на профиль PipeWire (или остановите воспроизведение) — устройство сразу освобождается для системы.

Если USB DAC не отображается в ALSA-профиле deadbeef сразу после подключения — обновите список: `Настройки → Вывод → Устройство → (клик по списку заново)`, либо перезапустите PipeWire на секунду командой `systemctl --user restart pipewire wireplumber` (если PipeWire его уже занял, deadbeef всё равно сможет забрать `hw:` напрямую, PipeWire откроет устройство заново при освобождении).

### 9.3 Bluetooth-аудио (наушники/колонки) — да, вы это просили
```
sudo xbps-install -S bluez blueman
sudo ln -s /etc/sv/bluetoothd /var/service
xbps-install -S wireplumber   # уже стоит, просто убеждаемся
```
WirePlumber сам подхватывает Bluetooth-профили (A2DP для качественного звука, HFP для звонков/микрофона) — ничего специально настраивать не нужно, кроме подключения устройства через `blueman-manager` (иконка появится в трее Waybar, Часть 12).

---

## Часть 10. niri — компоситор и Wayland-сессия

### 10.1 Установка
```
sudo xbps-install -S niri xwayland-satellite seatd polkit polkit-kde-agent \
    wayland wayland-protocols libinput mesa-dri
```
- `xwayland-satellite` — нужен для запуска старых X11-приложений внутри niri (некоторые вещи, например часть Java-приложений, только X11).
- `seatd` — управление сеансом/устройствами без полноценного logind (Void по умолчанию использует `elogind` частично, но `seatd` — более простой и надёжный вариант для нестандартных сессий типа niri).
- `polkit-kde-agent` — раз у нас KDE-приложения, берём именно KDE-агент polkit для единообразных диалогов повышения прав (запрос пароля при системных действиях).

### 10.2 Включаем seatd и добавляем пользователя в группу
```
sudo ln -s /etc/sv/seatd /var/service
sudo usermod -aG seat,video,input $USER
```
Перелогиньтесь (или `newgrp seat`) чтобы группы применились.

### 10.3 Портал (обязательно — раз используете KDE-приложения)
```
sudo xbps-install -S xdg-desktop-portal xdg-desktop-portal-kde xdg-desktop-portal-gtk
```
Оба портала ставим специально: `-kde` даёт нормальные системные диалоги "Открыть файл" в духе KDE-приложений (Dolphin/Okular/Kate и т.д.), `-gtk` — то же самое для GTK-программ (Firefox и др.), чтобы у них тоже были нормальные диалоги, а не сырые GTK по умолчанию. Настроим, какой портал что обслуживает, конфигом:
```
mkdir -p ~/.config/xdg-desktop-portal
cat > ~/.config/xdg-desktop-portal/niri-portals.conf << 'EOF'
[preferred]
default=gtk
org.freedesktop.impl.portal.FileChooser=kde
org.freedesktop.impl.portal.AppChooser=kde
org.freedesktop.impl.portal.Screenshot=gtk
org.freedesktop.impl.portal.ScreenCast=gtk
EOF
```
Это значит: диалоги выбора файлов — красивые KDE (Dolphin-style), а скриншоты/screencast идут через wlroots-совместимый портал (в GTK-портале есть нужная реализация под niri).

### 10.4 Курсор, шрифты, иконки (общая база под все стили)
```
sudo xbps-install -S adwaita-icon-theme breeze-icons papirus-icon-theme \
    ttf-dejavu noto-fonts-ttf noto-fonts-emoji font-awesome \
    qt5ct qt6ct kvantum kvantum-qt5 lxappearance nwg-look
```
`qt5ct`/`qt6ct`+`kvantum` — чтобы Qt/KDE-приложения красиво красились темой (не оставались серыми "по умолчанию") в окружении без полноценной Plasma. `nwg-look`/`lxappearance` — то же самое для GTK-приложений.

Пропишите переменные окружения для Qt (создадим общий env-файл сессии в 10.5):
```
QT_QPA_PLATFORMTHEME=qt6ct
```

### 10.5 Автогенерация и правка конфига niri
Первый запуск niri сам создаст конфиг по умолчанию. Пока не запускаем сессию — сначала настроим greetd (Часть 11), чтобы сразу входить красиво. Заготовим `~/.config/niri/config.kdl` заранее:

```
mkdir -p ~/.config/niri
cat > ~/.config/niri/config.kdl << 'EOF'
// === Ввод ===
input {
    keyboard {
        xkb {
            layout "us,ru"
            options "grp:alt_shift_toggle"
        }
        numlock
    }
    touchpad {
        tap
        natural-scroll
        dwt // отключение тачпада во время печати
    }
    mouse {
        accel-profile "flat"
    }
}

// === Экран ===
output "eDP-1" {
    mode "2880x1800@120.000"
    scale 1.5
}

// === Автозапуск сессии ===
spawn-at-startup "dbus-update-activation-environment" "--systemd" "--all"
spawn-at-startup "pipewire"
spawn-at-startup "pipewire-pulse"
spawn-at-startup "wireplumber"
spawn-at-startup "waybar"
spawn-at-startup "dunst"
spawn-at-startup "nm-applet" "--indicator"
spawn-at-startup "blueman-applet"
spawn-at-startup "swaybg" "-i" "~/.config/niri/wallpapers/current.jpg" "-m" "fill"
spawn-at-startup "wl-paste" "--watch" "cliphist" "store"
spawn-at-startup "swayidle" "-w"
spawn-at-startup "/usr/lib/polkit-kde-authentication-agent-1"
spawn-at-startup "xwayland-satellite"

environment {
    XDG_CURRENT_DESKTOP "niri"
    XDG_SESSION_TYPE "wayland"
    QT_QPA_PLATFORM "wayland"
    QT_QPA_PLATFORMTHEME "qt6ct"
    QT_WAYLAND_DISABLE_WINDOWDECORATION "1"
    MOZ_ENABLE_WAYLAND "1"
    GDK_BACKEND "wayland,x11"
    _JAVA_AWT_WM_NONREPARENTING "1"
    NIXOS_OZONE_WL "1"
    DISPLAY ":0"
}

// === Внешний вид ===
layout {
    gaps 8
    center-focused-column "never"
    default-column-width { proportion 0.5; }
    focus-ring {
        width 2
        active-color "#89b4fa"
        inactive-color "#45475a"
    }
    border {
        off
    }
}

prefer-no-csd

// === Горячие клавиши ===
binds {
    // Терминал / лаунчер / файлы
    Mod+Return { spawn "kitty"; }
    Mod+D { spawn "fuzzel"; }
    Mod+Shift+D { spawn "nwg-drawer" "-r"; }
    Mod+E { spawn "dolphin"; }
    Mod+B { spawn "firefox"; }

    // Управление окнами
    Mod+Q { close-window; }
    Mod+F { maximize-column; }
    Mod+Shift+F { fullscreen-window; }
    Mod+V { toggle-window-floating; }
    Mod+Space { switch-preset-column-width; }

    // Навигация — колонки/окна (стиль niri, "scrollable tiling")
    Mod+Left  { focus-column-left; }
    Mod+Right { focus-column-right; }
    Mod+Up    { focus-window-up; }
    Mod+Down  { focus-window-down; }
    Mod+Shift+Left  { move-column-left; }
    Mod+Shift+Right { move-column-right; }
    Mod+H { focus-column-left; }
    Mod+L { focus-column-right; }
    Mod+K { focus-window-up; }
    Mod+J { focus-window-down; }

    // Рабочие столы
    Mod+Page_Down { focus-workspace-down; }
    Mod+Page_Up   { focus-workspace-up; }
    Mod+Shift+Page_Down { move-window-to-workspace-down; }
    Mod+Shift+Page_Up   { move-window-to-workspace-up; }
    Mod+1 { focus-workspace 1; }
    Mod+2 { focus-workspace 2; }
    Mod+3 { focus-workspace 3; }
    Mod+4 { focus-workspace 4; }
    Mod+5 { focus-workspace 5; }
    Mod+Shift+1 { move-window-to-workspace 1; }
    Mod+Shift+2 { move-window-to-workspace 2; }
    Mod+Shift+3 { move-window-to-workspace 3; }
    Mod+Shift+4 { move-window-to-workspace 4; }
    Mod+Shift+5 { move-window-to-workspace 5; }

    // Система
    Mod+Alt+L { spawn "swaylock"; }
    Mod+Shift+E { quit; }
    Mod+Shift+P { spawn "wlogout"; }
    Print { spawn "grimblast" "copy" "area"; }
    Shift+Print { spawn "grimblast" "copy" "screen"; }
    Mod+Print { spawn "sh" "-c" "wf-recorder -g \"$(slurp)\" -f ~/Videos/rec-$(date +%s).mp4"; }
    Mod+Shift+V { spawn "cliphist-fuzzel-wrapper"; }

    // Звук/яркость (медиаклавиши)
    XF86AudioRaiseVolume { spawn "wpctl" "set-volume" "@DEFAULT_AUDIO_SINK@" "5%+"; }
    XF86AudioLowerVolume { spawn "wpctl" "set-volume" "@DEFAULT_AUDIO_SINK@" "5%-"; }
    XF86AudioMute { spawn "wpctl" "set-mute" "@DEFAULT_AUDIO_SINK@" "toggle"; }
    XF86AudioMicMute { spawn "wpctl" "set-mute" "@DEFAULT_AUDIO_SOURCE@" "toggle"; }
    XF86MonBrightnessUp { spawn "brightnessctl" "set" "5%+"; }
    XF86MonBrightnessDown { spawn "brightnessctl" "set" "5%-"; }
    XF86AudioPlay { spawn "playerctl" "play-pause"; }
    XF86AudioNext { spawn "playerctl" "next"; }
    XF86AudioPrev { spawn "playerctl" "previous"; }
}

// === Крышка ноутбука (логика ниже реализована отдельным скриптом) ===
switch-events {
    lid-close {
        spawn "/usr/local/bin/lid-close-handler.sh";
    }
}
EOF
```

Пояснения по важным местам:
- `layout "us,ru"` + `options "grp:alt_shift_toggle"` — переключение раскладки именно Alt+Shift, как вы просили.
- `output "eDP-1"` — имя монитора и точную строку режима **обязательно перепроверьте** после первого входа командой `niri msg outputs` (см. 10.6) и поправьте если отличается.
- `prefer-no-csd` — просим приложения (в т.ч. GTK) рисовать серверные рамки через декорации niri, а не свои — иначе разнобой во внешнем виде окон.
- `Mod` по умолчанию в niri — это клавиша Super (Win). Отдельно указывать не нужно, но если хотите сменить на Alt — добавьте `mod-key "Alt"` в секцию `input`.

### 10.6 Проверка вывода экрана после первого входа
```
niri msg outputs
```
Скопируйте точное имя выхода и точную строку "mode" (формат `WxH@Hz`) в конфиг вместо `eDP-1`/`2880x1800@120.000`, если они отличаются, и перезапустите niri (`Mod+Shift+E` → войти заново, либо `niri msg action reload-config` если такая команда доступна в вашей версии — иначе полный релогин).

---

## Часть 11. greetd + regreet — графический вход со списком пользователей

```
sudo xbps-install -S greetd
```
Проверьте наличие `regreet`:
```
xbps-query -Rs regreet
```
Если пусто — соберите из исходников (аналогично тому, как это делается для niri, `regreet` тоже на Rust/cargo):
```
sudo xbps-install -S xtools git base-devel cargo rustc
git clone --depth=1 https://github.com/void-linux/void-packages.git ~/void-packages
cd ~/void-packages
./xbps-src binary-bootstrap
xnew regreet   # если шаблона ещё нет — откроется редактор для создания
```
Если `xnew` откроет пустой шаблон — заполните по образцу (проверьте актуальную версию/checksum на странице релизов https://github.com/rharish101/ReGreet/releases):
```
# srcpkgs/regreet/template
pkgname=regreet
version=0.2.0
revision=1
build_style=cargo
hostmakedepends="pkg-config"
makedepends="gtk4-devel glib-devel"
depends="greetd"
short_desc="GTK-based greeter for greetd"
maintainer="you <you@example.com>"
license="GPL-3.0-or-later"
homepage="https://github.com/rharish101/ReGreet"
distfiles="https://github.com/rharish101/ReGreet/archive/refs/tags/v${version}.tar.gz"
checksum=<ВПИШИТЕ_SHA256_С_GITHUB_RELEASE>
```
```
./xbps-src pkg regreet
sudo xbps-install --repository=hostdir/binpkgs regreet
```

### 11.1 Настройка greetd → regreet → niri
```
sudo mkdir -p /etc/greetd
sudo tee /etc/greetd/config.toml > /dev/null << 'EOF'
[terminal]
vt = 1

[default_session]
command = "regreet --cage-cmd 'niri --session'"
user = "greeter"
EOF
```
Если у вашей сборки `regreet` нет флага `--cage-cmd` (зависит от версии) — используйте связку через `cage` (минимальный киоск-компоситор для запуска самого greeter'а, не путать с сессией пользователя):
```
sudo xbps-install -S cage
```
```
[default_session]
command = "cage -s -- regreet"
user = "greeter"
```
и отдельно настройте `~/.config/regreet.toml` (у пользователя `greeter`), где укажите команду сессии `niri --session` как один из пунктов выбора сессии — regreet сам подхватывает `.desktop`-файлы сессий из `/usr/share/wayland-sessions/`, а niri уже кладёт туда свой `niri.desktop` при установке пакета, так что обычно **ничего вручную указывать не требуется** — просто выберите "niri" в выпадающем списке сессий на экране входа.

### 11.2 Пользователь greeter и права
```
sudo groupadd -r greeter 2>/dev/null
sudo useradd -M -G video -s /usr/sbin/nologin greeter 2>/dev/null
```
(Если пакет `greetd` уже создал этого пользователя сам при установке — команда просто ничего не сделает / выдаст безобидную ошибку "уже существует", это нормально).

### 11.3 Включаем greetd как сервис входа
```
sudo ln -s /etc/sv/greetd /var/service
```
На этом этапе **все TTY-автологины отключите**, если вы их где-то включали вручную — greetd сам должен управлять сеансом на vt1.

### 11.4 Фон и список пользователей
regreet берёт фон и список системных пользователей автоматически (пользователей с `UID >= 1000` и домашней директорией). Обои для экрана входа настраиваются в `/etc/greetd/regreet.toml` (общесистемный, не домашний путь пользователя greeter):
```
sudo mkdir -p /etc/greetd
sudo tee -a /etc/greetd/regreet.toml > /dev/null << 'EOF'
[background]
path = "/usr/share/backgrounds/login.jpg"
fit = "Cover"

[GTK]
application_prefer_dark_theme = true
theme_name = "Adwaita-dark"
icon_theme_name = "Papirus-Dark"
EOF
```
Положите желаемую картинку в `/usr/share/backgrounds/login.jpg` (`sudo cp ~/Изображения/what.jpg /usr/share/backgrounds/login.jpg`).

---

## Часть 12. Waybar — панель (максимально функциональная: трей, всё как в Windows)

```
sudo xbps-install -S waybar network-manager-applet blueman playerctl brightnessctl \
    power-profiles-daemon upower
sudo ln -s /etc/sv/power-profiles-daemon /var/service
sudo ln -s /etc/sv/upower /var/service
```

### 12.1 Структура конфигов
```
mkdir -p ~/.config/waybar
```

`~/.config/waybar/config.jsonc`:
```jsonc
{
    "layer": "top",
    "position": "top",
    "height": 34,
    "spacing": 4,
    "modules-left": ["niri/workspaces", "niri/window"],
    "modules-center": ["clock"],
    "modules-right": [
        "idle_inhibitor",
        "power-profiles-daemon",
        "pulseaudio",
        "network",
        "bluetooth",
        "battery",
        "tray"
    ],

    "niri/workspaces": {
        "format": "{icon}",
        "format-icons": { "active": "", "default": "" }
    },
    "niri/window": {
        "max-length": 60,
        "separate-outputs": true
    },
    "clock": {
        "format": "{:%H:%M   %d.%m.%Y}",
        "tooltip-format": "<big>{:%Y %B}</big>\n<tt><small>{calendar}</small></tt>"
    },
    "idle_inhibitor": {
        "format": "{icon}",
        "format-icons": { "activated": "", "deactivated": "" }
    },
    "power-profiles-daemon": {
        "format": "{icon}",
        "tooltip-format": "Профиль питания: {profile}\n{driver}",
        "format-icons": {
            "default": "",
            "performance": "",
            "balanced": "",
            "power-saver": ""
        }
    },
    "pulseaudio": {
        "format": "{icon} {volume}%",
        "format-muted": " Выкл",
        "format-icons": { "default": ["", "", ""] },
        "on-click": "pavucontrol",
        "scroll-step": 5
    },
    "network": {
        "format-wifi": " {signalStrength}%",
        "format-ethernet": " Провод",
        "format-disconnected": " Нет сети",
        "tooltip-format": "{ifname}: {ipaddr}",
        "on-click": "nm-connection-editor"
    },
    "bluetooth": {
        "format": "",
        "format-connected": " {device_alias}",
        "on-click": "blueman-manager"
    },
    "battery": {
        "format": "{icon} {capacity}%",
        "format-charging": " {capacity}%",
        "format-icons": ["", "", "", "", ""],
        "states": { "warning": 20, "critical": 10 }
    },
    "tray": {
        "icon-size": 18,
        "spacing": 8
    }
}
```

Слева ещё желательно добавить кнопку "Пуск" (грид-меню, Часть 13) — модуль `custom/launcher`:
```jsonc
    "modules-left": ["custom/launcher", "niri/workspaces", "niri/window"],
    "custom/launcher": {
        "format": "",
        "on-click": "nwg-drawer -r",
        "tooltip": false
    },
```

`~/.config/waybar/style.css` (базовая тема — детальнее переключаемые темы в Части 22):
```css
* {
    font-family: "JetBrainsMono Nerd Font", "Noto Sans", sans-serif;
    font-size: 13px;
}
window#waybar {
    background-color: rgba(30, 30, 46, 0.85);
    color: #cdd6f4;
}
#workspaces button {
    padding: 0 8px;
    color: #6c7086;
}
#workspaces button.active {
    color: #89b4fa;
}
#clock, #battery, #network, #pulseaudio, #tray, #power-profiles-daemon,
#bluetooth, #idle_inhibitor, #custom-launcher {
    padding: 0 10px;
}
#battery.warning { color: #f9e2af; }
#battery.critical { color: #f38ba8; }
```

Для иконок вроде `` понадобится Nerd Font:
```
sudo xbps-install -S nerd-fonts-jetbrains-mono
```
(если такого точного имени пакета нет — `xbps-query -Rs nerd-fonts` покажет доступные варианты в вашей версии репозитория, возьмите любой Nerd Font, названия слегка меняются между релизами Void).

---

## Часть 13. "Пуск" — грид иконок с категориями + быстрый поиск

Вы выбрали грид с категориями, максимально близко к ощущению Windows-меню. Ближайший рабочий инструмент в экосистеме wlroots/niri — **nwg-drawer** (полноэкранный грид всех приложений с фильтром по категориям и поиском по вводу). Плюс отдельный быстрый лаунчер **fuzzel** для мгновенного запуска по Alt+Space/`Mod+D` (как Windows-поиск/PowerToys Run).

```
sudo xbps-install -S fuzzel
xbps-query -Rs nwg-drawer
```
Если `nwg-drawer` есть в репозитории — ставьте:
```
sudo xbps-install -S nwg-drawer
```
Если нет — соберите из исходников тем же способом, что и `regreet` в Части 11 (это часть пакета `nwg-shell`, репозиторий https://github.com/nwg-piotr/nwg-drawer, зависимости — `gtk3-devel`, `gtk-layer-shell-devel`).

### 13.1 Настройка fuzzel (быстрый запуск, `Mod+D`)
`~/.config/fuzzel/fuzzel.ini`:
```ini
[main]
font=JetBrainsMono Nerd Font:size=12
terminal=kitty
width=40
horizontal-pad=20
vertical-pad=12
inner-pad=8
prompt="  "

[colors]
background=1e1e2edd
text=cdd6f4ff
match=89b4faff
selection=45475aff
selection-text=cdd6f4ff
border=89b4faff
```

### 13.2 Настройка nwg-drawer (грид с категориями, `Mod+Shift+D` или клик по кнопке "Пуск" на панели)
`nwg-drawer` без конфига уже собирает грид из всех `.desktop` файлов с автоматической разбивкой по категориям (Интернет, Графика, Офис, Система и т.д.) на основе поля `Categories=` в самих `.desktop`-файлах — специально настраивать список не требуется, работает "из коробки". Можно задать только внешний вид:
```
mkdir -p ~/.config/nwg-drawer
cat > ~/.config/nwg-drawer/drawer.css << 'EOF'
window {
    background-color: rgba(24, 24, 37, 0.92);
}
.category-button {
    color: #cdd6f4;
}
.category-button:checked {
    color: #89b4fa;
}
EOF
```
Запуск: `nwg-drawer -r` (флаг `-r` — закрыть предыдущий инстанс перед открытием нового, полезно если жмёте комбинацию повторно).

---

## Часть 14. Уведомления — dunst

```
sudo xbps-install -S dunst libnotify
```
`~/.config/dunst/dunstrc`:
```ini
[global]
    width = 320
    height = 150
    origin = top-right
    offset = 12x40
    frame_width = 1
    frame_color = "#89b4fa"
    font = Noto Sans 10
    format = "<b>%s</b>\n%b"
    corner_radius = 8

[urgency_low]
    background = "#1e1e2e"
    foreground = "#cdd6f4"
    timeout = 5

[urgency_normal]
    background = "#1e1e2e"
    foreground = "#cdd6f4"
    timeout = 8

[urgency_critical]
    background = "#f38ba8"
    foreground = "#1e1e2e"
    timeout = 0
```
Уже добавлен в автозапуск в конфиге niri (`spawn-at-startup "dunst"`).

---

## Часть 15. kitty — терминал

```
sudo xbps-install -S kitty
```
`~/.config/kitty/kitty.conf`:
```ini
font_family      JetBrainsMono Nerd Font
font_size        12.0

cursor_shape     beam
cursor_blink_interval 0

background_opacity 0.92
window_padding_width 8
confirm_os_window_close 0

# Тема подключается отдельным файлом — см. Часть 22 (переключение тем)
include current-theme.conf

enable_audio_bell no
allow_remote_control yes

map ctrl+shift+c copy_to_clipboard
map ctrl+shift+v paste_from_clipboard
map ctrl+shift+t new_tab
map ctrl+shift+enter new_window
map ctrl+tab next_tab
map ctrl+shift+tab previous_tab
```
Заготовка `~/.config/kitty/current-theme.conf` создастся автоматически системой смены тем (Часть 22) — до тех пор создайте пустой файл, чтобы kitty не ругался при первом запуске:
```
touch ~/.config/kitty/current-theme.conf
```
Если по какой-то причине GPU-рендеринг kitty будет работать нестабильно на вашей графике (маловероятно на Arc/Xe-LPG, но на всякий случай) — запасной вариант **foot**:
```
sudo xbps-install -S foot
```
и замена `spawn "kitty"` на `spawn "foot"` в биндах niri.

---

## Часть 16. KDE-приложения (настоящие, не GTK-аналоги)

```
sudo xbps-install -S dolphin ark okular gwenview kate spectacle kcalc \
    kio-extras ffmpegthumbs kimageformats breeze breeze-icons \
    kde-cli-tools kdegraphics-thumbnailers
```
- `kio-extras` — сетевые протоколы в Dolphin (smb://, sftp:// и т.д.) и превью иконок для многих форматов.
- `ffmpegthumbs`, `kdegraphics-thumbnailers`, `kimageformats` — миниатюры видео/RAW-фото/доп. форматов в Dolphin и Gwenview.
- `breeze`+`breeze-icons` — родная тема KDE, чтобы приложения выглядели "как надо", а не серыми.

Настройте Qt-тему один раз (Часть 10.4 уже поставила `qt5ct`/`qt6ct`/`kvantum`):
```
qt6ct
```
В открывшемся окне: Style → `kvantum` (после того как в kvantum-manager выберете тему Breeze — `kvantum-manager` из пакета `kvantum`, там же можно поставить Breeze-Dark и т.п.). Это заставит и Qt5, и Qt6 приложения (включая Dolphin/Okular/Kate) выглядеть цельно.

Ассоциации файлов по умолчанию настраиваются прямо в Dolphin: `Настройки → Настроить Dolphin → Общие`, или системно через `xdg-mime default org.kde.okular.desktop application/pdf` и т.п.

---

## Часть 17. Простой, крышка, блокировка

Логика, которую вы просили:
- **3 минуты простоя без действий** → просто гасим экран (DPMS off), без блокировки, без сна.
- **Закрытие крышки:**
  - если в этот момент **играет музыка** (любой MPRIS-плеер, включая deadbeef) → **только гасим экран**, ноутбук НЕ уходит в сон, музыка продолжает играть.
  - если музыки **нет** → уходим в **настоящий сон** (suspend to RAM).
- Обычный ручной suspend (по кнопке питания/явной команде) — настраиваем отдельно, раз автоматика по крышке/простою больше не трогает system suspend просто так.

### 17.1 Инструменты
```
sudo xbps-install -S swayidle swaylock playerctl
```

### 17.2 Простой 3 минуты → гасим экран (swayidle)
Уже добавлено `spawn-at-startup "swayidle" "-w"` в конфиг niri (10.5) без аргументов — реальные таймауты задаём отдельным конфиг-файлом, чтобы не захламлять `config.kdl` длинной командой:
```
mkdir -p ~/.config/swayidle
cat > ~/.config/swayidle/config << 'EOF'
timeout 180 "niri msg action power-off-monitors"
resume "niri msg action power-on-monitors"
EOF
```
Поправьте автозапуск в niri, чтобы swayidle читал именно этот конфиг (замените строку `spawn-at-startup "swayidle" "-w"` на):
```kdl
spawn-at-startup "swayidle" "-w" "-C" "/home/youruser/.config/swayidle/config"
```
(замените `/home/youruser` на реальный путь — `swayidle` не всегда разворачивает `~` из systemd/niri spawn-контекста, лучше указать абсолютный путь явно). 180 секунд = 3 минуты. Без `timeout ... lock` — блокировки по простою специально нет, только гашение экрана, как вы и просили.

### 17.3 Крышка ноутбука — скрипт с проверкой "играет ли музыка"
Создаём скрипт, который проверяет через `playerctl`, есть ли активно играющий MPRIS-плеер (deadbeef поддерживает MPRIS "из коробки" через штатный плагин, включите его: `deadbeef → Настройки → Плагины → MPRIS` — обычно уже включён по умолчанию):

```
sudo tee /usr/local/bin/lid-close-handler.sh > /dev/null << 'EOF'
#!/bin/sh
# Если есть хоть один MPRIS-плеер в состоянии Playing — просто гасим экран.
# Если играющих плееров нет — уходим в настоящий сон.

if playerctl -a status 2>/dev/null | grep -q "Playing"; then
    niri msg action power-off-monitors
else
    systemctl suspend
fi
EOF
sudo chmod +x /usr/local/bin/lid-close-handler.sh
```
Этот скрипт уже подключён в `config.kdl` (Часть 10.5, секция `switch-events { lid-close { ... } }`).

**Важный момент про пробуждение при открытии крышки:** когда экран просто погашен через `power-off-monitors` (музыка играла), для возврата картинки достаточно открыть крышку/пошевелить мышью — niri сам включает монитор обратно по факту открытия крышки (это встроенное поведение компоситора). Отдельно прописывать `lid-open` не обязательно, но можно на всякий случай добавить принудительное включение — допишите в `config.kdl`:
```kdl
switch-events {
    lid-close {
        spawn "/usr/local/bin/lid-close-handler.sh";
    }
    lid-open {
        spawn "niri" "msg" "action" "power-on-monitors";
    }
}
```
(это заменяет старую секцию `switch-events` из 10.5 — просто добавьте туда блок `lid-open`).

### 17.4 systemd-logind — чтобы closing lid по умолчанию не делал ничего лишнего в обход нашего скрипта
Void по умолчанию не использует systemd, но `elogind`(если он у вас есть как часть `polkit`/сессии) тоже реагирует на крышку через `logind.conf`-подобный файл. На всякий случай отключим системную реакцию на крышку, чтобы работала **только** наша логика через niri:
```
sudo mkdir -p /etc/elogind
sudo tee -a /etc/elogind/logind.conf > /dev/null << 'EOF'
HandleLidSwitch=ignore
HandleLidSwitchExternalPower=ignore
HandleLidSwitchDocked=ignore
EOF
sudo sv restart elogind 2>/dev/null || true
```
Если у вас в системе нет каталога/сервиса `elogind` (используется чистый `seatd` без него) — эта команда просто не нужна, `niri` сам полностью управляет реакцией на крышку через `switch-events`, конфликтов не будет.

### 17.5 Ручной suspend (по вашему запросу — отдельно от автоматики)
Уже есть в биндах — добавьте явный бинд на кнопку питания и/или в меню `wlogout` (Часть 17.6):
```kdl
binds {
    // ...
    XF86PowerOff { spawn "systemctl" "suspend"; }
}
```

### 17.6 Меню выхода/блокировки (аналог "Пуск → Завершение работы" в Windows)
```
sudo xbps-install -S wlogout
```
`~/.config/wlogout/layout`:
```
{
    "label" : "lock",
    "action" : "swaylock",
    "text" : "Заблокировать",
    "keybind" : "l"
}
{
    "label" : "suspend",
    "action" : "systemctl suspend",
    "text" : "Спящий режим",
    "keybind" : "s"
}
{
    "label" : "logout",
    "action" : "niri msg action quit",
    "text" : "Выйти",
    "keybind" : "e"
}
{
    "label" : "reboot",
    "action" : "systemctl reboot",
    "text" : "Перезагрузка",
    "keybind" : "r"
}
{
    "label" : "shutdown",
    "action" : "systemctl poweroff",
    "text" : "Выключить",
    "keybind" : "p"
}
```
Уже подключено на `Mod+Shift+P` в конфиге niri.

### 17.7 swaylock — оформление (используется по `Mod+Alt+L` и в wlogout)
```
mkdir -p ~/.config/swaylock
cat > ~/.config/swaylock/config << 'EOF'
color=1e1e2e
font=JetBrainsMono Nerd Font
indicator-radius=90
indicator-thickness=8
inside-color=1e1e2e
ring-color=89b4fa
key-hl-color=a6e3a1
line-color=00000000
show-failed-attempts
EOF
```

---

## Часть 18. Управление питанием — power-profiles-daemon

Уже установлен и включён в Части 12. Проверка:
```
powerprofilesctl list
powerprofilesctl set balanced
```
Переключение — кликом по иконке в Waybar (модуль `power-profiles-daemon`, клик по нему циклически меняет профиль; если хотите явное меню — можно повесить `on-click` на `rofi`/`fuzzel`-обёртку со списком трёх режимов, но обычно клика достаточно). Профили: `power-saver`, `balanced`, `performance`.

Для Core Ultra 9 285H — thermal-режим лэптопа сильно зависит от свежести ядра (у вас mainline — плюс), power-profiles-daemon уже умеет работать с `intel_pstate`/`amd_pstate`-аналогами на Intel через platform-profile ACPI, дополнительной настройки для базового сценария не нужно.

---

## Часть 19. Скриншоты и запись экрана

```
sudo xbps-install -S grim slurp wf-recorder swappy
xbps-query -Rs grimblast
```
`grimblast` — удобная обёртка над `grim`+`slurp` (копирование в буфер/сохранение одной командой), уже использована в биндах Части 10.5. Если пакета нет в репозитории Void — поставьте вручную (это простой bash-скрипт из репозитория hyprwm/contrib, совместим с любым wlroots-композитором, не только Hyprland):
```
sudo curl -o /usr/local/bin/grimblast \
    https://raw.githubusercontent.com/hyprwm/contrib/main/grimblast/grimblast
sudo chmod +x /usr/local/bin/grimblast
```
`swappy` — быстрый редактор скриншота (стрелки/выделение) сразу после снимка, добавьте по желанию `grimblast --notify copy area | swappy -f -`.

Запись экрана уже забинжена на `Mod+Print` (Часть 10.5), сохраняет в `~/Videos/`. Создайте папку:
```
mkdir -p ~/Videos ~/Pictures/Screenshots
```

---

## Часть 20. Буфер обмена — cliphist

```
sudo xbps-install -S cliphist wl-clipboard
```
Уже добавлен сборщик истории в автозапуск (`wl-paste --watch cliphist store`, Часть 10.5). Обёртка для показа истории через fuzzel:
```
sudo tee /usr/local/bin/cliphist-fuzzel-wrapper > /dev/null << 'EOF'
#!/bin/sh
cliphist list | fuzzel --dmenu | cliphist decode | wl-copy
EOF
sudo chmod +x /usr/local/bin/cliphist-fuzzel-wrapper
```
Вызов — `Mod+Shift+V` (уже в биндах), работает прямо как история буфера обмена в Windows (`Win+V`).

---

## Часть 21. Firefox и Flatpak

```
sudo xbps-install -S firefox
```
Wayland-режим уже включён переменной `MOZ_ENABLE_WAYLAND=1` в окружении сессии (Часть 10.5).

```
sudo xbps-install -S flatpak
sudo flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
```
Перелогиньтесь после установки flatpak, чтобы `.desktop`-файлы Flatpak-приложений подхватились в `nwg-drawer`/`fuzzel` (они лежат в `~/.local/share/flatpak/exports/share/applications`, эта директория должна попасть в `XDG_DATA_DIRS` — обычно flatpak прописывает это сам при первой установке приложения).

---

## Часть 22. Система тем — переключение одной командой/меню (аналог того, что было на Hyprland)

Готового инструмента уровня ML4W/HyDE для niri пока не существует (у niri нет такой большой dotfiles-экосистемы, как у Hyprland) — но мы соберём свой лёгкий переключатель, который одним нажатием красит **сразу**: Waybar, kitty, GTK, Qt/Kvantum и обои. Технически — набор готовых "тем" (папок с конфигами) + один bash-скрипт + меню на fuzzel.

### 22.1 Структура
```
mkdir -p ~/.config/theme-switcher/themes/{catppuccin-mocha,nord,gruvbox-dark}
mkdir -p ~/.config/niri/wallpapers
```

Каждая тема — это папка с 4 файлами:
- `waybar.css` — стиль для Waybar (переопределяет `~/.config/waybar/style.css`)
- `kitty.conf` — цветовая схема kitty (переопределяет `~/.config/kitty/current-theme.conf`)
- `gtk.theme` — имя GTK-темы (просто текстовый файл с одной строкой, например `Catppuccin-Mocha-Standard-Blue-Dark`)
- `wallpaper.jpg` — обои

Пример для `catppuccin-mocha` (остальные темы делаете по аналогии — можно взять готовые палитры, например с https://github.com/catppuccin/kitty и https://github.com/catppuccin/waybar, это официальные наборы под эти же программы, просто копируете нужный `.conf`/`.css` файл в соответствующую папку темы):
```
sudo xbps-install -S catppuccin-gtk-theme 2>/dev/null || true
```
(если пакета темы GTK нет в репозитории — скачайте вручную с https://github.com/catppuccin/gtk/releases и распакуйте в `~/.local/share/themes/`).

### 22.2 Скрипт-переключатель
```
cat > ~/.config/theme-switcher/apply-theme.sh << 'EOF'
#!/bin/sh
set -e
THEME="$1"
DIR="$HOME/.config/theme-switcher/themes/$THEME"

if [ -z "$THEME" ] || [ ! -d "$DIR" ]; then
    echo "Использование: apply-theme.sh <имя_темы>"
    echo "Доступные темы:"
    ls "$HOME/.config/theme-switcher/themes"
    exit 1
fi

cp "$DIR/waybar.css" "$HOME/.config/waybar/style.css"
cp "$DIR/kitty.conf" "$HOME/.config/kitty/current-theme.conf"
cp "$DIR/wallpaper.jpg" "$HOME/.config/niri/wallpapers/current.jpg"

if [ -f "$DIR/gtk.theme" ]; then
    GTK_THEME_NAME=$(cat "$DIR/gtk.theme")
    gsettings set org.gnome.desktop.interface gtk-theme "$GTK_THEME_NAME" 2>/dev/null || true
    sed -i "s/^gtk-theme-name=.*/gtk-theme-name=$GTK_THEME_NAME/" "$HOME/.config/gtk-3.0/settings.ini" 2>/dev/null || true
fi

# Перезапускаем то, что не подхватывает изменения на лету
pkill -SIGUSR2 waybar 2>/dev/null || pkill waybar && waybar &
pkill swaybg 2>/dev/null || true
swaybg -i "$HOME/.config/niri/wallpapers/current.jpg" -m fill &
kitty @ set-colors -a -c "$HOME/.config/kitty/current-theme.conf" 2>/dev/null || true

echo "Применена тема: $THEME"
EOF
chmod +x ~/.config/theme-switcher/apply-theme.sh
```

### 22.3 Графический выбор темы через fuzzel (та самая "визуальная настройка" по кнопке)
```
cat > ~/.config/theme-switcher/pick-theme.sh << 'EOF'
#!/bin/sh
THEME=$(ls "$HOME/.config/theme-switcher/themes" | fuzzel --dmenu --prompt "Тема: ")
[ -n "$THEME" ] && "$HOME/.config/theme-switcher/apply-theme.sh" "$THEME"
EOF
chmod +x ~/.config/theme-switcher/pick-theme.sh
```
Добавьте бинд в `config.kdl`:
```kdl
binds {
    Mod+T { spawn "/home/youruser/.config/theme-switcher/pick-theme.sh"; }
}
```
Нажатие `Mod+T` — всплывает список тем через fuzzel, выбираете стрелками/вводом — применяется сразу вся тема (панель, терминал, GTK, обои). Это и есть ваш "графический конфигуратор", просто реализованный без внешней зависимости от готового проекта, которого под niri в нужном виде не существует.

Чтобы добавить новую тему в будущем — просто создаёте новую папку в `~/.config/theme-switcher/themes/<имя>/` с теми же 4 файлами, скрипт подхватит её автоматически (никакой регистрации не требуется).

---

## Часть 23. Подключение Wi-Fi (BE301) в готовой системе

Раз NetworkManager уже запущен (Часть 5.10) и апплет `nm-applet` в трее Waybar (Часть 12):
- Кликните по иконке сети в трее → выберите сеть → введите пароль. Апплет создаёт полноценный GUI-диалог.
- Либо через терминал:
  ```
  nmcli device wifi list
  nmcli device wifi connect "ИмяСети" password "пароль"
  ```
Если Wi-Fi адаптер вообще не отображается в `nmcli device status` — вернитесь к диагностике в Части 7.3 (прошивка BE301).

---

## Часть 24. Итоговый список автозапуска сессии (что стартует при входе)

Всё уже прописано в `spawn-at-startup` внутри `~/.config/niri/config.kdl` (Часть 10.5):

| Что | Зачем |
|---|---|
| `pipewire`, `pipewire-pulse`, `wireplumber` | звук |
| `waybar` | панель |
| `dunst` | уведомления |
| `nm-applet --indicator` | сеть в трее |
| `blueman-applet` | Bluetooth в трее |
| `swaybg` | обои |
| `wl-paste --watch cliphist store` | сбор истории буфера |
| `swayidle -w` | гашение экрана по простою + логика крышки (через `switch-events`) |
| `polkit-kde-authentication-agent-1` | графические запросы пароля от sudo-действий GUI-программ |
| `xwayland-satellite` | поддержка X11-приложений |

Не забудьте установить `swaybg`:
```
sudo xbps-install -S swaybg
```

---

## Часть 25. Шпаргалка горячих клавиш

| Комбинация | Действие |
|---|---|
| `Mod+Return` | Открыть kitty |
| `Mod+D` | Быстрый поиск/запуск (fuzzel) |
| `Mod+Shift+D` | "Пуск" — грид приложений с категориями (nwg-drawer) |
| `Mod+E` | Dolphin (файлы) |
| `Mod+B` | Firefox |
| `Mod+Q` | Закрыть окно |
| `Mod+F` | Развернуть колонку |
| `Mod+Shift+F` | Полноэкранный режим |
| `Mod+V` | Плавающее окно вкл/выкл |
| `Mod+←/→/↑/↓` или `H/L/K/J` | Навигация между окнами/колонками |
| `Mod+1..5` | Переключение рабочего стола |
| `Mod+Alt+L` | Заблокировать экран |
| `Mod+Shift+P` | Меню выхода (wlogout) |
| `Mod+Shift+E` | Выйти из сессии niri |
| `Mod+T` | Выбор темы оформления |
| `Print` | Скриншот области (в буфер) |
| `Shift+Print` | Скриншот всего экрана |
| `Mod+Print` | Запись экрана (область) |
| `Mod+Shift+V` | История буфера обмена |
| Крышка закрыта | Музыка играет → гаснет экран; музыки нет → сон |
| 3 мин простоя | Гаснет экран (без блокировки) |

---

## Часть 26. Быстрый troubleshooting

- **Чёрный экран после входа в niri.** Проверьте, что стоит `mesa-dri` (Часть 10.1) — без него на некоторых системах именно так и происходит. Также проверьте `niri msg outputs` через TTY2 (`Ctrl+Alt+F2`) на предмет того, что монитор вообще определился.
- **os-prober не нашёл Windows при `grub-mkconfig`.** Убедитесь, что Fast Startup в Windows правда выключен (Часть 1.3) и что при разметке диска (Часть 4) вы **не** отформатировали существующий EFI/NTFS раздел Windows. Проверьте вручную: `os-prober` (от root, отдельно, без grub-mkconfig) — должен вывести строку с путём к `bootmgfw.efi`.
- **Wi-Fi BE301 не виден вообще (`iwlwifi` не грузится, нет даже интерфейса в `ip a`).** `dmesg | grep -i iwlwifi` — если модуль вообще не находит устройство, проверьте `lspci -nnk | grep -i network` на предмет того, что чип определяется на уровне PCI. Если PCI видит, а модуль — нет: `modprobe iwlwifi` вручную и смотрите ошибку.
- **PipeWire не стартует / нет звука вообще.** `pactl info` (пакет `pulseaudio-utils` не обязателен, `wpctl status` из `wireplumber` тоже покажет граф устройств). Проверьте, что `dbus` включён как сервис (Часть 5.10) — PipeWire у активируется через DBus-сокет.
- **deadbeef не видит USB DAC в ALSA-профиле.** Отключите/включите USB DAC заново физически, обновите список устройств в настройках вывода deadbeef. Если PipeWire успел "прибрать" устройство к рукам — эксклюзивный ALSA-плагин deadbeef всё равно способен вытеснить его при открытии потока напрямую через `hw:`, но иногда требуется секундная задержка.
- **greetd не показывает список пользователей / падает в консоль.** Проверьте логи: `sudo sv status greetd` и `journalctl` недоступен без systemd — смотрите `/var/log/greetd.log`, если такого нет — временно замените `command` в `/etc/greetd/config.toml` на просто `niri --session`, чтобы убедиться, что сама сессия стартует без greeter'а, а проблема именно в regreet/cage.
- **niri видит только английскую раскладку, Alt+Shift не переключает.** Убедитесь, что в `input { keyboard { xkb { ... } } }` именно `layout "us,ru"` (через запятую, один блок), а не два отдельных `layout`.

---

Готово — это полный путь от чистой флешки до рабочего стола. Двигайтесь по частям строго по порядку (особенно разметка диска и GRUB — не торопитесь на этих шагах, дважды сверяйте номера разделов через `lsblk`/`parted print` перед любой операцией записи). Если на каком-то шаге команда выдаст ошибку — не идите дальше "на автомате", разберитесь в сообщении или напишите мне, что именно вышло, и я поправлю именно этот шаг.
