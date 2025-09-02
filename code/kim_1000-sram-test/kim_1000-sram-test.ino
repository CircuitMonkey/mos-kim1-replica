/*
  Keyboard and Display Module tester for KIM-1000

  This code will test/exercise the individual board module used the
  KIM-1000 using a RPi2040 Pico.

  Pico was chosen as it works at 3V IO levels. Most ATmega are 5V.

  by Circuit Monkey - 2025

  License: MIT
  This code is in the public domain. Free to study/copy/change.
*/
#define LED LED_BUILTIN  // the pin the LED is attached to

#define PHI 19
#define RW 20
#define RAM_RW 21

#define K5 22
#define K6 26
#define K7 27

#define ADDR_SZ 8

// Only test eight bits of address. Unless we have more GPIO pins to use.
// On bread board, tie A8-15 low.
int ADDR[ADDR_SZ] = { 2, 3, 4, 5, 6, 7, 8, 9 };

int DATA[8] = { 10, 11, 12, 13, 14, 16, 17, 18 };  // GP15 <== do not use


char lastKey = '*';  // Undefined.

// the setup routine runs once when you press reset:
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
    delay(1000);
  }
  analogWrite(LED, 200);

  pinMode(PHI, OUTPUT);
  pinMode(RW, OUTPUT);
  pinMode(RAM_RW, OUTPUT);
  pinMode(K5, OUTPUT);
  pinMode(K6, OUTPUT);
  pinMode(K7, OUTPUT);

  digitalWrite(PHI, LOW);
  digitalWrite(RW, HIGH);
  digitalWrite(RAM_RW, HIGH);
  enableChip(HIGH);  // Disable chip, K5, K6, K7 low.

  for (int i = 0; i < 8; i++) {
    pinMode(ADDR[i], OUTPUT);
  }

  ramTestSimple();  // Read write patterns to one address 0000
  ramTestAll();     // Read write patterns to all addresses.
  ramTestBlock();  // Sequential block test.
}

// the loop routine runs over and over again forever:
void loop() {

  
}

// Usage:  setDataBus( OUTPUT|INPUT_PULLUP )
void setDataDir(int dir) {
  for (int i = 0; i < 8; i++) {
    pinMode(DATA[i], dir);
  }
}

void setAddress(int addr) {
  for (int i = 0; i < ADDR_SZ; i++) {
    digitalWrite(ADDR[i], (addr >> i) & 0x1);
  }
}

void enableChip(int state) {
  //LOW = EN, HIGH = disable
  digitalWrite(K5, ~state);  // Kn are inverted.
  digitalWrite(K6, ~state);
  digitalWrite(K7, ~state);
}

// Toggle RAM Write.
void toggleWrite() {
  digitalWrite(RAM_RW, LOW);
  digitalWrite(RAM_RW, HIGH);
}

// Make sure WE is high.
// Set address
// Set data
// Make sure OE is high.

// Assert CE
// Toggle WE low-high

void writeByte(int addr, int val) {
  enableChip(HIGH);            // Disable chip. Should already be high.
  setDataDir(OUTPUT);          // Write to SRAM
  digitalWrite(RAM_RW, HIGH);  // High to start with.

  // Output enable. No output.
  digitalWrite(RW, LOW);  // Flipped for now. Needs PCB fix.
  digitalWrite(PHI, LOW);

  setAddress(addr);

  // Set up data value.
  for (int i = 0; i < 8; i++) {
    digitalWrite(DATA[i], (val >> i) & 0x1);
  }

  enableChip(LOW);  // Enable chip.  LOW == enable
  toggleWrite();

  enableChip(HIGH);  // Disable chip
}

int readByte(int addr) {
  enableChip(HIGH);            // Disable chip. Should already be high.
  digitalWrite(RAM_RW, HIGH);  // High for reads.

  setDataDir(INPUT_PULLUP);

  // Output enable of SRAM. Tri-state.
  digitalWrite(RW, LOW);  // Flipped for now. Needs PCB fix.
  digitalWrite(PHI, LOW);

  setAddress(addr);

  enableChip(LOW);  // Enable chip

  // Output enable of SRAM. Apply output to bus.
  digitalWrite(PHI, HIGH);
  digitalWrite(RW, HIGH);

  // Data should be on the bus now.
  int val = 0;
  for (int i = 7; i >= 0; i--) {
    val = (val << 1) | (digitalRead(DATA[i]) & 0x01);
  }
  // Output enable of SRAM. Tri-state.
  digitalWrite(PHI, LOW);
  digitalWrite(RW, LOW);

  enableChip(HIGH);  // Disable chip

  return val;
}

// Read write patterns to one address 0000
void ramTestSimple() {
  int testVal = random(256);
  writeByte(0, testVal);
  int val = readByte(0);
  Serial.print("Read/Write test:  ");
  if (val != testVal) {
    Serial.print("MIS-MATCH ==>");
    Serial.print("   wrote: ");
    Serial.print(testVal);
    Serial.print("   read: ");
    Serial.println(val);
  } else {
    Serial.println("OK.");
  }
}

void ramTestAll() {
  for (int addr = 0; addr < 0xFF; addr++) {
    int testVal = random(256);
    writeByte(addr, testVal);
    int val = readByte(addr);
    Serial.print("Read/Write [");
    Serial.print(addr);
    Serial.print("]:  ");
    if (val != testVal) {
      Serial.print("MIS-MATCH ==>");
      Serial.print("   wrote: ");
      Serial.print(testVal);
      Serial.print("   read: ");
      Serial.println(val);
    } else {
      Serial.println("OK.");
    }
  }
}

// Write entire block. Read block back.
void ramTestBlock() {
  for (int addr = 0; addr <= 0xFF; addr++) {
    int testVal = (~addr) + 1;
    writeByte(addr, testVal);
    Serial.print("Write [");
    Serial.print(addr);
    Serial.println("]");
  }

  // Red and compare expected values.
  for (int addr = 0; addr <= 0xFF; addr++) {
    int testVal = (~addr + 1) & 0xFF;
    int val = readByte(addr);
    Serial.print("Read [");
    Serial.print(addr);
    Serial.print("]:  ");
    if (val != testVal) {
      Serial.print("MIS-MATCH ==>");
      Serial.print("   wrote: ");
      Serial.print(testVal);
      Serial.print("   read: ");
      Serial.println(val);
    } else {
      Serial.println("OK.");
    }
  }
}