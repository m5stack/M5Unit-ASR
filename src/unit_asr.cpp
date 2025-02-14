/*
 *SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 *SPDX-License-Identifier: MIT
 */

#include <Arduino.h>
#include <unit_asr.hpp>

void ASRUnit::begin(HardwareSerial *serial, int baud, uint8_t RX, uint8_t TX)
{
    _serial = serial;
    _serial->begin(baud, SERIAL_8N1, RX, TX);
    commandNum = 0;
}

bool ASRUnit::update()
{
    if (_serial == nullptr) {
        Serial.println("Please call begin() first.");
        return 0;
    }

    while (_serial->available() >= 2) {
        uint8_t header[2];
        _serial->readBytes(header, 2);

        if (header[0] != 0xAA || header[1] != 0x55) {
            continue;
        }

        unsigned long startTime = millis();
        while (_serial->available() < 3) {
            if (millis() - startTime > 100) {
                return 0;
            }
        }

        uint8_t message[3];
        _serial->readBytes(message, 3);

        rawMessage = "";
        char buffer[6];
        sprintf(buffer, "0x%02X 0x%02X ", header[0], header[1]);
        rawMessage += buffer;
        for (int i = 0; i < 3; i++) {
            sprintf(buffer, "0x%02X ", message[i]);
            rawMessage += buffer;
        }

#ifdef UNIT_ASR_DEBUG
        Serial.println("Received data: " + rawMessage);
#endif

        if (message[1] == 0x55 && message[2] == 0xAA) {
            commandNum = message[0];
            checkTickCallback();
            return 1;
        }
    }

    return 0;
}

void ASRUnit::sendComandNum(uint8_t commandNum)
{
    uint8_t message[] = {0xAA, 0x55, commandNum, 0x55, 0xAA};
    _serial->write(message, sizeof(message));
}

String ASRUnit::getCurrentRawMessage()
{
    return rawMessage;
}

String ASRUnit::getCurrentCommandWord()
{
#ifdef UNIT_ASR_DEBUG
    Serial.println("Command number: " + String(commandNum, HEX));
#endif
    return commandList.count(commandNum) ? commandList[commandNum].first : "Unknown command word";
}

uint8_t ASRUnit::getCurrentCommandNum()
{
    return commandNum;
}

bool ASRUnit::checkCurrentCommandHandler()
{
    return commandList.count(commandNum) && commandList[commandNum].second != nullptr;
}

bool ASRUnit::addCommandWord(uint8_t commandNum, const String &commandWord, CommandHandler handler)
{
    if (commandNum > 255 || commandNum < 0) return 0;
    commandList[commandNum] = {commandWord, handler};
    return 1;
}

bool ASRUnit::removeCommandWord(const String &commandWord)
{
    uint8_t commandNum = searchCommandNum(commandWord);
    if (commandNum == -1) return 0;
    commandList.erase(commandNum);
    return 1;
}

void ASRUnit::printCommandList()
{
    Serial.println("--------------------------------------------------------");
    Serial.println("| Command Num |      Command Word       |   Handler   |");
    Serial.println("--------------------------------------------------------");

    for (const auto &entry : commandList) {
        uint8_t commandNum = entry.first;
        String commandWord = entry.second.first;
        bool hasHandler    = (entry.second.second != nullptr);
        Serial.printf("|    0x%02X    | %-23s |     %s     |\n", commandNum, commandWord.c_str(),
                      hasHandler ? "Yes" : "No");
    }

    Serial.println("--------------------------------------------------------");
}

int8_t ASRUnit::searchCommandNum(const String &commandWord)
{
    for (const auto &cmd : commandList) {
        if (cmd.second.first == commandWord) {
            return cmd.first;
        }
    }
    return -1;
}

String ASRUnit::searchCommandWord(uint8_t commandNum)
{
    return commandList.count(commandNum) ? commandList[commandNum].first : "Unknown command word";
}

void ASRUnit::checkTickCallback()
{
    if (commandList.count(commandNum) && commandList[commandNum].second) {
        commandList[commandNum].second();
    }
}