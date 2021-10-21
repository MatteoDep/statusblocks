# statusblocks
This project is a fork of dwmblocks that I'm modifying to make it work with
other bars that read from standard input like dzen2 and lemonbar.
To date it has only been tested for lemonbar with bspwm.

## Installation

Clone the repo, navigate inside and `sudo make clean install`.
To uninstall use `sudo make uninstall`.

## Configuration
In `example_config/statusblocksrc` you can find an example
configuration file that you can put in your
`XDG_CONFIG_HOME/statusblocks/` (usually `~/.config/statusblocks/`)
and adjust for your needs.
The configuration file is simply a shell file which, at least, needs to include
the definition of a `panel_command` function that takes the bar name as a
parameter.
To retrieve information from the window manager a FIFO file can be used.
For example in bspwm put this in your bspwmrc:

```
# feed fifo file to inform statusbar of wm changes
PANEL_FIFO=/tmp/panel-fifo
[ -e "$PANEL_FIFO" ] && rm "$PANEL_FIFO"
mkfifo "$PANEL_FIFO"
bspc subscribe report > "$PANEL_FIFO" &
```

To create the statusbar `bar1` you need to create the file
`XDG_CONFIG_HOME/statusblocks/bar1` which specifies the bar modules or blocks.
For the syntax of this file look at the `examples` folder.
Then you need to modify the `panel_command` function to accept the parameter
`bar1`.

## Usage
you can simply launch the bars in your config directory with

```
launchbar bar1 bar2
```

To update the modules (blocks) you can send the correspondent signal (as
specified in the bar file) to the statusblocks process.
For example if the signal is 10 you can put in your scripts `pkill -RTMIN+10
statusblocks`.
See [dwmblocks](https://github.com/torrinfail/dwmblocks) for more informations.

## Dependencies
+ your statusbar of choice

## CREDITS
+ [dwmblocks](https://github.com/torrinfail/dwmblocks)
+ [bspwm](https://github.com/baskerville/bspwm) (took inspiration from
  examples)
