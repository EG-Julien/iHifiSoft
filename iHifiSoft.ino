/** IDE ONLY **/

#include <Arduino.h>/*
#include <CmdParser/src/CmdParser.hpp>
#include <CmdParser/src/CmdBuffer.hpp>
#include "CmdParser/src/CmdParser.hpp"*/

/** END IDE ONLY **/

#include <CmdParser.hpp>
#include <CmdBuffer.hpp>

CmdBuffer<128> cmdBuffer;
CmdParser cmdParser;

#define SOUND_SELECTION_A 6
#define SOUND_SELECTION_B 7
#define POWER_RELAY 8
#define POWER_BT 3
#define POWER_UP_INT_A 2
#define POWER_UP_INT_B 5
#define TEMP_1 A5
#define TEMP_2 A4

#define ERROR_SERIAL_PARSER -1

String IP = "0001";

int x, y, k, autoPowerOn = 1, previousState = 0;

void checkCommand(void);

void reportERROR(int err);

void bluetoothState(String state);
void audioState(String state);
void powerState(String state);
void powerUpAudio(void);
void powerDownAudio(void);


void setup() {
    Serial.begin(115200);

    pinMode(SOUND_SELECTION_A, OUTPUT);
    pinMode(SOUND_SELECTION_B, OUTPUT);
    pinMode(POWER_RELAY, OUTPUT);
    pinMode(POWER_BT, OUTPUT);
    pinMode(POWER_UP_INT_A, INPUT);
    pinMode(POWER_UP_INT_B, INPUT);

    digitalWrite(SOUND_SELECTION_A, LOW);
    digitalWrite(SOUND_SELECTION_B, LOW);
    digitalWrite(POWER_RELAY, LOW);
    digitalWrite(POWER_BT, LOW);

    cmdParser.setOptKeyValue(true);
}

void loop() {
    checkCommand();
    x = 0; y = 0;

    if (autoPowerOn == 1) {

        for (k = 0; k < 30; k++) {
            x += digitalRead(POWER_UP_INT_A);
            y += digitalRead(POWER_UP_INT_B);

            delay(5);
        }

        Serial.print(x);
        Serial.print("   ");
        Serial.println(y);

        if (x >= 5 || y >= 5) {
            previousState = 0;
            powerDownAudio();
            if (x >= 5 && y >= 5) {
                digitalWrite(SOUND_SELECTION_A, HIGH);
                digitalWrite(SOUND_SELECTION_B, LOW);
            } else {
                if (x >= 5) {
                    digitalWrite(SOUND_SELECTION_A, HIGH);
                    digitalWrite(SOUND_SELECTION_B, LOW);
                }
                if (y >= 5) {
                    digitalWrite(SOUND_SELECTION_A, LOW);
                    digitalWrite(SOUND_SELECTION_B, LOW);
                }
            }
            powerUpAudio();
        } else {
            previousState += 1;
            if (previousState > 200) {
                powerDownAudio();
                previousState = 0;
            }
        }
    }
}

void powerUpAudio(void) {
    digitalWrite(POWER_RELAY, HIGH);
}

void powerDownAudio(void) {
    digitalWrite(POWER_RELAY, LOW);
}

