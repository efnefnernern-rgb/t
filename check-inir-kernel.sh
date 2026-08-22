#!/usr/bin/env bash
# Проверка kernel config под iNiR (Niri + Quickshell стек) на Gentoo
# Использование: ./check-inir-kernel.sh [путь_к_.config]
set -uo pipefail

CONFIG="${1:-}"
if [[ -z "$CONFIG" ]]; then
    for c in /usr/src/linux/.config /boot/config-$(uname -r) /proc/config.gz; do
        [[ -f "$c" ]] && { CONFIG="$c"; break; }
    done
fi
[[ -z "$CONFIG" ]] && { echo "Конфиг не найден. Укажи путь явно: $0 /usr/src/linux/.config"; exit 1; }

if [[ "$CONFIG" == *.gz ]]; then
    READ() { zcat "$CONFIG"; }
else
    READ() { cat "$CONFIG"; }
fi

check() {
    local opt="$1" recommend="$2" desc="$3"
    local status line

    if line=$(READ | grep -E "^${opt}="); then
        status="${line#*=}"
    elif READ | grep -q "^# ${opt} is not set"; then
        status="OFF"
    else
        status="MISSING"
    fi

    local mark
    case "$status" in
        y) mark=$'\033[32m[built-in]\033[0m' ;;
        m) mark=$'\033[36m[module]\033[0m' ;;
        OFF) mark=$'\033[31m[DISABLED]\033[0m' ;;
        MISSING) mark=$'\033[31m[NOT FOUND]\033[0m' ;;
        *) mark="[$status]" ;;
    esac

    printf "  %-30s %-42s %b\n" "$opt" "$desc" "$mark"

    if [[ "$recommend" == "m" && "$status" != "m" && "$status" != "y" ]]; then
        printf "      \033[33m-> рекомендуется как MODULE (=m)\033[0m\n"
    elif [[ "$status" == "OFF" || "$status" == "MISSING" ]]; then
        printf "      \033[33m-> нужно включить (y или m, см. рекомендацию выше)\033[0m\n"
    fi
}

echo "=== Проверка kernel config под iNiR/Niri ==="
echo "Файл: $CONFIG"
echo

echo "--- Графика / DRM (built-in) ---"
check CONFIG_DRM y "DRM core"
check CONFIG_DRM_KMS_HELPER y "KMS helper"
check CONFIG_DRM_FBDEV_EMULATION y "fbdev emulation"
check CONFIG_FB y "Framebuffer"

echo
echo "--- GPU-драйвер (MODULE рекомендуется) ---"
check CONFIG_DRM_I915 m "Intel GPU"
check CONFIG_DRM_AMDGPU m "AMD GPU"
check CONFIG_DRM_NOUVEAU m "Nouveau (открытый Nvidia)"
echo "  (проприетарный nvidia-drivers всегда собирается как module, отдельно от этого списка)"

echo
echo "--- Ввод / ydotool (built-in) ---"
check CONFIG_INPUT_EVDEV y "evdev"
check CONFIG_INPUT_UINPUT y "uinput (обязателен для ydotoold)"
check CONFIG_INPUT_LEDS y "input LEDs"

echo
echo "--- udev / devtmpfs (built-in) ---"
check CONFIG_DEVTMPFS y "devtmpfs"
check CONFIG_DEVTMPFS_MOUNT y "automount devtmpfs"

echo
echo "--- cgroups/namespaces для elogind/polkit (built-in) ---"
check CONFIG_CGROUPS y "cgroups"
check CONFIG_CGROUP_FREEZER y "cgroup freezer"
check CONFIG_NAMESPACES y "namespaces"
check CONFIG_UTS_NS y "UTS namespace"
check CONFIG_IPC_NS y "IPC namespace"
check CONFIG_PID_NS y "PID namespace"
check CONFIG_USER_NS y "user namespace"

echo
echo "--- Звук (ядро сама шина built-in, кодек-драйвер MODULE) ---"
check CONFIG_SND y "ALSA core"
check CONFIG_SND_HDA_INTEL m "HD-Audio Intel"
check CONFIG_SND_USB_AUDIO m "USB audio class (hotplug устройство)"

echo
echo "--- Яркость / ddcutil (built-in) ---"
check CONFIG_BACKLIGHT_CLASS_DEVICE y "backlight class"
check CONFIG_ACPI_VIDEO y "ACPI video (яркость на ноутах)"
check CONFIG_I2C y "I2C core"
check CONFIG_I2C_CHARDEV y "I2C /dev/i2c-* (нужно ddcutil)"

echo
echo "--- Bluetooth (ядро протокола built-in, драйвер контроллера MODULE) ---"
check CONFIG_BT y "Bluetooth core"
check CONFIG_BT_RFCOMM y "RFCOMM"
check CONFIG_BT_BNEP y "BNEP"
check CONFIG_BT_HIDP y "HIDP"
check CONFIG_BT_HCIBTUSB m "USB Bluetooth controller"

echo
echo "--- Wi-Fi (chip-драйвер MODULE, из-за прошивок) ---"
check CONFIG_CFG80211 y "cfg80211 core"
check CONFIG_MAC80211 y "mac80211 core"
echo "  ! Драйвер именно твоей wifi-карты (iwlwifi/ath9k/ath10k/rtw88 и т.д.)"
echo "    сделай =m — прошивка грузится позже, модуль безопаснее чем built-in."

echo
echo "=== Готово ==="
echo "Всё с пометкой DISABLED/NOT FOUND — включить в menuconfig."
echo "Всё с пометкой '-> рекомендуется как MODULE' — оставить =m, а не =y."
