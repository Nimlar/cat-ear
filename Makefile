TARGET=esp32 esp82 x86test

BUILD_DIR[esp32] = cat-ears-bt/
BUILD_DIR[esp82] = cat-ears-web/
BUILD_DIR[x86test] = esp82/test

COMMAND[esp82] = arduino-cli --fqbn esp8266:esp8266:nodemcuv2
COMMAND[esp32] = arduino-cli --fqbn esp32:esp32:esp32
    

COMPILE[esp82] = compile
COMPILE[esp32] = compile

UPLOAD[esp82] = upload
UPLOAD[esp32] = uplaod

TTY = /dev/ttyUSB0
all : build install

build: $(foreach target,$(TARGET),build-$(target))

build-% :
	$(COMMAND[$*])  $(COMPILE[$*]) $(BUILD_DIR[$*])

$(TTY) :
	@echo "$(TTY) doesn't exist"
	@echo "please plug your esp"
	@false

install-% : | $(TTY)
	$(COMMAND[$*]) $(UPLOAD[$*]) -p $(TTY) $(BUILD_DIR[$*])
