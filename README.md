# Compass

Compass is a web browser forked from [KDE Falkon](https://www.falkon.org/), using the QtWebEngine rendering engine.

## Downloads

See the [GitHub repository](https://github.com/bungamungil/compass) for source and releases.

## Building

```sh
mkdir build && cd build
cmake ..
make && make install
```

### Install to custom prefix

When installing Compass to a custom prefix, you may need to adjust the `XDG_DATA_DIRS` environment variable.

```sh
# Build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/compass

# Run
export XDG_DATA_DIRS="$HOME/compass/share:$XDG_DATA_DIRS"
$HOME/compass/bin/compass
```

## Contributing

Want to contribute? Open an issue or pull request on [GitHub](https://github.com/bungamungil/compass).

#### Reporting bugs

You can report bugs or feature requests on the [GitHub issue tracker](https://github.com/bungamungil/compass/issues). Before reporting, please make sure your issue isn't already reported.

## Credits

Compass is forked from [KDE Falkon](https://www.falkon.org/), licensed under GPL-3.0-or-later. See `src/lib/data/html/copyright` for full third-party attributions.

The Compass application icon is from the [Papirus Icon Theme](https://github.com/PapirusDevelopmentTeam/papirus-icon-theme), licensed under GPL-3.0.
