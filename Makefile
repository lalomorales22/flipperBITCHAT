# Makefile for BitChat Flipper Zero App
# Uses ufbt for building

.PHONY: all build clean flash launch

all: build

build:
	ufbt

clean:
	ufbt clean

flash: build
	ufbt flash

launch: flash
	ufbt launch

debug: build
	ufbt launch_app APPSRC=bitchat
