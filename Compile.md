```sh
arduino-cli config init --config-file ./myconfig
```
eventually change path in ./myconfig

add in ./myconfig
```yml
board_manager:
        additional_urls:
                - http://arduino.esp8266.com/stable/package_esp8266com_index.json
                - https://dl.espressif.com/dl/package_esp32_index.json
```
Update core list with the esp832 and esp32
```sh
arduino-cli core update-index
```
install the core we need:

```sh
arduino-cli core install esp8266:esp8266
arduino-cli core install esp32:esp32
```

Now we can compile the sketch:

```sh
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
```
or
```sh
arduino-cli compile --fqbn esp32:esp32:esp32 .
```

