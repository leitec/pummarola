pummarola
=========

Pummarola is a Twitter client for old Macs. Requires a 68020 or higher, and System 7 or higher. It also compiles and runs from Linux, OS X, A/UX and other UNIX-ish OSs.

Status
------
Currently very rudimentary, and a lot of the code is not particularly good. This is something I will work on: I intend to add a proper GUI, and clean up the library code as much as possible.

Development
-----------
Contains code from other repositories:
 * json-parser: https://github.com/udp/json-parser
 * tinyhttp: https://github.com/mendsley/tinyhttp
  
and requires my PolarSSL port for MacOS to build: https://github.com/leitec/polarssl-macos

To build with CodeWarrior Pro 1, fetch the archive, and then fetch the PolarSSL archive into the main archive's directory. Follow the build instructions for polarssl-macos. Then, unpack the Pummarola project file and build the desired target (PPC/68k/fat).

This was developed primarily under Classic in Tiger, so that `git` is available. All source code must be set to UNIX-style line endings in CodeWarrior, and the filetypes must be set so that CW can see them:

    find . -type f -name '*.[chs]' -print0 | xargs -0 /Developer/Tools/SetFile -c CWIE -t TEXT

The libraries `libpummarola` and `liboauth` are made for what I want to do with Pummarola, but should be generic enough to work in other projects if you so desire.

Usage
-----

The client only includes a very rudimentary command line interface.

    tweet <text> -- send a new tweet
    get <user> -- get a user's timeline (recent messages)
    home -- get your home timeline
    count <count> -- how many messages to fetch for get and home
    quit -- exit Pummarola
    
**Important:** in MacOS, always use `quit` to exit. If you exit by closing the window, MacTCP will be left in an unstable state. There is no quit message; after you type quit, it is done and you can close the window.

An accurate clock is necessary. OAuth requires the request's timestamp to be within a small tolerance, so make sure to set the clock on your machine via network time. On System 7 I use Vremya for NTP with good results. Due to the way the UNIX timestamp is calculated, the time zone must be accurately set as well. See `time.c` in `libcompat` for implementation details.

**Authentication:** when first run, Pummarola will print a long URL. This has to be entered in a sufficiently modern browser that can log into Twitter. Last year I used Classilla, but it may have changed since then. While tedious, the best option is to enter manually on another computer. You will have to authorize Pummarola to access your account, and will get a PIN. Enter this PIN into Pummarola. It will create a file `secrets` in the folder. You can move this file around, but as the name implies it contains the necessary details to access your account, so be careful.

**GUI:** A GUI is definitely planned. What makes Mac software truly Mac software is a proper interface. This is just a hacked-together proof of concept :-)
