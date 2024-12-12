#include <Arduino.h>
#include <SPI.h>

class EpaperDisplay {
private:
    int cs_pin;
    int dc_pin;
    int rst_pin;
    int busy_pin;

    void sendCommand(uint8_t command) {
        digitalWrite(dc_pin, LOW);
        digitalWrite(cs_pin, LOW);
        SPI.transfer(command);
        digitalWrite(cs_pin, HIGH);
    }

    void sendData(uint8_t data) {
        digitalWrite(dc_pin, HIGH);
        digitalWrite(cs_pin, LOW);
        SPI.transfer(data);
        digitalWrite(cs_pin, HIGH);
    }

    void waitUntilIdle() {
        while (digitalRead(busy_pin) == LOW) {
            delay(100);
        }
    }

public:
    EpaperDisplay(int cs, int dc, int rst, int busy)
        : cs_pin(cs), dc_pin(dc), rst_pin(rst), busy_pin(busy) {}

    void begin() {
        pinMode(cs_pin, OUTPUT);
        pinMode(dc_pin, OUTPUT);
        pinMode(rst_pin, OUTPUT);
        pinMode(busy_pin, INPUT);

        SPI.begin();

        digitalWrite(rst_pin, LOW);
        delay(200);
        digitalWrite(rst_pin, HIGH);
        delay(200);
    }

    void init() {
        sendCommand(0x01); // POWER_SETTING
        sendData(0x03);
        sendData(0x00);
        sendData(0x2B);
        sendData(0x2B);

        sendCommand(0x06); // BOOSTER_SOFT_START
        sendData(0x17);
        sendData(0x17);
        sendData(0x17);

        sendCommand(0x04); // POWER_ON
        waitUntilIdle();

        sendCommand(0x00); // PANEL_SETTING
        sendData(0x3F);

        sendCommand(0x30); // PLL_CONTROL
        sendData(0x3C);
    }

    void displayText(const char *text) {
        sendCommand(0x24); // WRITE_RAM
        for (int i = 0; i < 128 * 296 / 8; i++) {
            sendData(0xFF); // White background
        }

        // Render the text (placeholder, depends on font rendering)
        // Add custom font rendering code here if needed.
        for (int i = 0; text[i] != '\0'; i++) {
            sendData(text[i]);
        }

        sendCommand(0x22); // DISPLAY_UPDATE_CONTROL_2
        sendData(0xC7);
        sendCommand(0x20); // MASTER_ACTIVATION
        waitUntilIdle();
    }
};

EpaperDisplay epd(10, 9, 8, 7); // Example pin configuration

void setup() {
    epd.begin();
    epd.init();
    epd.displayText("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

void loop() {
    // Nothing to do in loop
}
