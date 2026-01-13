#include <Arduino.h>

// Pin definitions - ST7565 SPI interface
#define SCLK_PIN 13  // Clock
#define SID_PIN 11   // Data
#define CS_PIN 10    // Chip Select
#define A0_PIN 9     // Command/Data
#define RST_PIN 8    // Reset

// Display dimensions
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_PAGES 8

// Orientation settings
bool isFlipped = true;  // Control screen orientation

void setup() {
  // Initialize pins
  pinMode(SCLK_PIN, OUTPUT);
  pinMode(SID_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(A0_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialize serial
  Serial.begin(115200);
  while (!Serial);
  Serial.println("ST7565 LCD Controller - Fix Orientation");
  
  // Initialize display with CORRECT orientation
  initDisplay();
  
  // Clear and test
  clearDisplay();
  drawTestPattern();
  
  Serial.println("Ready. Commands:");
  Serial.println("C - Clear");
  Serial.println("T - Test pattern");
  Serial.println("F - Flip orientation");
  Serial.println("N - Normal orientation");
  Serial.println("R - Reset");
  Serial.println("Px,y - Set pixel");
  Serial.println("Lx1,y1,x2,y2 - Draw line");
  Serial.println("? - Help");
}

// SPI write function
void spiWrite(uint8_t data) {
  for (int8_t i = 7; i >= 0; i--) {
    digitalWrite(SCLK_PIN, LOW);
    digitalWrite(SID_PIN, (data >> i) & 0x01);
    digitalWrite(SCLK_PIN, HIGH);
  }
  digitalWrite(SCLK_PIN, LOW);
}

// Send command
void sendCommand(uint8_t cmd) {
  digitalWrite(A0_PIN, LOW);
  digitalWrite(CS_PIN, LOW);
  spiWrite(cmd);
  digitalWrite(CS_PIN, HIGH);
}

// Send data
void sendData(uint8_t data) {
  digitalWrite(A0_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  spiWrite(data);
  digitalWrite(CS_PIN, HIGH);
}

// Initialize display with orientation control
void initDisplay() {
  Serial.println("Initializing display...");
  
  // Hardware reset
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  delay(50);
  
  // Reset command
  sendCommand(0xE2);
  delay(10);
  
  // Bias: 1/9 bias (0xA2) or 1/7 bias (0xA3)
  sendCommand(0xA2);
  
  // ORIENTATION SETTINGS - THIS IS WHAT FIXES UPSIDE DOWN
  // -------------------------------------------------------
  if (isFlipped) {
    // For upside down screens, try these combinations:
    
    // OPTION 1: Mirror horizontally and vertically
    sendCommand(0xA1);  // ADC = 1 (segment direction reversed)
    sendCommand(0xC0);  // COM normal scan direction
    
    // OPTION 2: Just flip vertically
    // sendCommand(0xA0);  // ADC = 0 (normal segment)
    // sendCommand(0xC8);  // COM reversed scan
    
    Serial.println("Orientation: Flipped");
  } else {
    // Normal orientation (most common)
    sendCommand(0xA0);  // ADC = 0 (normal segment direction)
    sendCommand(0xC8);  // COM reversed scan direction
    
    // Alternative normal orientation:
    // sendCommand(0xA0);  // Normal segment
    // sendCommand(0xC0);  // Normal COM scan
    
    Serial.println("Orientation: Normal");
  }
  
  // Power control
  sendCommand(0x2F);  // All internal circuits on
  
  // Regulator resistor select
  sendCommand(0x26);  // Medium value
  
  // Contrast/volume
  sendCommand(0x81);  // Enter volume mode
  sendCommand(0x1F);  // Contrast value (0x00-0x3F)
  
  // Display start line (0x40 = line 0)
  sendCommand(0x40);
  
  // Display mode
  sendCommand(0xA6);  // Normal display (not inverse)
  
  // Turn display ON
  sendCommand(0xAF);  // Display ON
  
  Serial.println("Display initialized");
}

// Toggle screen orientation
void flipOrientation() {
  isFlipped = !isFlipped;
  reinitDisplay();
}

// Reinitialize display with current orientation
void reinitDisplay() {
  // Turn display off first
  sendCommand(0xAE);
  
  // Reapply orientation commands
  if (isFlipped) {
    sendCommand(0xA1);  // Reversed segment
    sendCommand(0xC0);  // Normal COM scan
  } else {
    sendCommand(0xA0);  // Normal segment
    sendCommand(0xC8);  // Reversed COM scan
  }
  
  // Turn display back on
  sendCommand(0xAF);
  
  Serial.print("Orientation: ");
  Serial.println(isFlipped ? "Flipped" : "Normal");
}

// Clear entire display
void clearDisplay() {
  for (uint8_t page = 0; page < LCD_PAGES; page++) {
    sendCommand(0xB0 | page);
    sendCommand(0x10);
    sendCommand(0x00);
    
    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      sendData(0x00);
    }
  }
  Serial.println("Display cleared");
}

// Draw comprehensive test pattern to see orientation
void drawTestPattern() {
  Serial.println("Drawing orientation test...");
  
  // Clear first
  clearDisplay();
  
  // Draw border
  for (uint8_t page = 0; page < LCD_PAGES; page++) {
    sendCommand(0xB0 | page);
    sendCommand(0x10);
    sendCommand(0x00);
    
    for (uint8_t col = 0; col < LCD_WIDTH; col++) {
      uint8_t pattern = 0x00;
      
      // Top and bottom borders (check which is which)
      if (page == 0) pattern = 0xFF;  // What should be top
      if (page == 7) pattern = 0x81;  // What should be bottom
      
      // Left and right edges
      if (col == 0 || col == 127) pattern |= 0x81;
      
      sendData(pattern);
    }
  }
  
  // Draw orientation markers
  drawChar(2, 0, 'T');  // Should appear at top-left
  drawChar(120, 0, 'R'); // Should appear at top-right
  drawChar(2, 7, 'B');   // Should appear at bottom-left
  drawChar(120, 7, 'L'); // Should appear at bottom-right
  
  // Draw arrow pointing "up"
  drawArrow(64, 3, true);
  
  // Draw text indicating orientation
  drawText(20, 2, isFlipped ? "FLIPPED" : "NORMAL");
  
  Serial.println("Test pattern drawn");
}

// Draw an arrow pointing up or down
void drawArrow(uint8_t x, uint8_t page, bool up) {
  // Set position
  sendCommand(0xB0 | page);
  sendCommand(0x10 | (x >> 4));
  sendCommand(0x00 | (x & 0x0F));
  
  // Arrow data (8x8)
  if (up) {
    // Up arrow
    sendData(0x18);  // 00011000
    sendData(0x3C);  // 00111100
    sendData(0x7E);  // 01111110
    sendData(0xFF);  // 11111111
    sendData(0x18);  // 00011000
    sendData(0x18);  // 00011000
    sendData(0x18);  // 00011000
    sendData(0x00);  // 00000000
  } else {
    // Down arrow
    sendData(0x18);  // 00011000
    sendData(0x18);  // 00011000
    sendData(0x18);  // 00011000
    sendData(0xFF);  // 11111111
    sendData(0x7E);  // 01111110
    sendData(0x3C);  // 00111100
    sendData(0x18);  // 00011000
    sendData(0x00);  // 00000000
  }
}

// Draw a character (simple 5x7 font)
void drawChar(uint8_t x, uint8_t page, char c) {
  if (x > LCD_WIDTH - 5 || page >= LCD_PAGES) return;
  
  // Simple font - numbers and letters
  const uint8_t font[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
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
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00}  // Space
  };
  
  uint8_t index;
  if (c >= '0' && c <= '9') {
    index = c - '0';
  } else if (c >= 'A' && c <= 'Z') {
    index = c - 'A' + 10;
  } else if (c >= 'a' && c <= 'z') {
    index = c - 'a' + 10;
  } else {
    index = 36; // Space
  }
  
  sendCommand(0xB0 | page);
  sendCommand(0x10 | (x >> 4));
  sendCommand(0x00 | (x & 0x0F));
  
  for (uint8_t i = 0; i < 5; i++) {
    sendData(font[index][i]);
  }
  sendData(0x00); // Space between chars
}

// Draw text string
void drawText(uint8_t x, uint8_t page, const char* text) {
  while (*text && x < LCD_WIDTH - 5) {
    drawChar(x, page, *text);
    x += 6;
    text++;
  }
}

// Set a single pixel
void setPixel(uint8_t x, uint8_t y) {
  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
  
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  
  // Set position
  sendCommand(0xB0 | page);
  sendCommand(0x10 | (x >> 4));
  sendCommand(0x00 | (x & 0x0F));
  
  // We need to draw all 8 bits of the column
  // Since we can't read from display, we'll draw just the pixel
  // by sending a byte with only that bit set
  uint8_t pixelData = 1 << bit;
  sendData(pixelData);
  
  Serial.print("Pixel at (");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.println(")");
}

// Reset display
void resetDisplay() {
  digitalWrite(RST_PIN, LOW);
  delay(100);
  digitalWrite(RST_PIN, HIGH);
  delay(100);
  initDisplay();
  Serial.println("Display reset");
}

// Process serial commands
void processCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;
  
  char c = cmd.charAt(0);
  
  switch (c) {
    case 'C':
    case 'c':
      clearDisplay();
      break;
      
    case 'T':
    case 't':
      drawTestPattern();
      break;
      
    case 'F':
    case 'f':
      flipOrientation();
      drawTestPattern(); // Redraw to show new orientation
      break;
      
    case 'N':
    case 'n':
      if (isFlipped) {
        flipOrientation(); // Switch to normal
        drawTestPattern();
      }
      break;
      
    case 'R':
    case 'r':
      resetDisplay();
      break;
      
    case '1':
      // Try orientation combination 1
      isFlipped = false;
      sendCommand(0xA0); // Normal segment
      sendCommand(0xC8); // Reverse COM
      drawTestPattern();
      break;
      
    case '2':
      // Try orientation combination 2
      isFlipped = false;
      sendCommand(0xA1); // Reverse segment
      sendCommand(0xC8); // Reverse COM
      drawTestPattern();
      break;
      
    case '3':
      // Try orientation combination 3
      isFlipped = false;
      sendCommand(0xA0); // Normal segment
      sendCommand(0xC0); // Normal COM
      drawTestPattern();
      break;
      
    case '4':
      // Try orientation combination 4
      isFlipped = false;
      sendCommand(0xA1); // Reverse segment
      sendCommand(0xC0); // Normal COM
      drawTestPattern();
      break;
      
    case 'P':
    case 'p': {
      int comma = cmd.indexOf(',');
      if (comma > 1) {
        int x = cmd.substring(1, comma).toInt();
        int y = cmd.substring(comma + 1).toInt();
        setPixel(x, y);
      }
      break;
    }
    
    case '?':
      Serial.println("=== ST7565 Orientation Tester ===");
      Serial.println("C - Clear display");
      Serial.println("T - Test pattern");
      Serial.println("F - Flip orientation");
      Serial.println("N - Normal orientation");
      Serial.println("R - Reset display");
      Serial.println("Px,y - Set pixel");
      Serial.println("1-4 - Try different orientations");
      Serial.println("? - This help");
      Serial.println("Current: " + String(isFlipped ? "FLIPPED" : "NORMAL"));
      break;
      
    default:
      Serial.println("Unknown command. Type ? for help");
      break;
  }
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
}
