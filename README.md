# dwall

Much like `wall`, or more closely related in functionality, `rwall`, `dwall` broadcasts messages from your device to discord webhooks.

### Building

Dependencies:
- `libcurl`
- `nix`

Build: `nix build`

`result/bin/dwall` will be the resulting binary.

### Usage

```
Usage: result/bin/dwall [options] [<message>]

Write a message to discord.

Options:
    -n, --no-banner     do not print banner
    -w, --webhooks      list of comma separated webhooks to broadcast to

    -h, --help          display this help
    -v, --verbose       display debug output
    -V, --version       display version
```
