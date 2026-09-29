### About the fork
Adds cmake and Visual Studio compatibility, and stdin/stdout support to encode and play LDAC files in players that support it, such as foobar2000.
#### Requirements
Needs [getopt-win](https://github.com/ludvikjerabek/getopt-win) in a parallel directory to compile under Windows.
Use *vcpkg* to install *libsndfile* and *libsamplerate* visible to cmake on Windows (first install *pkg-config* if you don't have it yet).
#### Known issue
Cmake under Linux will overwrite the original Makefile.
#### Foobar2000 configuration options
<img src="readme pics/foobar encoding.png">
<img src="readme pics/foobar decoding.png">

# LDAC decoder

this is an early stage, jet functional LDAC audio stream decoder.

Shout-out to [@Thealexbarney](https://github.com/Thealexbarney) for the heavy lifting.
LDAC is basically a stripped down, streaming only ATRAC9.

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
