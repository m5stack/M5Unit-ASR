/*
 *SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 *SPDX-License-Identifier: MIT
 */

#ifndef ASRUNIT_H
#define ASRUNIT_H

#include <Arduino.h>
#include <map>
#include <functional>
#include "driver/uart.h"

#define UNIT_ASR_BAUD 115200

// #define UNIT_ASR_DEBUG

class ASRUnit {
public:
    typedef std::function<void()> CommandHandler;

    /**
     * @brief Initializes the ASR unit with serial communication parameters.
     *
     * This function sets up the serial communication for the ASR unit by configuring
     * the specified hardware serial port with the given baud rate and pin assignments.
     *
     * @param serial Pointer to a HardwareSerial object, defaults to Serial1
     * @param baud The baud rate for serial communication, defaults to 115200
     * @param RX The GPIO pin number for receiving data, defaults to 16
     * @param TX The GPIO pin number for transmitting data, defaults to 17
     */
    void begin(HardwareSerial *serial = &Serial1, int baud = UNIT_ASR_BAUD, uint8_t RX = 16, uint8_t TX = 17);

    /**
     * @brief Sends a command number to the ASR unit.
     *
     * Transmits the specified command number to the ASR unit through the configured
     * serial connection.
     *
     * @param commandNum The command number to send to the ASR unit
     */
    void sendComandNum(uint8_t commandNum);

    /**
     * @brief Retrieves the current raw message from the ASR unit.
     *
     * @return The raw message string received from the ASR unit
     */
    String getCurrentRawMessage();

    /**
     * @brief Gets the currently recognized command word.
     *
     * @return The string representation of the current command word
     */
    String getCurrentCommandWord();

    /**
     * @brief Gets the current command number.
     *
     * @return The numeric identifier of the current command
     */
    uint8_t getCurrentCommandNum();

    /**
     * @brief Checks and executes the handler for the current command.
     *
     * Attempts to execute the handler function associated with the current command
     * if one exists.
     *
     * @return true if a handler exists and was executed successfully, false otherwise
     */
    bool checkCurrentCommandHandler();

    /**
     * @brief Adds a new command word and its associated handler to the command list.
     *
     * This function registers a new voice command with its corresponding command number
     * and optional handler function.
     *
     * @param commandNum The numeric identifier for the command
     * @param commandWord The voice command word or phrase to recognize
     * @param handler Optional callback function to execute when the command is recognized
     * @return true if the command was successfully added, false otherwise
     */
    bool addCommandWord(uint8_t commandNum, const String &commandWord, CommandHandler handler = nullptr);

    /**
     * @brief Removes a command word from the command list.
     *
     * @param commandWord The command word to remove from the recognition list
     * @return true if the command was successfully removed, false otherwise
     */
    bool removeCommandWord(const String &commandWord);

    /**
     * @brief Searches for the command number associated with a command word.
     *
     * @param commandWord The command word to search for
     * @return The associated command number if found, -1 if not found
     */
    int8_t searchCommandNum(const String &commandWord);

    /**
     * @brief Searches for the command word associated with a command number.
     *
     * @param commandNum The command number to search for
     * @return The associated command word if found, empty string if not found
     */
    String searchCommandWord(uint8_t commandNum);

    /**
     * @brief Processes any pending tick callbacks.
     *
     * Checks and executes any scheduled tick-based operations for the ASR unit.
     */
    void checkTickCallback();

    /**
     * @brief Updates the ASR unit's state and processes new commands.
     *
     * This function should be called regularly in the main loop to handle
     * new voice commands and update the unit's state.
     *
     * @return true if a new command was processed, false otherwise
     */
    bool update();

    /**
     * @brief Prints the complete list of registered command words.
     *
     * Outputs all registered command words and their associated command numbers
     * to the serial monitor for debugging purposes.
     */
    void printCommandList();

private:
    HardwareSerial *_serial;
    String rawMessage;
    uint8_t commandNum;
    std::map<uint8_t, std::pair<String, CommandHandler>> commandList = {
        {0x00, {"Unknown command word", nullptr}},
        {0x01, {"up", nullptr}},
        {0x02, {"down", nullptr}},
        {0x03, {"left", nullptr}},
        {0x04, {"turn left", nullptr}},
        {0x05, {"right", nullptr}},
        {0x06, {"turn right", nullptr}},
        {0x07, {"forward", nullptr}},
        {0x08, {"front", nullptr}},
        {0x09, {"backward", nullptr}},
        {0x0A, {"back", nullptr}},
        {0x10, {"open", nullptr}},
        {0x11, {"close", nullptr}},
        {0x12, {"start", nullptr}},
        {0x13, {"stop", nullptr}},
        {0x14, {"turn on", nullptr}},
        {0x15, {"turn off", nullptr}},
        {0x16, {"play", nullptr}},
        {0x17, {"pause", nullptr}},
        {0x18, {"turn on the lights", nullptr}},
        {0x19, {"turn off the lights", nullptr}},
        {0x1A, {"previous", nullptr}},
        {0x1B, {"next", nullptr}},
        {0x20, {"zero", nullptr}},
        {0x21, {"one", nullptr}},
        {0x22, {"two", nullptr}},
        {0x23, {"three", nullptr}},
        {0x24, {"four", nullptr}},
        {0x25, {"five", nullptr}},
        {0x26, {"six", nullptr}},
        {0x27, {"seven", nullptr}},
        {0x28, {"eight", nullptr}},
        {0x29, {"nine", nullptr}},
        {0x30, {"ok", nullptr}},
        {0x31, {"hi, A S R", nullptr}},
        {0x32, {"hello", nullptr}},
        {0x40, {"increase volume", nullptr}},
        {0x41, {"decrease volume", nullptr}},
        {0x42, {"maximum volume", nullptr}},
        {0x43, {"medium volume", nullptr}},
        {0x44, {"minimum volume", nullptr}},
        {0x45, {"check firmware version", nullptr}},
        {0xFE, {"Announce", nullptr}},
        {0xFF, {"Hi,M Five", nullptr}},
    };
};

#endif  // ASRUNIT_H