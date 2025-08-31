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

#define DEC_A 2
#define DEC_B 3
#define DEC_C 4
#define DEC_D 5

#define COL_A 6   // LED Top
#define COL_B 7   // LED Upper-Right
#define COL_C 8   // LED Lower-Right
#define COL_D 9   // LED Bottom
#define COL_E 10  // LED Lower-Left
#define COL_F 11  // LED Upper-Left
#define COL_G 12  // LED Center segment

int cols[7] = { COL_A, COL_B, COL_C, COL_D, COL_E, COL_F, COL_G };

#define CHAR_TABLE 17  // number of elements in segs table.
int segs[CHAR_TABLE][7] = {
  // t,ru,rl,b,ll,lu,c
  { 1, 1, 1, 1, 1, 1, 0 },  // 0
  { 0, 1, 1, 0, 0, 0, 0 },  // 1
  { 1, 1, 0, 1, 1, 0, 1 },  // 2
  { 1, 1, 1, 1, 0, 0, 1 },  // 3
  { 0, 1, 1, 0, 0, 1, 1 },  // 4
  { 1, 0, 1, 1, 0, 1, 1 },  // 5
  { 1, 0, 1, 1, 1, 1, 1 },  // 6
  { 1, 1, 1, 0, 0, 0, 0 },  // 7
  { 1, 1, 1, 1, 1, 1, 1 },  // 8
  { 1, 1, 1, 1, 0, 1, 1 },  // 9
  { 1, 1, 1, 0, 1, 1, 1 },  // A
  { 0, 0, 1, 1, 1, 1, 1 },  // b
  { 1, 0, 0, 1, 1, 1, 0 },  // C
  { 0, 1, 1, 1, 1, 0, 1 },  // d
  { 1, 0, 0, 1, 1, 1, 1 },  // E
  { 1, 0, 0, 0, 1, 1, 1 },  // F
  { 0, 0, 0, 0, 0, 0, 1 }   // -

};

//       A    B    C    D    E    F    G
// --------------------------------------
//  0 |   6    5    4    3    2    1    0
//  1 |   D    C    B    A    9    8    7
//  2 |   PC   GO   +    DA   AD   F    E
//  3 |   x    x    x    x    x    x    x
char keys[3][7] = {
  { '6', '5', '4', '3', '2', '1', '0' },
  { 'D', 'C', 'B', 'A', '9', '8', '7' },
  { 'P', 'G', '+', 'X', 'Y', 'F', 'E' }  // P=PG, G=Go, X=DA, Y=AD

};

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

  pinMode(DEC_A, OUTPUT);
  pinMode(DEC_B, OUTPUT);
  pinMode(DEC_C, OUTPUT);
  pinMode(DEC_D, OUTPUT);

  testDisplay();
}

// the loop routine runs over and over again forever:
void loop() {
  keyboardTest();
  //analogWrite(LED, 0);

  //delay(300);
  //analogWrite(LED, 200);
}

void testDisplay() {
  // Output Mode COL_x lines
  pinMode(COL_A, OUTPUT);
  pinMode(COL_B, OUTPUT);
  pinMode(COL_C, OUTPUT);
  pinMode(COL_D, OUTPUT);
  pinMode(COL_E, OUTPUT);
  pinMode(COL_F, OUTPUT);
  pinMode(COL_G, OUTPUT);

  // Set the decoder line/column to perform work on.
  // Perform test on each segment array.
  for (int i = 4; i < 10; i++) {
    digitalWrite(DEC_A, i & 0x1);
    digitalWrite(DEC_B, (i >> 1) & 0x1);
    digitalWrite(DEC_C, (i >> 2) & 0x1);
    digitalWrite(DEC_D, (i >> 3) & 0x1);

    // Exercise display segments.
    for (int j = 0; j < CHAR_TABLE; j++) {
      seg(j);  // Show char i from segs table.
      delay(200);
    }
  }
}

// LED Segments Test
// Assert COL_x for each segment
void keyboardTest() {
  // Output Mode COL_x lines
  // Input Pullup Mode COL_x lines.
  pinMode(COL_A, INPUT_PULLUP);
  pinMode(COL_B, INPUT_PULLUP);
  pinMode(COL_C, INPUT_PULLUP);
  pinMode(COL_D, INPUT_PULLUP);
  pinMode(COL_E, INPUT_PULLUP);
  pinMode(COL_F, INPUT_PULLUP);
  pinMode(COL_G, INPUT_PULLUP);

  // Set the decoder line/column to perform work on.
  // Perform test on each segment array.
  for (int i = 0; i < 4; i++) {
    digitalWrite(DEC_A, i & 0x1);
    digitalWrite(DEC_B, (i >> 1) & 0x1);
    digitalWrite(DEC_C, (i >> 2) & 0x1);
    digitalWrite(DEC_D, (i >> 3) & 0x1);

    if (i == 3) continue;  // No presses on row 3.

    int press = scanCols(i);
    if (press < 7) {
      char key = keys[i][press];
      if (lastKey == key) {continue;}
      Serial.print("Key pressed: ");
      Serial.println(key);
      lastKey = key;
    }
  }
}


// Keyboard Test
// Read each COL_x. 0==pressed.
int scanCols(int row) {
  for (int i = 0; i < 7; i++) {
    if (!digitalRead(cols[i])) {  // Not pressed = 1.
      return i;
    }
  }

  return 7;
}

void seg(int i) {
  digitalWrite(COL_A, segs[i][0]);
  digitalWrite(COL_B, segs[i][1]);
  digitalWrite(COL_C, segs[i][2]);
  digitalWrite(COL_D, segs[i][3]);
  digitalWrite(COL_E, segs[i][4]);
  digitalWrite(COL_F, segs[i][5]);
  digitalWrite(COL_G, segs[i][6]);
}
