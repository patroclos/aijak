#pragma once

#include "types.h"
#include "system.h"
#include "string.h"

#define VIDMEMADDR 0xB8000

extern int cursorX;
extern int cursorY;

extern const uint8 sw, sh, sd;

void clearLine(uint8 from, uint8 to);

void updateCursor();

void clearScreen();

void scrollUp(uint8 lineNumber);

void newLineCheck();

void printch(char c);

void print(string ch);
