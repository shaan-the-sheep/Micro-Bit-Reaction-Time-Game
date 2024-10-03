/**
 * @file main.cpp
 * @author Shaan Jassal
 * @brief Reaction Time game that uses: OLED display, built-in display, button A, 2 LEDs
 */

#include "MicroBit.h"
#include "SSD1306.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define AVG_RTIME 250
#define NUM_ROUNDS 4

MicroBit uBit;
SSD1306 *oled = new SSD1306(OLED_WIDTH, OLED_HEIGHT, 0x78);

const char * const happy_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

const char * const sad_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    000,000,000,000,000\n\
    000,255,255,255,000\n\
    255,000,000,000,255\n";

/**
 * @brief Draws a pixel onto the OLED display
 * Creates a buffer to represent the display
 * Sets first byte in the buffer for communication protocol
 * Sets the pixel in the buffer
 * Sends the image buffer to the display
 * @param x x-coordinate
 * @param y y-coordinate
 */
void drawPixel(int16_t x, int16_t y) {
    ManagedBuffer buf(((OLED_WIDTH * OLED_HEIGHT) / 8) + 1);
    buf[0] = 0x40;  

    buf[x + (y >> 3) * OLED_WIDTH + 1] |= (1 << (x & 7));

    oled->sendData(buf.getBytes(), buf.length());
}

/**
 * @brief Displays a HEX image onto the OLED display
 * Creates a buffer to represent the display
 * Sets first byte in the buffer for communication protocol
 *
 * Sends the image buffer to the display
 * @param hexValue the image to display
 */
void displayHexImage(uint8_t hexValue) {

    ManagedBuffer buf(((OLED_WIDTH * OLED_HEIGHT) / 8) + 1);
    buf[0] = 0x40;

    for (int i = 1; i < buf.length(); i++)
        buf[i] = hexValue;

    oled->sendData(buf.getBytes(), buf.length());
}

/**
 * @brief displays the countdown for the start of the game
 */
void showCountdown() {
    uBit.display.scroll("3 2 1");
    uBit.display.print("GO!");
}

/**
 * @brief Gives random delay up to 5 seconds
 * @return int 
 */
int randDelay() {
    int delay =  (rand() % 5 + 1) * 1000;
    uBit.sleep(delay);
}

/**
 * @brief Returns random num 1 - 50
 * @return int random coordinate
 */
int getRandCoordinate() {
    return rand() % 50 + 1;
}

/**
 * @brief Loops until button A is pressed
 * 
 */
void waitButtonPress() {
    while (!uBit.buttonA.isPressed()) {
    }
}

/**
 * @brief Clears P0 (red LED) and P2(green)
 */
void clearLEDs() {
    uBit.io.P0.setDigitalValue(0);
    uBit.io.P2.setDigitalValue(0);
}

/**
 * @brief Displays empty buffer to OLED screen
 */
void clearScreen() {
    displayHexImage(0x0);
}

/**
 * @brief Displays the image to react to
 * Creates a random delay 1 - 5s
 * If in 1st half of game, turns all pixels on OLED on (easy level)
 * Else, sets a random pixel (hard level)
 * @param round 
 */
void displayRandomPattern(int round) {
    randDelay();
    uBit.sleep(randDelay);

    if (round <= NUM_ROUNDS/2) {
        displayHexImage(0xFF);
    } else {
        int randX = getRandCoordinate();
        int randY = getRandCoordinate();
        drawPixel(randX, randY);
    }
}

/**
 * @brief Logic after button press
 * Calculates reaction time
 * If faster than avg, sets green led
 * If slower, sets red light
 * Displays reaction time
 * Clears both LEDs
 * @param startTime 
 * @param totalReactionTime 
 */
void handleButtonPress(int startTime, int &totalReactionTime) {
    int reactionTime = system_timer_current_time() - startTime;

    if (reactionTime <= AVG_RTIME) {
        uBit.io.P2.setDigitalValue(1); // LED on P2
    } else {
        uBit.io.P0.setDigitalValue(1); // LED on P0
    }

    uBit.display.scroll(reactionTime);
    totalReactionTime += reactionTime;
    clearLEDs();
}

/**
 * @brief Logic for a round
 * Displays image 
 * Stores start time
 * Waits for button press
 * Calculates and displays reaction time
 * Clears screen
 * @param totalReactionTime 
 * @param round 
 */
void performReactionRound(int &totalReactionTime, int round) {
    displayRandomPattern(round);

    int startTime = system_timer_current_time();

    waitButtonPress();
    handleButtonPress(startTime, totalReactionTime);
    clearScreen();
}

/**
 * @brief Mean reaction time logic
 * Calculates mean reaction time
 * Displays mean time
 * If faster than avg, displays smiley face
 * If slower, displays sad face
 * @param totalReactionTime 
 */
void displayMean(int totalReactionTime) {
    int meanReactionTime = totalReactionTime / NUM_ROUNDS;

    uBit.display.scroll("T");
    uBit.display.scroll(meanReactionTime);

    MicroBitImage facialExpression = (meanReactionTime <= AVG_RTIME) ? MicroBitImage(happy_emoji) : MicroBitImage(sad_emoji);
    uBit.display.print(facialExpression);
}

/**
 * @brief Game logic
 * Starts game
 * Performs each round
 * Displays mean time
 */
void runGame() {
    int totalReactionTime = 0;
    showCountdown();

    for (int round = 1; round <= NUM_ROUNDS; ++round) {
        performReactionRound(totalReactionTime, round);
    }
    displayMean(totalReactionTime);
}

int main() {
    uBit.init();
    uBit.io.P19.setPull(PullMode::Up);
    uBit.io.P20.setPull(PullMode::Up);
    runGame();
}