# simstatbar
A simple async status bar runner (like dwmblocks!)

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
make && sudo make install
```
