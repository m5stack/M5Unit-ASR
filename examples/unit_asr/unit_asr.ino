/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * @Hardwares: Basic/Fire/Gray(PortA) + Unit ASR
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5Unit-ASR:https://github.com/m5stack/M5Unit-ASR
 */

#include <M5Unified.h>
#include <unit_asr.hpp>

ASRUnit asr;

void setup()
{
    auto cfg            = M5.config();
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);
    int8_t port_a_pin1 = -1, port_a_pin2 = -1;
    port_a_pin1 = M5.getPin(m5::pin_name_t::port_a_pin1);
    port_a_pin2 = M5.getPin(m5::pin_name_t::port_a_pin2);
    Serial.printf("getPin: RX:%d TX:%d\n", port_a_pin1, port_a_pin2);
    asr.begin(&Serial1, 115200, port_a_pin1, port_a_pin2);
}

void myCommandHandler()
{
    Serial.println("hello command received!");
}

void loop()
{
    M5.update();
    if (asr.update()) {
        Serial.printf("Command word: %s\n", asr.getCurrentCommandWord());
        Serial.printf("Command number: 0x%X\n", asr.getCurrentCommandNum());
        Serial.print("Raw message:");
        Serial.println(asr.getCurrentRawMessage());
        Serial.printf("Have command handler: %s\n", asr.checkCurrentCommandHandler() ? "Yes" : "No");
    }
    if (M5.BtnA.wasPressed()) {
        asr.printCommandList();
        Serial.printf("Search command word: %s\n", asr.searchCommandWord(0x32));
        Serial.printf("Search command number: %d\n", asr.searchCommandNum("hello"));
    } else if (M5.BtnB.wasPressed()) {
        // asr.sendMessage(0xFE);
        Serial.printf(asr.addCommandWord(0x32, "hello", myCommandHandler) ? "Add Success\n" : "Add Fail\n");
    } else if (M5.BtnC.wasPressed()) {
        Serial.printf(asr.removeCommandWord("hello") ? "Remove Success\n" : "Remove Fail\n");
    }
}
