# ESP8266 WiFi Deauther

## Basic configuration

### Windows
To compile this project you need MSYS2, which can be downloaded here: https://www.msys2.org/

After installing MSYS2 you need to run the following command

```bash
make install-arduino-cli
```

to install Arduino CLI. 

## Make commands

`make install-arduino-cli` - installs Arduino CLI in ./bin/ directory

`make install-deps` - installs libraries and boards

`make build` - compiles the project

`make flash` - compiles and flashes the project into ESP8266 **connected on port COM3**
