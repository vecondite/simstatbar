# simstatbar
A simple async status bar runner (like dwmblocks!)

> Config location: ```~/.config/simstatbar/config.ini```

> Example config:
```ini
[settings]
delimiter = ' | ' 
blocks = memory, date

[date]
interval = 1000
command = "date"

[memory]
interval = 5000
command = "~/scripts/memory.sh"
```

> Build:
```sh
make && sudo make install
```
