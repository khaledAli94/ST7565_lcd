#include <SPI.h>
#include "command.h"

// Pin definitions - ST7565 SPI interface
#define SCLK_PIN 13  // Clock
#define SID_PIN 11   // Data

#define CS_PIN 10    // Chip Select
#define CMD_PIN 9     // Command/Data
#define RST_PIN 8    // Reset

// Display dimensions
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_PAGES 8


unsigned char displayBuffer[LCD_WIDTH * LCD_PAGES];  // Full buffer: 128 × 8 = 1024 bytes
char index = 0;

void initDisplaySPI() {
  // Hardware reset
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialization sequence
  sendCommandSPI(0xE2);     // Reset
  delay(10);
  
  sendCommandSPI(0xA2);     // Bias 1/9
  sendCommandSPI(SEG_DIR_REVERSE);     // Segment rev
  sendCommandSPI(COM_SCAN_REVERSE);     // COM norm
  
  sendCommandSPI(0x2F);     // Power all on
  sendCommandSPI(0x26);     // Regulator mid
  
  sendCommandSPI(0x81);     // Enter contrast mode
  sendCommandSPI(0x1F);     // Contrast value
  
  sendCommandSPI(0xA6);     // Normal display
  sendCommandSPI(0x40);     // Start line 0
  sendCommandSPI(0xAF);     // Display ON
}

void sendCommandSPI(enum cmd_t cmd) {
  digitalWrite(CMD_PIN, LOW); //activate CMD
  digitalWrite(CS_PIN, LOW); //activate CS
  SPI.transfer(cmd);              // Hardware SPI does the bit shifting!
  digitalWrite(CS_PIN, HIGH); 
}

void sendDataSPI(uint8_t data) {
  digitalWrite(CMD_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(data);
  digitalWrite(CS_PIN, HIGH);
}

// Clear display using hardware SPI
void clearDisplay() {
  for (uint8_t page = 0; page < LCD_PAGES; page++) {
    sendCommandSPI(0xB0 | page);
    sendCommandSPI(0x10);
    sendCommandSPI(0x00);
    
    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      sendDataSPI(0x00);
      displayBuffer[page * LCD_WIDTH + col] = 0x00;
    }
  }
}

// Update entire display from buffer
void updateDisplay() {
  for (uint8_t page = 0; page < LCD_PAGES; page++) {
    sendCommandSPI(0xB0 | page);
    sendCommandSPI(0x10);
    sendCommandSPI(0x00);
    
    digitalWrite(CMD_PIN, HIGH);
    digitalWrite(CS_PIN, LOW);
    
    // Fast block transfer for entire page
    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      SPI.transfer(displayBuffer[page * LCD_WIDTH + col]);
    }
    
    digitalWrite(CS_PIN, HIGH);
  }
}


void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(CS_PIN, OUTPUT);
  pinMode(CMD_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialize SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  
  // Initialize display
  initDisplaySPI();
  
  // Test
  clearDisplay();
  
  Serial.println("ST7565 with Hardware SPI Ready");
}

uint16_t setPixel(unsigned char *fb, uint8_t x, uint8_t y) {
  uint8_t page = y >> 3;
  uint8_t bit  = y & 0x07;
  uint16_t offset = page * LCD_WIDTH + x;

  fb[offset] |= (1 << bit);
  // Update display (efficient: update only changed byte)
  sendCommandSPI(0xB0 | page);
  sendCommandSPI(0x10 | (x >> 4));
  sendCommandSPI(0x00 | (x & 0x0F));
  sendDataSPI(displayBuffer[index]);
  return offset;
}

uint16_t clearPixel(unsigned char *fb, uint8_t x, uint8_t y) {
  uint8_t page = y >> 3;
  uint8_t bit  = y & 0x07;
  uint16_t offset = page * LCD_WIDTH + x;

  fb[offset] &= ~(1 << bit);
  // Update display (efficient: update only changed byte)
  sendCommandSPI(0xB0 | page);
  sendCommandSPI(0x10 | (x >> 4));
  sendCommandSPI(0x00 | (x & 0x0F));
  sendDataSPI(displayBuffer[index]);
  return offset;
}


static const uint8_t char8x8_A[] = { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00};   // U+0041 (A)
static const uint8_t char8x8_B[] = { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00};   // U+0042 (B)

static const uint8_t char8x8_a[] = { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00};  // U+0061 (a)
static const uint8_t char8x8_b[] = { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00};   // U+0062 (b)
// static const uint8_t char8x8_A[] =
// {
//     0x00, 0x78, 0x14, 0x12, 0x12, 0x14, 0x78, 0x00
// };

void drawGlyph8x8(uint8_t x, uint8_t page, const uint8_t *glyph) {
    uint16_t offset = page * LCD_WIDTH + x;

    for (uint8_t i = 0; i < 8; i++) {
        displayBuffer[offset + i] = glyph[i];
    }
}


void loop() {
    if (Serial.available()) {
      char c = Serial.read();
    }
    drawGlyph8x8(10, 0, char8x8_A);
    drawGlyph8x8(10, 1, char8x8_B);
    drawGlyph8x8(10, 2, char8x8_a);
    drawGlyph8x8(10, 3, char8x8_b);

    drawGlyph8x8(10, 4, char8x8_A);
    drawGlyph8x8(10, 5, char8x8_B);
    drawGlyph8x8(10, 6, char8x8_a);
    drawGlyph8x8(10, 7, char8x8_b);
    updateDisplay();
}

