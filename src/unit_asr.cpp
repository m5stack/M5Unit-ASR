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

    while (_serial->available() >= 5) {  // 至少需要5个字节
        uint8_t data[6];
        _serial->readBytes(data, 5);

        if (data[0] == 0xAA && data[1] == 0x55 && data[3] == 0x55 && data[4] == 0xAA) {
            commandNum = data[2];
            rawMessage = "";
            for (int i = 0; i < 5; i++) {
                char buffer[6];
                snprintf(buffer, sizeof(buffer), "0x%02X ", data[i]);
                rawMessage += buffer;
            }
#ifdef UNIT_ASR_DEBUG
            Serial.println("Received data: " + rawMessage);
#endif
            checkTickCallback();
            return 1;
        } else if (data[0] == 0xAA && data[1] == 0x55 && data[4] == 0x55) {
            if (_serial->available() >= 1) {
                _serial->readBytes(&data[5], 1);
                if (data[5] == 0xAA) {
                    commandNum = data[2];
                    msg        = data[3];
                    rawMessage = "";
                    for (int i = 0; i < 6; i++) {
                        char buffer[6];
                        snprintf(buffer, sizeof(buffer), "0x%02X ", data[i]);
                        rawMessage += buffer;
                    }
#ifdef UNIT_ASR_DEBUG
                    Serial.println("Received data: " + rawMessage);
#endif
                    checkTickCallback();
                    return 1;
                }
            }
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

uint8_t ASRUnit::getFirmwareVersion()
{
    if (msg == 0) sendComandNum(0x45);
    return msg;
}

void ASRUnit::checkTickCallback()
{
    if (commandList.count(commandNum) && commandList[commandNum].second) {
        commandList[commandNum].second();
    }
}