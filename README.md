# EntStream

> “Any one who considers arithmetical methods of producing random digits is, of course, in a state of sin.” – John Von Neumann, 1951

If you have good reasons for streaming raw entropy (no health check, no whitening) from one or more [Infinite Noise](https://www.crowdsupply.com/13-37/infinite-noise-trng) TRNGs, here it is. Use at your own risk, and may God have mercy upon your soul.

Works on POSIX systems. Tested on Linux (musl libc) and MacOS. Windows not supported.

Not intended for cryptographic purposes!

## Features

- Stripped-down version of [libinfnoise](https://github.com/waywardgeek/infnoise)
- Highly-accurate scheduler
- Configurable sampling rate
- Multiple devices support
- Pub/Sub broker
- Stdout and/or ZeroMQ streaming
- Client examples in C and Python
- Pretty image generator

## Usage

```
Usage: entstream [OPTION...]
  -o, --enable-stdout        Print entropy to stdout
  -p, --enable-pubsub        Send entropy to a Pub/Sub broker
  -e, --endpoint=ADDRESS     Endpoint address
  -s, --serial=SERIAL        If set, connect only to the specifed device
  -r, --rate=RATE            If 0, will run as fast as possible

Help options:
  -?, --help                 Show this help message
      --usage                Display brief usage message
```

See `sub.c` and `sub.py` for example code on how to subscribe to the entropy feed and parse the resulting data.

## Building

### Alpine Linux

Enable the community repository in `/etc/apk/repositories`.

```
apk update
apk upgrade
apk add build-base
apk add linux-headers libftdi1-dev czmq-dev popt-dev
make && make install
```

### MacOS

Install [Homebrew](https://brew.sh/).

```
brew update
brew install libftdi czmq argp-standalone popt
make && make install
```
