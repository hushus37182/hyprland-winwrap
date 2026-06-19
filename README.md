## hyprland-winwrap
### Display any window as a background in Hyprland
#####fixed the bug with freezes on workspace swaps and window killing that hyprwinwrap has
---
this plugin renders window textures on background, which can be used to bypass live wallpapers memory leaks (e.g. run mpv on bg) and it's physically unable to kill a window by itself.

tested on hyprland 0.54.3

#### Installation
```bash
hyprpm add https://github.com/hushus37182/hyprland-winwrap.git
hyprpm enable hyprland-winwrap
```

to run windows on background, add following to your hyprland.conf:
   ```
    plugin{
        hyprland-winwrap{
            title=your_window_title
                # or
            class=your_window_class
        }
    }
   ```

the window also must be opened somewhere. \n
it could be a special workspace or some workspace 99 
