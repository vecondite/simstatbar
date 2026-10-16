# simstatbar
A (WIP) simple async status bar runner (like dwmblocks!)

> Config location: ```~/.config/simstatbar/config.ini```

> Example config:
```ini
[settings]
delimiter = | 
blocks = date, date2

[date]
interval = 1000
command = "date"

[date2]
interval = 5000
command = "date"
```

> Build:
```sh
mkdir -p build && g++ -std=c++17 -Wall -Wextra -O2 src/main.cpp -o build/main -lX11 -pthread && ./build/main
```
