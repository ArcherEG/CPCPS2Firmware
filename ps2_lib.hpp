
#define SET_DATA_MODE_OUTPUT DDRC |= (1 << PC5);
#define SET_DATA_MODE_INPUT DDRC &= ~(1 << PC5);
#define SET_CLOCK_MODE_OUTPUT DDRC |= (1 << PC4);
#define SET_CLOCK_MODE_INPUT DDRC &= ~(1 << PC4);

#define SET_DATA_LOW PORTC &= ~(1 << PC5);
#define SET_DATA_HIGH PORTC |= (1 << PC5);
#define SET_CLOCK_LOW PORTC &= ~(1 << PC4);
#define SET_CLOCK_HIGH PORTC |= (1 << PC4);

#define CLKFULL 40
#define CLKHALF 20

#define TIMEOUT 30


class PS2 {
  public:
    void init();
    void setListenMode();
    void setSendMode();
    uint8_t readByte();
    void sendByte(uint8_t byte_code);
    bool available();
    uint8_t executeCommand();

  private:
    void waitHigh();
    void sendBit(bool bit);
    void generateClockPulse();
    bool readBitWithClock();
    void sendACK();
    uint8_t led_status;
};


void PS2::sendACK() {
  delayMicroseconds(800);
  sendByte(0xFA);
}


uint8_t PS2::executeCommand() {
  uint8_t scancode = 0x00;
  
  uint8_t cmd = readByte();

  if (cmd == 0xFF) {
    sendACK();
    delay(100);
    sendByte(0xAA);
  } else if (cmd == 0xF0) {
    sendACK();
    scancode = readByte();
    sendACK();
  } else if (cmd = 0xED) {
    sendACK();
    led_status = readByte();
    delay(10);
    sendACK();
  } else {
    sendACK();
  }

  return cmd;
}


void PS2::init() {
  uint8_t scancode = 0x00;
  led_status = 0xFF;

  while (led_status == 0xFF) {
    if (available()) executeCommand();
  }
}


void PS2::setListenMode() {
  SET_DATA_MODE_INPUT
  SET_CLOCK_MODE_INPUT
}


void PS2::setSendMode() {
  SET_DATA_HIGH
  SET_CLOCK_HIGH
  SET_DATA_MODE_OUTPUT
  SET_CLOCK_MODE_OUTPUT 
}


void PS2::waitHigh() {
  while (!(PINC & (1 << PC4)) || !(PINC & (1 << PC5))) {}
}

void PS2::sendBit(bool bit) {
  if (bit) {
    SET_DATA_HIGH
  } else {
    SET_DATA_LOW
  }
  delayMicroseconds(10);

  SET_CLOCK_LOW
  delayMicroseconds(40);

  SET_CLOCK_HIGH
  delayMicroseconds(30);
}


void PS2::sendByte(uint8_t byte_code) {
  waitHigh();
  setSendMode();

  sendBit(0);

  byte parity = 1;
  for (int i = 0; i < 8; i++) {
      sendBit(byte_code & 1);
      parity ^= (byte_code & 1);
      byte_code >>= 1;
  }
  sendBit(parity);
  sendBit(1);

  SET_CLOCK_HIGH
  SET_DATA_HIGH
  setListenMode();
}


bool PS2::available() {
  return !(PINC & (1 << PC5));
}


void PS2::generateClockPulse() {
  delayMicroseconds(CLKHALF);
  SET_CLOCK_LOW
  delayMicroseconds(CLKFULL);
  SET_CLOCK_HIGH
  delayMicroseconds(CLKHALF);
}


bool PS2::readBitWithClock() {
    generateClockPulse();
    return (PINC & (1 << PC5)) ? 1 : 0;
}

uint8_t PS2::readByte() {
  unsigned int data = 0x00;
  unsigned int bit = 0x01;

  unsigned char parity = 1;
  unsigned char read_parity = 0;

  while (!(PINC & (1 << PC4))) {}

  delayMicroseconds(100);
  
  SET_CLOCK_HIGH
  SET_CLOCK_MODE_OUTPUT
  for (int i = 0; i < 8; i++) {
    if (readBitWithClock()) {
      data |= (1 << i);
    }
  }

  readBitWithClock(); // parity
  readBitWithClock();
  
  waitHigh();
  SET_DATA_MODE_OUTPUT
  SET_DATA_LOW
  generateClockPulse();

  SET_DATA_HIGH
  SET_CLOCK_HIGH
  setListenMode();

  return data;
}





