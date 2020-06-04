# Embedded Master Password
[Maarten Billemont's](http://www.lhunath.com/) original [Master Password](https://masterpassword.app/) algorithm ported to the embedded
world providing a physical manifestation using [Paul Stoffregen's](https://www.pjrc.com/) amazing [Teensy 4.x](https://www.pjrc.com/store/teensy40.html) for the password generator and [Adafruit's](https://learn.adafruit.com/) easy-to-use [Feather Huzzah ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/overview) as a secure web server for the UI.

## Why?
Master Password already comes in every possible flavour, why would you want to do this? And anyway, you can't possibly run crypto code
on those little things, they don't have enough ... everything!

Ok, so here are my reasons:

1. Primarily I wanted a custom hardware solution with the algorithm running on it so that I could be reasonably sure it wasn't compromised in any way. Once the firmware is uploaded to the hardware devices, it is much more attack proof than code on a PC or mobile device. Without physical access to the hardware, updating the firmware is difficult, if not impossible.
2. Understanding that there is nothing in the code that is nefarious. Of course that really only applies to me since I typed (almost) every line of code in just to make sure. You'll have to take my word for it ... well, actually you won't because you can see it all here in this repo and make sure there is nothing that can compromise your security. I also didn't take any standard libraries, every algorithm was re-implemented from scratch, and they all come with unit tests to verify the algorithms.
3. Running the scrypt algorithm on a tiny MCU is a challenge, but that's part of what makes writing code such a pleasure!


## Hardware requirements

* PJRC Teensy 4.0 or 4.1 (I recommend a [4.1 with 8MB PSRAM](https://www.pjrc.com/store/teensy41.html) soldered on to the back. If you're scared of doing this and, I have to admit, it's quite fiddly, you can still use the 4.0 or 4.1 without it, the algorithm will be a little slower to login though.)
* Adafruit [Feather Huzzah ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/overview)

## Software requirements

* [Arduino IDE 1.8.12](https://www.arduino.cc/en/Main/Software) or later
* [Teensyduino 1.52](https://www.pjrc.com/teensy/td_download.html) or later
* [Arduino ESP8266 2.7.0](https://arduino-esp8266.readthedocs.io/en/2.7.1/installing.html) or later

## Getting EMPW firmware on your Teensy board

* Download this repo to your computer
* Run the Arduino IDE
* Open the EmbeddedMasterPassword.ino sketch
* Select your board (Teensy 4.0 or 4.1)
* Press the "Upload" button. The IDE should compile and upload the firmware to the Teensy device
* Open the serial monitor. You should be presented with a screen similar to

```
### Embedded Master Password v0.1.5 ###
Build date Jun  3 2020 22:34:17
Running on Teensy 4.0
This implementation, Copyright © 2020, Gazoodle (https://github.com/gazoodle)
Algorithm, Copyright © 2011-2020, Maarten Billemont (https://masterpassword.app/)

For instructions use command `help`
```

Type the following commands into the Serial monitor input box

```
adduser user; login user,password; addsite example.com; site example.com
```

The serial monitor should show something like this

```
Calculating ... 0%
Calculating ... 5%
    :
Calculating ... 96%
User [user] logged in
TOKEN:538982992
add site [example.com]
password: ZedaFaxcZaso9*
```

If you see the password as `ZedaFaxcZaso9*` then it is working as expected. Go ahead and play with the interface

## EMPW Command Reference

Using your new Embedded Master Password device is easy. Simply plug the USB device into your PC, launch your favourite serial terminal program, connect to your USB serial port and type `help`. 

### Adding a user, a site and getting a password

```
adduser Robert Lee Mitchell; login Robert Lee Mitchell, banana colored duckling; addsite masterpasswordapp.com; site masterpasswordapp.com
```
gives
```
User [Robert Lee Mitchell] logged in
TOKEN:869630000
add site [masterpasswordapp.com]
password: Jejr5[RepuSosp
```

The `adduser` command adds the named user to the persistent user list stored on the device. The `login` command provides the user and password to start the Master Password login process. Depending on your device, this can take up to 20 seconds to complete the generation of the master key. You can ignore the token that is returned from the UI, this is used by the web server UI to support multiple concurrent users. The `addsite` command adds the website to the persistent list of sites associated with the user 'Robert Lee Mitchell', and finally the `site` command will generate the password for you to add a named user to the system.

### Updating the password type
```
settype masterpasswordapp.com, phrase; site masterpasswordapp.com
```
gives
```
password: jejr quv cabsibu tam
```

The `settype` command sets the site password for 'masterpasswordapp.com' to 'phrase'. You can use any of the (case insensitive) types below:
* Maximum
* Long
* Medium
* Basic
* Short
* PIN
* Name
* Phrase

### Adding a site username
```
sethasusername masterpasswordapp.com, true; site masterpasswordapp.com
```
gives
```
user: wohzaqage
password: jejr quv cabsibu tam

```

The `sethasusername` command instructs EMPW to generate a site user name in addition to a password. The `true` parameter enables this feature, and anything else will disable it.

### Adding a recovery phrase
```
sethasrecovery masterpasswordapp.com, true; site masterpasswordapp.com
```
gives
```
user: wohzaqage
password: jejr quv cabsibu tam
recovery: xin diyjiqoja hubu
```

The `sethasrecovery` command instructs EMPW to generate a recovery phrase in addition to a password. The `true` parameter enables this feature, and anything else will disable it.

### Adding challenge/response words
```
addanswer masterpasswordapp.com, maiden; site masterpasswordapp.com
```
gives
```
user: wohzaqage
password: jejr quv cabsibu tam
recovery: xin diyjiqoja hubu
recovery[maiden]: din riqxocera qodo
```

The `addanswer` command instructs EMPW to generate a recovery answer with the keyword `maiden` (as the most significant word in a question such as "What is your mother's maiden name?"). The `removeanswer` command will remove a saved answer.

### Updating the site counter

```
setcounter masterpasswordapp.com, 2; site masterpasswordapp.com
```
gives
```
user: wohzaqage
password: gor juckakafe sigi
recovery: xin diyjiqoja hubu
recovery[maiden]: din riqxocera qodo

```

The `setcounter` command sets the site counter for 'masterpasswordapp.com' to 2. You can set this to any number between 1 and 254 (an implementation decision to reduce memory usage and based on practical limits. This could be changed if required). Notice how the username and recovery phrases are unaffected by the site counter.

## Repo layout

```
/       -   Repo root, contains the EmbeddedMasterPassword.ino sketch file, LICENSE and README.md
/cli    -   A command line interface test program.
                Build the CLI using:
                    cd cli
                    make
                    ./cli
/tests  -   Unit tests for the various algorithms
                Build the unit tests using:
                    cd tests
                    make
                    ./tests
/src/app    -   EMPW client application code (command processor etc)
/src/lib    -   EMPW algorithm and supporting classes
                    SHA256, HMAC, PBKDF2, scrypt and MPW
```

# Coming next

A secure webserver built on Arduino ESP8266 to provide a multi-user web UI that can be hosted in your home, and then accessed via a VPN to provide access to your Embedded Master Password device anywhere in the world!



Using Semantic versioning https://semver.org/

# License

https://www.gnu.org/licenses/gpl-3.0.html
