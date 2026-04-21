/*
 *SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 *SPDX-License-Identifier: MIT
 */

#include "unit_asr.hpp"

#if defined(ARDUINO)
void ASRUnit::begin(HardwareSerial *serial, int baud, uint8_t RX, uint8_t TX)
{
    _serial = serial;
    _serial->begin(baud, SERIAL_8N1, RX, TX);
    commandNum = 0;
}
#else
void ASRUnit::begin(uart_port_t uart_num, int baud, int RX, int TX)
{
    _uart_num                 = uart_num;
    uart_config_t uart_config = {
        .baud_rate  = baud,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(_uart_num, &uart_config);
    uart_set_pin(_uart_num, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(_uart_num, 256, 0, 0, NULL, 0);
    _is_initialized = true;
    commandNum      = 0;
}
#endif

bool ASRUnit::update()
{
    size_t available_bytes = 0;

#if defined(ARDUINO)
    if (_serial == nullptr) {
        printf("Please call begin() first.\n");
        return 0;
    }
    available_bytes = _serial->available();
#else
    if (!_is_initialized) {
        printf("Please call begin() first.\n");
        return 0;
    }
    uart_get_buffered_data_len(_uart_num, &available_bytes);
#endif

    while (available_bytes >= 5) {
        uint8_t data[6];

#if defined(ARDUINO)
        _serial->readBytes(data, 5);
#else
        uart_read_bytes(_uart_num, data, 5, 20 / portTICK_PERIOD_MS);
#endif

        if (data[0] == 0xAA && data[1] == 0x55 && data[3] == 0x55 && data[4] == 0xAA) {
            commandNum = data[2];
            rawMessage = "";
            for (int i = 0; i < 5; i++) {
                char buffer[6];
                snprintf(buffer, sizeof(buffer), "0x%02X ", data[i]);
                rawMessage += buffer;
            }
#ifdef UNIT_ASR_DEBUG
            printf("Received data: %s\n", rawMessage.c_str());
#endif
            checkTickCallback();
            return 1;
        } else if (data[0] == 0xAA && data[1] == 0x55 && data[4] == 0x55) {
            size_t extra_byte_available = 0;
#if defined(ARDUINO)
            extra_byte_available = _serial->available();
            if (extra_byte_available >= 1) {
                _serial->readBytes(&data[5], 1);
#else
            uart_get_buffered_data_len(_uart_num, &extra_byte_available);
            if (extra_byte_available >= 1) {
                uart_read_bytes(_uart_num, &data[5], 1, 20 / portTICK_PERIOD_MS);
#endif
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
                    printf("Received data: %s\n", rawMessage.c_str());
#endif
                    checkTickCallback();
                    return 1;
                }
            }
        }
#if defined(ARDUINO)
        available_bytes = _serial->available();
#else
        uart_get_buffered_data_len(_uart_num, &available_bytes);
#endif
    }
    return 0;
}

void ASRUnit::sendComandNum(uint8_t commandNum)
{
    uint8_t message[] = {0xAA, 0x55, commandNum, 0x55, 0xAA};
#if defined(ARDUINO)
    _serial->write(message, sizeof(message));
#else
    uart_write_bytes(_uart_num, (const char *)message, sizeof(message));
#endif
}

std::string ASRUnit::getCurrentRawMessage()
{
    return rawMessage;
}

std::string ASRUnit::getCurrentCommandWord()
{
#ifdef UNIT_ASR_DEBUG
    printf("Command number: %X\n", commandNum);
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

bool ASRUnit::addCommandWord(uint8_t commandNum, const std::string &commandWord, CommandHandler handler)
{
    if (commandNum > 255 || commandNum < 0) return 0;
    commandList[commandNum] = {commandWord, handler};
    return 1;
}

bool ASRUnit::removeCommandWord(const std::string &commandWord)
{
    uint8_t commandNum = searchCommandNum(commandWord);
    if (commandNum == -1) return 0;
    commandList.erase(commandNum);
    return 1;
}

void ASRUnit::printCommandList()
{
    printf("--------------------------------------------------------\n");
    printf("| Command Num |      Command Word       |   Handler   |\n");
    printf("--------------------------------------------------------\n");
    for (const auto &entry : commandList) {
        uint8_t commandNum      = entry.first;
        std::string commandWord = entry.second.first;
        bool hasHandler         = (entry.second.second != nullptr);
        printf("|    0x%02X    | %-23s |     %s     |\n", commandNum, commandWord.c_str(), hasHandler ? "Yes" : "No");
    }
    printf("--------------------------------------------------------\n");
}

int8_t ASRUnit::searchCommandNum(const std::string &commandWord)
{
    for (const auto &cmd : commandList) {
        if (cmd.second.first == commandWord) {
            return cmd.first;
        }
    }
    return -1;
}

std::string ASRUnit::searchCommandWord(uint8_t commandNum)
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