void checkCommand(void) {
    int is_received = cmdBuffer.readFromSerial(&Serial, 1000);

    if (is_received) {
        if (cmdParser.parseCmd(cmdBuffer.getStringFromBuffer()) != CMDPARSER_ERROR) {
            String current_cmd = cmdParser.getCommand();
            if (cmdParser.equalCommand_P(PSTR("SET"))) {
                String ip = cmdParser.getValueFromKey_P(PSTR("IP"));
                if (ip == IP) {
                    bluetoothState(cmdParser.getValueFromKey_P(PSTR("BLUETOOTH")));
                    audioState(cmdParser.getValueFromKey_P(PSTR("AUDIO_OUTPUT")));
                    powerState(cmdParser.getValueFromKey_P(PSTR("POWER")));
                }
            }
            /*
             * Command sample = SET IP="0001" BLUETOOTH="off" AUDIO_OUTPUT="ext" POWER="off"
             *                  GET IP="0001" TO="0000"
             * */
            if (cmdParser.equalCommand_P(PSTR("GET"))) {
                String ip = cmdParser.getValueFromKey_P(PSTR("IP"));
                if (ip == IP) {
                    int bt_state = digitalRead(POWER_BT);
                    int ssa      = digitalRead(SOUND_SELECTION_A);
                    int ssb      = digitalRead(SOUND_SELECTION_B);
                    int power    = digitalRead(POWER_RELAY);
                    int temp_a   = analogRead(TEMP_1);
                    int temp_b   = analogRead(TEMP_2);

                    String __to = cmdParser.getValueFromKey_P(PSTR("TO"));
                    String reply = "MASTER IP=\"0000\" FROM=\"" + IP + "\" BLUETOOTH=\"" + bt_state + "\" SOUND_SELECTION_A=\"" + ssa + "\" SOUND_SELECTION_B=\"" + ssb + "\" POWER=\"" + power + "\" TEMP_1=\"" + temp_a + "\" TEMP_2=\"" + temp_b + "\"";
                    Serial.println(reply);
                }
            }

            if (cmdParser.equalCommand_P(PSTR("DEBUG"))) {
                String ip = cmdParser.getValueFromKey_P(PSTR("IP"));
                if (ip == IP) {
                    int bt_state = digitalRead(POWER_BT);
                    int ssa      = digitalRead(SOUND_SELECTION_A);
                    int ssb      = digitalRead(SOUND_SELECTION_B);
                    int power    = digitalRead(POWER_RELAY);
                    int temp_a   = analogRead(TEMP_1);
                    int temp_b   = analogRead(TEMP_2);

                    String __to = cmdParser.getValueFromKey_P(PSTR("TO"));
                    String reply = "MASTER IP=\"0000\" FROM=\"" + IP + "\" BLUETOOTH=\"" + bt_state + "\" SOUND_SELECTION_A=\"" + ssa + "\" SOUND_SELECTION_B=\"" + ssb + "\" POWER=\"" + power + "\" TEMP_1=\"" + temp_a + "\" TEMP_2=\"" + temp_b + "\"";
                    Serial.println(reply);
                }
            }
        } else {
            reportERROR(ERROR_SERIAL_PARSER);
        }
    } else {
        //reportERROR(ERROR_SERIAL_TIMEOUT);
    }
}

void reportERROR(int err) {
    Serial.print("MASTER IP=\"0000\" FROM=\"");
    Serial.print(IP);
    Serial.print("\" ERROR=\"");
    Serial.print(err);
    Serial.println("\"");
}

void bluetoothState(String state) {
    if (state == "on") {
        digitalWrite(POWER_BT, HIGH);
    } else {
        digitalWrite(POWER_BT, LOW);
    }
}

void audioState(String state) {
    digitalWrite(POWER_RELAY, LOW);
    delay(500);
    if (state == "int") {
        digitalWrite(SOUND_SELECTION_A, HIGH);
        digitalWrite(SOUND_SELECTION_B, LOW);
    } else if (state == "ext") {
        digitalWrite(SOUND_SELECTION_A, LOW);
        digitalWrite(SOUND_SELECTION_B, LOW);
    } else if (state == "bluetooth") {
        digitalWrite(SOUND_SELECTION_A, LOW);
        digitalWrite(SOUND_SELECTION_B, HIGH);
    }
    delay(500);
    digitalWrite(POWER_RELAY, HIGH);

}

void powerState(String state) {
    if (state == "on") {
        digitalWrite(POWER_RELAY, HIGH);
        autoPowerOn = 0;
    } else if (state == "auto") {
        autoPowerOn = 1;
    } else {
        digitalWrite(POWER_RELAY, LOW);
        autoPowerOn = 0;
    }
}