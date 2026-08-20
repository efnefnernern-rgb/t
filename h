{
    "layer": "top",
    "position": "top",
    "height": 34,
    "spacing": 4,
    "modules-left": ["custom/launcher", "niri/workspaces", "niri/window"],
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
     
    "custom/launcher": {
        "format": "",
        "on-click": "nwg-drawer -r",
        "tooltip": false
    },   
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
