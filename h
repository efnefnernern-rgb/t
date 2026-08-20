sudo tee /etc/greetd/config.toml > /dev/null << 'EOF'
[terminal]
vt = 1
 
[default_session]
command = "regreet --cage-cmd 'niri --session'"
user = "greeter"
EOF
