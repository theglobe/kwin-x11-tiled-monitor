# kwin-x11-tiled-monitor

Make KDE Plasma 5 (KWin on X11) treat a **tiled monitor** as one screen, so
**maximize fills the whole monitor** instead of one half.

## The problem

Some high-resolution monitors (5K and 8K models such as the HP Z27q, Dell
UP2715K, Dell UP3218K, or 4K monitors in MST mode) are driven as two
"tiles", over two DisplayPort cables or two MST streams. The X server then
shows two RandR outputs, for example:

```
DP-4 connected 2560x2880+0+0
DP-2 connected 2560x2880+2560+0
```

KWin 5.27 on X11 makes one screen per output. So:

- maximizing a window fills only the half it is on;
- quick-tiling, window placement and "move to screen" all act per half;
- fullscreen (F11, video players) may already cover both halves.

xrandr can group the outputs into one monitor (`xrandr --setmonitor`, and
recent X servers do it automatically from the EDID TILE property), but KWin
5.27 ignores RandR monitors. See [KDE bug 427545](https://bugs.kde.org/show_bug.cgi?id=427545).

## How this fixes it

`kwin-no-randr.so` is a tiny `LD_PRELOAD` library. Inside the `kwin_x11`
process only, it makes `xcb_get_extension_data()` report the RANDR
extension as missing. KWin then falls back to a single screen the size of
the whole X screen (e.g. 5120x2880). Other programs are untouched.

After this, maximize fills the full monitor, and quick-tile left/right lands
exactly on the two tiles.

## Requirements

- KDE Plasma 5 on **X11** (tested with KWin 5.27.11, Kubuntu 24.04,
  NVIDIA 595 proprietary driver, HP Z27q over two DP cables)
- KWin started by systemd (`plasma-kwin_x11.service`; the default on
  Plasma 5.25+). Check with `systemctl --user status plasma-kwin_x11`.
- `gcc` and `make`. No X or xcb development headers are needed.

Not for Wayland, and not tested with Plasma 6.

## Install

```sh
make install
systemctl --user restart plasma-kwin_x11
```

Your open windows stay open while KWin restarts. Check it worked:

```sh
qdbus org.kde.KWin /KWin supportInformation | grep -A4 '^Number of Screens'
```

You should see `Number of Screens: 1` and the full resolution.

## Uninstall

```sh
make uninstall
systemctl --user restart plasma-kwin_x11
```

## Limitations

Because KWin no longer sees RandR:

- **Only use this with a single (tiled) monitor.** With extra monitors,
  KWin would treat all of them as one big screen.
- KWin does not react to display changes (hotplug, resolution change, the
  monitor falling back to one tile) until it restarts.
- **Night Color** does not work (it uses RandR gamma).
- KWin reports a 60 Hz refresh rate.
- Plasma's desktop and panels are a separate process and still see two
  screens; only window management changes.

## Alternatives

- [hax11](https://github.com/CyberShadow/hax11) is a more general
  `LD_PRELOAD` library that rewrites X11 replies; its `JoinMST=1` option
  presents tiled monitors as one. It was mainly written for games, but can
  be loaded into a window manager too.

## License

GPL-3.0-or-later, see [LICENSE](LICENSE).
