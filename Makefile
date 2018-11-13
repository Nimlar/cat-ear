BUILD_DIR = .
TARGET = esp8266:esp8266:nodemcuv2
TTY = /dev/ttyUSB0
all : build install

build :
	arduino-cli compile --fqbn $(TARGET) $(BUILD_DIR)

$(TTY) :
	@echo "$(TTY) doesn't exist"
	@echo "please plug your esp"
	@false

install : | $(TTY)
	arduino-cli upload -p $(TTY) --fqbn $(TARGET) $(BUILD_DIR)
