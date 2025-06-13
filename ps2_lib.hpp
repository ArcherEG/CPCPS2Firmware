
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



const uint16_t PS2_REPEAT_DELAYS_MAP[] = {250, 500, 750, 1000};

const uint16_t PS2_REPEAT_RATES_MAP[] = {
    33, 37, 42, 45, 50, 55, 60, 66,
    75, 83, 90, 100, 111, 125, 142, 166,
    200, 250, 333, 400, 500, 666, 1000, 1250, 
    1666, 2000, 2500, 3000, 3333, 4000, 5000, 6666
};


class PS2 {
  public:
    void setListenMode();
    void setSendMode();
    uint8_t readByte();
    void sendByte(uint8_t byte_code);
    bool available();
    uint8_t executeCommand();
    void setRepeatTimes(uint8_t);
    uint16_t getRepeatDelay() const { return _repeat_delay_ms; }
    uint16_t getRepeatRate() const { return _repeat_rate_ms; }
    
  private:
    void waitHigh();
    void sendBit(bool bit);
    void generateClockPulse();
    bool readBitWithClock();
    void sendACK();
    uint8_t _led_status;
    uint16_t _repeat_delay_ms;
    uint16_t _repeat_rate_ms;
};


void PS2::setRepeatTimes(uint8_t cfg) {
    uint8_t delay_index = (cfg >> 5) & 0x03;
    uint8_t rate_index = cfg & 0x1F;

    if (delay_index < sizeof(PS2_REPEAT_DELAYS_MAP) / sizeof(PS2_REPEAT_DELAYS_MAP[0])) {
        _repeat_delay_ms = PS2_REPEAT_DELAYS_MAP[delay_index];
    } else {
        _repeat_delay_ms = PS2_REPEAT_DELAYS_MAP[0];
    }

    if (rate_index < sizeof(PS2_REPEAT_RATES_MAP) / sizeof(PS2_REPEAT_RATES_MAP[0])) {
        _repeat_rate_ms = PS2_REPEAT_RATES_MAP[rate_index];
    } else {
        _repeat_rate_ms = PS2_REPEAT_RATES_MAP[0];
    }
}


void PS2::sendACK() {
  delayMicroseconds(800);
  sendByte(0xFA);
}


uint8_t PS2::executeCommand() {
  uint8_t scancode = 0x00;
  
  uint8_t cmd = readByte();

  if (cmd == 0xFF) {          // Reset
    sendACK();
    delay(100);
    sendByte(0xAA);
  } else if (cmd == 0xF0) {   // Set/Get Scan Code Set
    sendACK();
    scancode = readByte();
    sendACK();
  } else if (cmd == 0xED) {   // Set/Reset LEDs
    sendACK();
    _led_status = readByte();
    delay(10);
    sendACK();
  } else if(cmd == 0xF3) {    // Set Typematic Rate and Delay
    sendACK();
    uint8_t cfg = readByte();
    delay(10);
    sendACK();
    setRepeatTimes(cfg);
  } else {
    sendACK();
  }

  return cmd;
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





