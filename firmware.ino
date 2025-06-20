/******************************************************************************************
 *                                CPCESP Keyboard Firmware                               *
 *----------------------------------------------------------------------------------------*
 * Project:   Custom PS2 Keyboard for Amstrad CPC464 Emulator - CPCESP                    *
 * Author:    Archer                                                                      *
 * Date:      25/09/2024                                                                  *
 *----------------------------------------------------------------------------------------*
 * Description:                                                                           *
 * - Implements a custom PS2 keyboard firmware for the CPCESP emulator.                   *
 * - Uses the original Amstrad CPC464 keyboard matrix layout.                             *
 * - Includes an additional FN key to enable an extra layout tailored for the emulator.   *
 *                                                                                        *
 * Features:                                                                              *
 * - Full support for the Amstrad CPC464 original key mapping.                            *
 * - FN key functionality for alternate key layouts specific to the CPCESP emulator.      *
 * - Optimized for compatibility with standard PS2 keyboards.                             *
 *                                                                                        *
 ******************************************************************************************/
#include "ps2_lib.hpp"

#define VERSION 1.0.3
#define BYTE_SEND_WAIT 200

uint8_t last_key_offset = 0;
uint8_t last_key_scancode = 0;
bool key_repeat = false;

uint32_t last_key_trigger_time = 0;


const uint8_t scancode[160] = {
  // Layout 1
  0x42, 0x43, 0x40, 0x41, 0x46, 0x47, 0x00, 0x66,
  0x16, 0x1E, 0x76, 0x15, 0x0D, 0x1C, 0x58, 0x1A,
  0x25, 0x26, 0x24, 0x1D, 0x1B, 0x23, 0x21, 0x22,
  0x36, 0x2E, 0x2D, 0x2C, 0x34, 0x2B, 0x32, 0x2A,
  0x3E, 0x3D, 0x3C, 0x35, 0x33, 0x3B, 0x31, 0x29,
  0x45, 0x46, 0x44, 0x43, 0x4B, 0x42, 0x3A, 0x41,
  0x55, 0x4E, 0x54, 0x4D, 0x52, 0x4C, 0x4A, 0x49,
  0x71, 0x5B, 0x5A, 0x5D, 0x25, 0x12, 0x0E, 0x14,
  0x6B, 0x2F, 0x3D, 0x3E, 0x2E, 0x16, 0x1E, 0x45,
  0x75, 0x74, 0x72, 0x46, 0x36, 0x26, 0x5A, 0x49,
  
  // Layout 2 (FN)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0B, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0A, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x07, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x14,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t scancode_extended[160] = {
  // Layout 1
  0xE2, 0xE2, 0xE2, 0xE2, 0xE2, 0xE2, 0xE2, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x00,

  // Layout 2
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

bool key_status[80];

PS2 kbd;


void sendPS2Key(byte key_code) {
  uint8_t key_extended = scancode_extended[key_code];
  if (key_extended) {
    kbd.sendByte(key_extended);
    delayMicroseconds(BYTE_SEND_WAIT);
  }
  kbd.sendByte(scancode[key_code]);
}


void sendPS2BreakKey(byte key_code) {
  uint8_t key_extended = scancode_extended[key_code];
  if (key_extended) {
    kbd.sendByte(key_extended);
    delayMicroseconds(BYTE_SEND_WAIT);
  }
  kbd.sendByte(0xF0);
  delayMicroseconds(BYTE_SEND_WAIT);
  kbd.sendByte(scancode[key_code]);
}


void activeCol(int row) {
  uint16_t estado = 0x0001;
  estado <<= (9 - row);
  uint8_t portb_mask = estado & 0x3F;
  uint8_t portc_mask = (estado >> 6) & 0x0F;

  PORTC |= 0x0F;
  PORTB |= 0x3F;

  PORTC &= ~portc_mask;
  PORTB &= ~portb_mask;
}


void setup() {
  DDRB |= 0x3F;
  DDRC |= 0x0F;
  DDRD = 0x00;

  PORTB |= 0x3F;
  PORTC |= 0x0F;
  PORTD = 0xFF;

  pinMode(A6, INPUT);
  kbd.setRepeatTimes(0x42);   // Delay = 500ms, Rate = 20.7Hz
  delay(10);
}


void loop() { 
  int fn_key_status = analogRead(A6);

  uint8_t fn_page = 0;
  if (fn_key_status > 512) {
    fn_page = 80;
  }

  if (kbd.available()) kbd.executeCommand();

  // --- Keyboard Matrix Scan Function ---
  // This loop scans a 10x8 keyboard matrix to detect the state of each key.
  // It uses direct port reading (PIND) to determine if a key is pressed,
  // and manages sending corresponding PS/2 scancodes (make and break) to the system.
  for (int row = 0; row < 10; row++) {
    int rowm = row * 8;
    
    activeCol(row);

    for (int col = 0; col < 8; col++) {
      uint8_t estadoPuertoD = PIND;
      uint8_t estadoInvertido = ~estadoPuertoD;
      uint8_t x_col = (estadoInvertido >> col) & 0x01;
      uint8_t offset = rowm + col;
      uint8_t scancode_offset = offset + fn_page;
      
      // --- Key Detection Logic ---
      if (x_col) { // If the column bit is 1, it means the key is pressed.
        if (!key_status[offset]) {
          sendPS2Key(scancode_offset);
          key_status[offset] = true;
          
          // Stores information about the last pressed key for repeat logic.
          last_key_offset = offset;
          last_key_scancode = scancode_offset;

          last_key_trigger_time = millis();
          key_repeat = false;
        }
      } else { // If the column bit is 0, the key is not pressed.
        if (key_status[offset]) {
          sendPS2BreakKey(scancode_offset);
          key_status[offset] = false;
          
          last_key_offset = 0;
          last_key_scancode = 0;
          last_key_trigger_time = 0;
        }
      }
    }
  }

  // --- Key Repetition Management ---
  // This block of code manages the automatic repetition of a key when it's held down.
  // It implements a standard keyboard behavior: an initial delay before the first repeat,
  // followed by a faster, continuous repetition rate.
  if (key_status[last_key_offset]) {
    uint32_t current_time = millis();
    uint32_t elapsed_time = current_time - last_key_trigger_time;

    uint16_t time_delay = key_repeat ? kbd.getRepeatRate() : kbd.getRepeatDelay();

    if (elapsed_time > time_delay) {
      last_key_trigger_time = millis();
      key_repeat = true;
      sendPS2Key(last_key_scancode);
    }
  }
}
