### About the fork
Adds cmake and Visual Studio compatibility, stdin/stdout support output to encode and play LDAC files
in players that support it, such as foobar2000 - and float wave output to maximize quality.

# LDAC decoder

this is an early stage, jet functional LDAC audio stream decoder.

Shout-out to [@Thealexbarney](https://github.com/Thealexbarney) for the heavy lifting.
LDAC is basically a stripped down, streaming only ATRAC9.

## Linux

#### Requirements

```sh
$ sudo apt install libsndfile1-dev libsamplerate0-dev
```

#### Build
```sh
$ make
```

#### Usage
see ldacdec.c for example usage

#### ldacdec
takes an LDAC stream and decodes it to WAV

#### ldacenc
uses Android LDAC encoder library to create LDAC streams from audio

## Windows
Use cmake to build the project in Visual Studio.
#### Requirements
Needs [getopt-win](https://github.com/ludvikjerabek/getopt-win) in a parallel directory to compile under Windows.
Use *vcpkg* to install *libsndfile* and *libsamplerate* visible to cmake on Windows (first install *pkg-config* if you don't have it yet).
#### Known issue
Cmake under Linux will overwrite the original Makefile.
#### TODO
These are raw LDAC streams that only support rudimentary tagging and will not play on smartphones. Maybe they can be wrapped in RIFF to make .at9 files, who knows.
#### Foobar2000 configuration options
<img src="readme pics/foobar encoding.png" width="42%">
<img src="readme pics/foobar decoding.png" width="56%">
