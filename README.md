# statusblocks
This project is a fork of dwmblocks that I'm modifying to make it work
with other bars that read from standard input like dzen2 and lemonbar.
To date it has only been tested for lemonbar with bspwm.

## Installation

Clone the repo, navigate inside and do `sudo make clean install`.
To uninstall do `sudo make uninstall`.

## Configuration
In `example_config/statusblocksrc` you can find an example configuration file that
you can put in your `XDG_CONFIG_HOME/statusblocks/` or
`~/.config/statusblocks/` and adjust for your needs.
The configuration file is simply a shell script which will need to provide some
variables and 2 functions.
To retrieve information from the window manager a FIFO file can be used, in the
example bspwm is used.
The blocks for a statusbar can then be specified in simple textfiles like
`example_config/bar1`.

## Usage
you can simply launch the bars in your config directory with

```
launchbar bar1 bar2
```

or specifying the full path

```
launchbar /path/to/bar1 ...
```

To update the modules (blocks) you can send the correspondent signal (as
specified in the bar file) to the statusblocks process.
For example if the signal is 10 you can put in your scripts `pkill --RTMIN+10
statusblocks`.
See [dwmblocks](https://github.com/torrinfail/dwmblocks) for more informations.

## Dependencies
+ [xdo](https://github.com/baskerville/xdo)
+ your statusbar of choice

## CREDITS
+ [dwmblocks](https://github.com/torrinfail/dwmblocks)
+ [bspwm](https://github.com/baskerville/bspwm) (took inspiration from
  examples)
