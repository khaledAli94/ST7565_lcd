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


unsigned char lcdbuffer[LCD_WIDTH * LCD_PAGES];  // Full buffer: 128 × 8 = 1024 bytes = 8192 bits
/*
  lcd : page major 
  Page 0 (y = 0–7):     128 bytes
  Page 1 (y = 8–15):    128 bytes
  Page 2 (y = 16–23):   128 bytes
  ...
  Page 7 (y = 56–63):   128 bytes
*/

void initDisplaySPI() {
  // Hardware reset
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialization sequence
  sendCommandSPI(RESET);     // Reset
  delay(10);
  
  sendCommandSPI(BIAS_1_9);     // Bias 1/9
  sendCommandSPI(SEG_DIR_NORMAL);     // Segment rev
  sendCommandSPI(COM_SCAN_NORMAL);     // COM norm
  
  sendCommandSPI(POWER_ALL_ON);     // Power all on
  sendCommandSPI(REG_RESISTOR_MID);     // Regulator mid
  
  sendCommandSPI(ENTER_VOLUME_MODE);     // Enter contrast mode
  sendCommandSPI(CONTRAST_MID);     // Contrast value
  
  sendCommandSPI(DISPLAY_NORMAL);     // Normal display
  sendCommandSPI(SET_START_LINE);     // Start line 0
  sendCommandSPI(DISPLAY_ON);     // Display ON
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
    sendCommandSPI(SET_PAGE | page);
    sendCommandSPI(SET_COLUMN_HIGH);
    sendCommandSPI(SET_COLUMN_LOW);

    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      sendDataSPI(0x00);
      lcdbuffer[page * LCD_WIDTH + col] = 0x00;
    }
  }
}

// Update entire display from buffer
void updateDisplay() {
  for (uint8_t page = 0; page < LCD_PAGES; page++) {
    sendCommandSPI(SET_PAGE | page);
    sendCommandSPI(SET_COLUMN_HIGH);
    sendCommandSPI(SET_COLUMN_LOW);
    
    digitalWrite(CMD_PIN, HIGH);
    digitalWrite(CS_PIN, LOW);
    
    // Fast block transfer for entire page
    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      SPI.transfer(lcdbuffer[page * LCD_WIDTH + col]);
    }
    
    digitalWrite(CS_PIN, HIGH);
  }
}

uint16_t setPixel(unsigned char *fb, uint8_t x, uint8_t y) {
  uint8_t page = y >> 3;
  uint8_t bit  = y & 0x07; //y % 8
  uint16_t offset = page * LCD_WIDTH + x;

  fb[offset] |= (1 << bit);
  // Update display (efficient: update only changed byte)
  sendCommandSPI(SET_PAGE | page);
  sendCommandSPI(SET_COLUMN_HIGH | (x >> 4));
  sendCommandSPI(SET_COLUMN_LOW | (x & 0x0F));
  sendDataSPI(fb[offset]);
  return offset;
}

uint16_t clearPixel(unsigned char *fb, uint8_t x, uint8_t y) {
  uint8_t page = y >> 3;
  uint8_t bit  = y & 0x07;
  uint16_t offset = page * LCD_WIDTH + x;

  fb[offset] &= ~(1 << bit);
  // Update display (efficient: update only changed byte)
  sendCommandSPI(SET_PAGE | page);
  sendCommandSPI(SET_COLUMN_HIGH | (x >> 4));
  sendCommandSPI(SET_COLUMN_LOW | (x & 0x0F));
  sendDataSPI(fb[offset]);
  return offset;
}

// row major order font
static const uint8_t char8x8_A[] = { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00};   // U+0041 (A)
static const uint8_t char8x8_B[] = { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00};   // U+0042 (B)

static const uint8_t char8x8_a[] = { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00};  // U+0061 (a)
static const uint8_t char8x8_b[] = { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00};   // U+0062 (b)


// void drawGlyph8x8(const uint8_t *glyph, uint8_t x, uint8_t y) {

//   for (uint8_t i = 0; i < 8; i++) {
//     lcdbuffer[offset + i] = glyph[i];
//   }

//     // uint16_t offset = page * LCD_WIDTH + col;
//     // for (uint8_t i = 0; i < 8; i++) {
//     //     lcdbuffer[offset + i] = glyph[i];
//     // }
// }

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

void loop() {
    if (Serial.available()) {
      char c = Serial.read();
    }
    setPixel(lcdbuffer, 0, 0);
    setPixel(lcdbuffer, 127, 0);

    // drawGlyph8x8(char8x8_A, 0, 0);
    // drawGlyph8x8(char8x8_A, 0, 7);

    // drawGlyph8x8(char8x8_A, 8, 0);
    // drawGlyph8x8(char8x8_B, 8, 1);
    // drawGlyph8x8(char8x8_a, 8, 2);
    // drawGlyph8x8(char8x8_b, 8, 3);

    // drawGlyph8x8(char8x8_A, 8, 4);
    // drawGlyph8x8(char8x8_B, 8, 5);
    // drawGlyph8x8(char8x8_a, 8, 6);
    // drawGlyph8x8(char8x8_b, 8, 7);
    updateDisplay();
}
