# adf435xcfg
This is a command line tool to allow control of the Analog Devices ADF435x devices. It
is essentially a quick and dirty port of the Python application [pyadf435x ](https://github.com/jhol/pyadf435x) to C,
and instead of using a microcontroller to form the interface between the PC and the ADF435x, it instead uses a CH341A
since that is what I had available.

Note that there are many optional settings for the ADF435x that are not exposed. If there are any that you would really like
to be able to change at the command line, please don't hesitate to contact me and i'll see what I can do, or alternatively change
the source yourself and send me a pull request and i'll merge the changes.

Hopefully this will be of use to someone else!

## CH341 <-> ADF435x Connections

|  CH341 Pin  |  ADF4350/1 Pin  |
|  ---------  |  -------------  |
|  CS0        |  LE             |
|  SCK        |  CLK            |
|  MOSI       |  DAT            |

## Binaries
A binary compiled for win64 is included. I've not built or tested this on Linux, but since it uses [libusb](https://libusb.info/), it should be possible to build this on any platform that it supports.

## Use

### Get the available options:
~~~
.\adf435xcfg.exe --help

Usage: adf435xcfg.exe <options>

  -f --frequency <freq>           Set the output frequency to <freq> Hz

  -s --sweep                       Enable sweep mode

  -l --low <freq>                  Set the sweep lower frequency to <freq> Hz

  -h --high <freq>                 Set the sweep upper frequency to <freq> Hz

  -r --resolution <freq>           Set the sweep step frequency to <freq> Hz

  -d --delay <delay>               Set the sweep mode step delay to <delay> milliseconds

  -? --help                        Display help
~~~

### Command to generate a single frequency
~~~
.\adf435xcfg.exe --freq 900000000
~~~

### Command to sweep a frequency range of 800MHz to 1GHz with 100KHz steps, delay of 1ms per step
~~~
.\adf435xcfg.exe --sweep --low 800000000 --high 1000000000 --resolution 100000 --delay 1
~~~
