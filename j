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
