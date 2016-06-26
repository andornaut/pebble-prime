# pebble-prime

Prime is a minimal and easy to read watchface for [Pebble smartwatches][pebble-smartwatches]. It is available in the [Pebble app store][download].

![Banner Image][banner-image]

## Features

- 12 or 24 hour time + day of the week + day of the month
- Background color indicates connection status
- Border size indicates battery level

## Instructions

```
# Install SDK (Debian / Ubuntu)
$ make sdk

# Install and run in the (basalt - Pebble Time) emulator
$ make install-basalt

# Install on your Pebble watch
$ make install PEBBLE_PHONE=${IP_ADDRESS}

# Take screenshots and create animated GIFs
$ make screenshots
```

## License

[MIT][license].

[banner-image]: /banner.png
[license]: /LICENSE
[download]: https://apps.getpebble.com/en_US/application/5764b04b3feaee913e000030
[pebble-smartwatches]: https://www.pebble.com
