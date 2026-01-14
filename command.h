
enum cmd_t {
  // ===== DISPLAY POWER CONTROL =====
  DISPLAY_ON        = 0xAF,  // Turn display ON (pixels visible)
  DISPLAY_OFF       = 0xAE,  // Turn display OFF (pixels hidden, memory preserved)
  
  // ===== DISPLAY MODE SETTINGS =====
  DISPLAY_NORMAL    = 0xA6,  // Normal display mode (black on white)
  DISPLAY_INVERSE   = 0xA7,  // Inverse display mode (white on black)
  DISPLAY_ALL_ON    = 0xA5,  // Force all pixels ON (test mode)
  DISPLAY_ALL_OFF   = 0xA4,  // Return to normal from all-on mode
  
  // ===== ORIENTATION & MIRRORING =====
  SEG_DIR_NORMAL    = 0xA0,  // Normal segment direction (left to right)
  SEG_DIR_REVERSE   = 0xA1,  // Reverse segment direction (right to left)
  COM_SCAN_NORMAL   = 0xC0,  // Normal COM scan direction (top to bottom)
  COM_SCAN_REVERSE  = 0xC8,  // Reverse COM scan direction (bottom to top)
  
  // ===== BIAS & DUTY CYCLE =====
  BIAS_1_9          = 0xA2,  // LCD bias 1/9 (for 1/65 duty)
  BIAS_1_7          = 0xA3,  // LCD bias 1/7 (for 1/33 duty)
  
  // ===== POSITION CONTROL =====
  SET_START_LINE    = 0x40,  // Base for start line: 0x40 | (0-63)
  SET_PAGE          = 0xB0,  // Base for page: 0xB0 | (0-7)
  SET_COLUMN_HIGH   = 0x10,  // Base for column high nibble: 0x10 | (0-7)
  SET_COLUMN_LOW    = 0x00,  // Base for column low nibble: 0x00 | (0-15)
  
  // ===== CONTRAST/BRIGHTNESS =====
  ENTER_VOLUME_MODE = 0x81,  // Must be sent before contrast value
  // Contrast value: 0x00-0x3F (0x00 = darkest, 0x3F = lightest)
  CONTRAST_MIN      = 0x00,
  CONTRAST_MID      = 0x1F,
  CONTRAST_MAX      = 0x3F,
  
  // ===== POWER CONTROL =====
  POWER_CTRL_BASE   = 0x28,  // Base for power control: 0x28 | settings
  POWER_ALL_ON      = 0x2F,  // Booster+Regulator+Follower all ON
  
  // Power control bits (OR with 0x28):
  POWER_BOOSTER_OFF = 0x00,
  POWER_BOOSTER_ON  = 0x04,
  POWER_REGULATOR_OFF = 0x00,
  POWER_REGULATOR_ON  = 0x02,
  POWER_FOLLOWER_OFF = 0x00,
  POWER_FOLLOWER_ON  = 0x01,
  
  // ===== REGULATOR RESISTOR =====
  REG_RESISTOR_BASE = 0x20,  // Base for regulator: 0x20 | (0-7)
  REG_RESISTOR_MID  = 0x26,  // Medium resistor value
  
  // ===== SYSTEM COMMANDS =====
  RESET             = 0xE2,  // Software reset (like hardware reset)
  NOP               = 0xE3,  // No operation
  
  // ===== STATIC INDICATOR (for some models) =====
  STATIC_IND_OFF    = 0xAC,
  STATIC_IND_ON     = 0xAD,
  STATIC_IND_REG    = 0x00,  // Base: 0x00 | (0-15)
  
  // ===== TEST MODES =====
  ENTER_TEST_MODE   = 0xF0,  // Factory test mode (be careful!)
  EXIT_TEST_MODE    = 0xF1,  // Exit factory test mode
};
