mkdir -p ~/.local/share/fonts/material-symbols
cd ~/.local/share/fonts/material-symbols

curl -LO "https://raw.githubusercontent.com/google/material-design-icons/master/variablefont/MaterialSymbolsOutlined%5BFILL,GRAD,opsz,wght%5D.ttf"
curl -LO "https://raw.githubusercontent.com/google/material-design-icons/master/variablefont/MaterialSymbolsRounded%5BFILL,GRAD,opsz,wght%5D.ttf"
curl -LO "https://raw.githubusercontent.com/google/material-design-icons/master/variablefont/MaterialSymbolsSharp%5BFILL,GRAD,opsz,wght%5D.ttf"

fc-cache -f ~/.local/share/fonts
