/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2023 John Maloney, Bernat Romagosa, and Jens Mönig

// hidPrims.cpp - Keyboard and Mouse emulation
// Bernat Romagosa, February 2023

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "interp.h"

#if defined(ADAFRUIT_METRO_M0_EXPRESS) || defined(ADAFRUIT_TRINKET_M0) || \
	(defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))

#include "Keyboard.h"
#include "Mouse.h"

char mouseInitialized = 0;
char keyboardInitialized = 0;

void initMouse () {
	if (!mouseInitialized) {
		Mouse.begin();
		mouseInitialized = 1;
	}
}

void initKeyboard () {
	if (!keyboardInitialized) {
		Keyboard.begin();
		keyboardInitialized = 1;
	}
}

OBJ primMouseMove(int argCount, OBJ *args) {
	initMouse();
	int deltaX = obj2int(args[0]);
	int deltaY = obj2int(args[1]);
	Mouse.move(deltaX, deltaY);
	return falseObj;
}

OBJ primMousePress(int argCount, OBJ *args) {
	initMouse();
	int button = obj2int(args[0]);
	Mouse.press(button);
	return falseObj;
}

OBJ primMouseRelease(int argCount, OBJ *args) {
	initMouse();
	for (int i = 1; i < 5; i++) { Mouse.release(i); }
	return falseObj;
}

OBJ primMouseScroll(int argCount, OBJ *args) {
	initMouse();
	int delta = obj2int(args[0]);
	Mouse.move(0, 0, delta);
	return falseObj;
}

OBJ primPressKey(int argCount, OBJ *args) {
	initKeyboard();
	OBJ key = args[0];
	int modifier = obj2int(args[1]);
	switch (modifier) {
		case 1: // shift
			Keyboard.press(KEY_LEFT_SHIFT);
			break;
		case 2: // control
			Keyboard.press(KEY_LEFT_CTRL);
			break;
		case 3: // alt (option on Mac)
			Keyboard.press(KEY_LEFT_ALT);
			break;
		case 4: // meta (command on Mac)
			Keyboard.press(KEY_LEFT_GUI);
			break;
		case 5: // AltGr (option on Mac)
			Keyboard.press(KEY_RIGHT_ALT);
			break;
	}
	// accept both characters and ASCII values
	if (IS_TYPE(key, StringType)) {
		Keyboard.write(obj2str(key)[0]);
	} else if (isInt(key)) {
		Keyboard.write(obj2int(key));
	}
	if (modifier) {
		Keyboard.releaseAll();
	}
	return falseObj;
}
#else

// stubs
OBJ primMouseMove(int argCount, OBJ *args) { return falseObj; }
OBJ primMousePress(int argCount, OBJ *args) { return falseObj; }
OBJ primMouseRelease(int argCount, OBJ *args) { return falseObj; }
OBJ primMouseScroll(int argCount, OBJ *args) { return falseObj; }
OBJ primPressKey(int argCount, OBJ *args) { return falseObj; }

#endif

// Primitives

static PrimEntry entries[] = {
	{"mouseMove", primMouseMove},
	{"mousePress", primMousePress},
	{"mouseRelease", primMouseRelease},
	{"mouseScroll", primMouseScroll},
	{"pressKey", primPressKey},
};

void addHIDPrims() {
	addPrimitiveSet("hid", sizeof(entries) / sizeof(PrimEntry), entries);
}