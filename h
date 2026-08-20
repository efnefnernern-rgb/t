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
