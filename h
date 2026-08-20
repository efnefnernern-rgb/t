sudo tee -a /etc/elogind/logind.conf > /dev/null << 'EOF'
HandleLidSwitch=ignore
HandleLidSwitchExternalPower=ignore
HandleLidSwitchDocked=ignore
EOF
