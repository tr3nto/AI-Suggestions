#ifndef RF24_H
#define RF24_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

class RF24 {
private:
    void (*spi_write)(uint8_t);  // SPI write function pointer
    uint8_t (*spi_read)(void);   // SPI read function pointer
    volatile uint8_t &ce_pin;    // CE pin reference
    volatile uint8_t &csn_pin;   // CSN pin reference

    void csn_low() { csn_pin = 0; __delay_us(10); }
    void csn_high() { csn_pin = 1; __delay_us(10); }
    void ce_low() { ce_pin = 0; __delay_us(10); }
    void ce_high() { ce_pin = 1; __delay_us(10); }

    uint8_t write_register(uint8_t reg, uint8_t value);
    uint8_t read_register(uint8_t reg);

public:
    RF24(void (*spi_write_func)(uint8_t), uint8_t (*spi_read_func)(void), volatile uint8_t &ce, volatile uint8_t &csn);

    void begin();
    void setChannel(uint8_t channel);
    void setPALevel(uint8_t level);
    void openWritingPipe(const uint8_t *address);
    void openReadingPipe(uint8_t pipe, const uint8_t *address);
    void startListening();
    void stopListening();
    bool write(const void *data, uint8_t len);
    bool available();
    void read(void *data, uint8_t len);
};

#endif

#include "RF24.h"

RF24::RF24(void (*spi_write_func)(uint8_t), uint8_t (*spi_read_func)(void), volatile uint8_t &ce, volatile uint8_t &csn)
    : spi_write(spi_write_func), spi_read(spi_read_func), ce_pin(ce), csn_pin(csn) {}

void RF24::begin() {
    ce_low();
    csn_high();
    write_register(0x00, 0x0F); // Set CONFIG register (default value)
    write_register(0x01, 0x3F); // Enable Auto Acknowledgment on all pipes
    write_register(0x02, 0x03); // Enable RX on pipe 0 and pipe 1
    write_register(0x03, 0x03); // Address width (5 bytes)
    write_register(0x04, 0x04); // Auto retransmit delay and count
    write_register(0x05, 0x02); // RF channel (default to 2)
    write_register(0x06, 0x07); // RF setup (default value)
    write_register(0x07, 0x70); // Clear STATUS register flags
}

void RF24::setChannel(uint8_t channel) {
    write_register(0x05, channel);
}

void RF24::setPALevel(uint8_t level) {
    uint8_t rfSetup = read_register(0x06);
    rfSetup &= 0xF8; // Clear current level bits
    rfSetup |= (level & 0x07);
    write_register(0x06, rfSetup);
}

void RF24::openWritingPipe(const uint8_t *address) {
    csn_low();
    spi_write(0x20 | 0x10); // Write to TX_ADDR register
    for (uint8_t i = 0; i < 5; i++) {
        spi_write(address[i]);
    }
    csn_high();
}

void RF24::openReadingPipe(uint8_t pipe, const uint8_t *address) {
    if (pipe > 5) return; // Valid pipes are 0-5
    uint8_t reg = 0x0A + pipe;
    csn_low();
    spi_write(0x20 | reg); // Write to RX_ADDR_Px register
    for (uint8_t i = 0; i < 5; i++) {
        spi_write(address[i]);
    }
    csn_high();
}

void RF24::startListening() {
    write_register(0x00, read_register(0x00) | 0x01); // Set PRIM_RX bit
    ce_high();
}

void RF24::stopListening() {
    ce_low();
    write_register(0x00, read_register(0x00) & ~0x01); // Clear PRIM_RX bit
}

bool RF24::write(const void *data, uint8_t len) {
    csn_low();
    spi_write(0xA0); // Write payload command
    for (uint8_t i = 0; i < len; i++) {
        spi_write(((uint8_t *)data)[i]);
    }
    csn_high();

    ce_high();
    __delay_us(10);
    ce_low();

    uint8_t status;
    for (uint8_t i = 0; i < 10; i++) {
        status = read_register(0x07);
        if (status & 0x20) { // TX_DS (data sent) flag
            write_register(0x07, 0x20); // Clear TX_DS flag
            return true;
        }
        if (status & 0x10) { // MAX_RT (max retries reached) flag
            write_register(0x07, 0x10); // Clear MAX_RT flag
            return false;
        }
    }
    return false;
}

bool RF24::available() {
    return (read_register(0x07) & 0x40); // RX_DR (data ready) flag
}

void RF24::read(void *data, uint8_t len) {
    csn_low();
    spi_write(0x61); // Read payload command
    for (uint8_t i = 0; i < len; i++) {
        ((uint8_t *)data)[i] = spi_read();
    }
    csn_high();
    write_register(0x07, 0x40); // Clear RX_DR flag
}

uint8_t RF24::write_register(uint8_t reg, uint8_t value) {
    csn_low();
    spi_write(0x20 | reg);
    uint8_t status = spi_write(value);
    csn_high();
    return status;
}

uint8_t RF24::read_register(uint8_t reg) {
    csn_low();
    spi_write(reg);
    uint8_t value = spi_read();
    csn_high();
    return value;
}
