SHELL := /bin/bash
SDK_PATH := sdk
SDK_URL := https://s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.3-linux64.tar.bz2
VENV_PATH := $(SDK_PATH)/.env

export PATH := $(SDK_PATH)/bin:$(PATH)

.DEFAULT_GOAL := install
.PHONY: build clean screenshots

build: sdk
	pebble build

clean:
	pebble clean

install: build
	pebble install --phone $(PEBBLE_PHONE)

install-aplite: build
	pebble install --emulator aplite

install-basalt: build
	pebble install --emulator basalt

install-chalk: build
	pebble install --emulator chalk

logs:
	pebble logs

screenshots:
	./screenshots aplite
	./screenshots basalt

sdk:
	sudo apt-get -qqy update
	sudo apt-get -qqy install python-pip python2.7-dev \
		libsdl1.2debian libfdt1 libpixman-1-0
	sudo pip install -q virtualenv
	mkdir -p "$(SDK_PATH)"
	wget -qO - $(SDK_URL) | tar -xj --overwrite --strip-components=1 -C "$(SDK_PATH)"
	virtualenv -q "$(VENV_PATH)"
	source "$(VENV_PATH)/bin/activate" \
		&& pip install -qr "$(SDK_PATH)/requirements.txt"
