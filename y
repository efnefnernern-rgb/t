sudo tee /usr/local/bin/cliphist-fuzzel-wrapper > /dev/null << 'EOF'
#!/bin/sh
cliphist list | fuzzel --dmenu | cliphist decode | wl-copy
EOF
