#pragma once

extern void initHW(void);
extern String getMoveInput(void);
extern void clearDisplay(void);
extern void displayMove(String mv);
extern void displayBootWait(void);
extern void displayConnectWait(void);
extern void displayArray(void);
extern void readHall(byte read_hall_array[]);

extern void displayNewGame(void);
extern void displayWaitForGame(void);

extern String getFen(void);
extern bool areFensSame(const String& peripheralFen, const String& centralFen);
