sudo tee -a /etc/greetd/regreet.toml > /dev/null << 'EOF'
[background]
path = "/usr/share/backgrounds/login.jpg"
fit = "Cover"
 
[GTK]
application_prefer_dark_theme = true
theme_name = "Adwaita-dark"
icon_theme_name = "Papirus-Dark"
EOF
