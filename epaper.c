#include <Arduino.h>
#include <SPI.h>

// Basic 5x7 ASCII font table
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    // Add more characters as needed...
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    // Add remaining characters...
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

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

        // Display text using font table
        int x = 0, y = 0;
        for (int i = 0; text[i] != '\0'; i++) {
            if (text[i] >= ' ' && text[i] <= 'Z') {
                uint8_t *charBitmap = (uint8_t *)font5x7[text[i] - ' '];
                for (int col = 0; col < 5; col++) {
                    sendData(charBitmap[col]);
                }
                sendData(0x00); // Space between characters
            }
            x += 6; // Move to the next character position
            if (x > 128) { // Wrap to next line if needed
                x = 0;
                y += 8;
            }
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
