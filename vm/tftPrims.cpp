/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// tftPrims.cpp - Microblocks TFT screen primitives for the Citilab ED1 board
// Bernat Romagosa, November 2018

#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>

#include "mem.h"
#include "interp.h"

int useTFT = false;

#if defined(ARDUINO_CITILAB_ED1) || defined(ARDUINO_ARCH_ESP32)

	#define TFT_BLACK 0
	#define TFT_GREEN 0x7E0

	#if defined(ARDUINO_CITILAB_ED1)
		#include "Adafruit_GFX.h"
		#include "Adafruit_ST7735.h"

		#define TFT_CS	5
		#define TFT_DC	9
		#define TFT_RST	10
		Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

		void tftInit() {
			tft.initR(INITR_144GREENTAB);
			tft.setRotation(0);
			tftClear();
			useTFT = true;
		}
	#else
		#include "Adafruit_GFX.h"
		#include "Adafruit_ILI9341.h"

		#define TFT_CS	5
		#define TFT_DC	27
		Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

		void tftInit() {
			tft.begin();
			tft.setRotation(1);
			tftClear();
			// Turn on backlight on IoT-Bus
			pinMode(33, OUTPUT);
			digitalWrite(33, HIGH);
			useTFT = true;
		}
	#endif

void tftClear() {
	tft.fillScreen(TFT_BLACK);
}

OBJ primEnableDisplay(int argCount, OBJ *args) {
	if (trueObj == args[0]) {
		tftInit();
	} else {
		useTFT = false;
	}
}

int color24to16b(int color24b) {
	// converts 24-bit RGB888 format to 16-bit RGB565
	int r = (color24b >> 16) & 0xFF;
	int g = (color24b >> 8) & 0xFF;
	int b = color24b & 0xFF;
	return ((r << 8) & 0xF800) | ((g << 3) & 0x7E0) | ((b >> 3) & 0x1F);
}

OBJ primSetPixel(int argCount, OBJ *args) {
	int x = obj2int(args[0]);
	int y = obj2int(args[1]);
	// Re-encode color from 24 bits into 16 bits
	int color16b = color24to16b(obj2int(args[2]));
	tft.drawPixel(x, y, color16b);
}

OBJ primLine(int argCount, OBJ *args) {
	int x0 = obj2int(args[0]);
	int y0 = obj2int(args[1]);
	int x1 = obj2int(args[2]);
	int y1 = obj2int(args[3]);
	// Re-encode color from 24 bits into 16 bits
	int color16b = color24to16b(obj2int(args[4]));
	// optimize horizontal and vertical lines
	if (x0 == x1) {
		tft.drawFastVLine(x0, y0, abs(y1 - y0), color16b);
        } else if (y0 == y1) {
		tft.drawFastHLine(x0, y0, abs(x1 - x0), color16b);
        } else {
		tft.drawLine(x0, y0, x1, y1, color16b);
	}
}

OBJ primRect(int argCount, OBJ *args) {
	int x = obj2int(args[0]);
	int y = obj2int(args[1]);
	int width = obj2int(args[2]);
	int height = obj2int(args[3]);
	// Re-encode color from 24 bits into 16 bits
	int color16b = color24to16b(obj2int(args[4]));
	int fill = (trueObj == args[5]);
	if (fill) {
		tft.fillRect(x, y, width, height, color16b);
        } else {
		tft.drawRect(x, y, width, height, color16b);
	}
}

OBJ primCircle(int argCount, OBJ *args) {
	int x = obj2int(args[0]);
	int y = obj2int(args[1]);
	int radius = obj2int(args[2]);
	// Re-encode color from 24 bits into 16 bits
	int color16b = color24to16b(obj2int(args[3]));
	int fill = (trueObj == args[4]);
	if (fill) {
		tft.fillCircle(x, y, radius, color16b);
        } else {
		tft.drawCircle(x, y, radius, color16b);
	}
}

OBJ primText(int argCount, OBJ *args) {
	char *text = obj2str(args[0]);
	int x = obj2int(args[1]);
	int y = obj2int(args[2]);
	// Re-encode color from 24 bits into 16 bits
	int color16b = color24to16b(obj2int(args[3]));
	int scale = obj2int(args[4]);
	int wrap = (trueObj == args[5]);
	tft.setCursor(x, y);
	tft.setTextColor(color16b);
	tft.setTextSize(scale);
	tft.setTextWrap(wrap);
	tft.print(text);
}

void tftSetHugePixel(int x, int y, int state) {
	// simulate a 5x5 array of square pixels like the micro:bit LED array
	int minDimension, xInset = 0, yInset = 0;
	if (tft.width() > tft.height()) {
		minDimension = tft.height();
		xInset = (tft.width() - tft.height()) / 2;
	} else {
		minDimension = tft.width();
		yInset = (tft.height() - tft.width()) / 2;
	}
	int lineWidth = 3;
	int squareSize = (minDimension - (6 * lineWidth)) / 5;
	tft.fillRect(
		xInset + ((x - 1) * squareSize) + (x * lineWidth), // x
		yInset + ((y - 1) * squareSize) + (y * lineWidth), // y
		squareSize, squareSize,
		state ? TFT_GREEN : TFT_BLACK);
}

void tftSetHugePixelBits(int bits) {
	if (0 == bits) {
		tftClear();
	} else {
		for (int x = 1; x <= 5; x++) {
			for (int y = 1; y <= 5; y++) {
				tftSetHugePixel(x, y, bits & (1 << ((5 * (y - 1) + x) - 1)));
			}
		}
	}
}

#else // stubs

void tftInit() { }
void tftClear() { }
void tftSetHugePixel(int x, int y, int state) { }
void tftSetHugePixelBits(int bits) { }

OBJ primEnableDisplay(int argCount, OBJ *args) { return falseObj; }
OBJ primSetPixel(int argCount, OBJ *args) { return falseObj; }
OBJ primLine(int argCount, OBJ *args) { return falseObj; }
OBJ primRect(int argCount, OBJ *args) { return falseObj; }
OBJ primCircle(int argCount, OBJ *args) { return falseObj; }
OBJ primText(int argCount, OBJ *args) { return falseObj; }

#endif

// Primitives

static PrimEntry entries[] = {
	"enableDisplay", primEnableDisplay,
	"setPixel", primSetPixel,
	"line", primLine,
	"rect", primRect,
	"circle", primCircle,
	"text", primText,
};

void addTFTPrims() {
	addPrimitiveSet("tft", sizeof(entries) / sizeof(PrimEntry), entries);
}
