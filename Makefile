install-arduino-cli:
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

install-deps:
	./bin/arduino-cli.exe core update-index --config-file ./arduino-cli.json
	./bin/arduino-cli.exe core download deauther:esp8266 --config-file ./arduino-cli.json
	./bin/arduino-cli.exe core install deauther:esp8266 --config-file ./arduino-cli.json

clean:
	rm -rf ./build

build: esp8266/*
	./bin/arduino-cli.exe compile --fqbn deauther:esp8266:generic --build-path ./build --warnings all --config-file ./arduino-cli.json ./esp8266/esp8266.ino

flash: build
	./bin/arduino-cli.exe upload --fqbn deauther:esp8266:generic --port COM3 --verbose --input-dir ./build/ --config-file ./arduino-cli.json ./esp8266/esp8266.ino

