#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROMex.h>

#define BUZ_ON   PORTB |= _BV(5);
#define BUZ_OFF  PORTB &= ~_BV(5);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

byte m1 = 1;
byte m2 = 2;
byte m3 = 30; //30
byte m4 = 0;
byte t;
int sensorValue = 0;
int sensorValue2 = 0;
double aa;
double bb;

byte buttons[3] = {4,5,6};
byte pressedButton;

String units[] = { "dBm", "mW " };
String calculationMode[] = { "CM", "AM", "MM" };
//int frequency[] = { 27, 144, 433, 868, 915, 1200, 2400, 5800 };
struct params_t {
  int frequency;
  byte aaCell;
  byte bbCell;
};
const params_t params[8] = {
  {27, 10, 15},
  {144, 20, 25},
  {433, 30, 35},
  {868, 40, 45},
  {915, 50, 55},
  {1200, 60, 65},
  {2400, 70, 75},
  {5800, 80, 85}
};

void printDouble2( double val, byte precision){
 // prints val with number of decimal places determine by precision
 // precision is a number from 0 to 6 indicating the desired decimial places
 // example: lcdPrintDouble( 3.1415, 2); // prints 3.14 (two decimal places)

  if(val < 0.0){
    Serial.print('-');
    val = -val;
  }

  Serial.print (int(val));  //prints the int part
  if( precision > 0) {
    Serial.print("."); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--) {
      mult *=10;
    }
    if(val >= 0) {
      frac = (val - int(val)) * mult;
    } else {
      frac = (int(val) - val ) * mult;
    }

    unsigned long frac1 = frac;
    while( frac1 /= 10 ) {
      padding--;
    }
    while(  padding--) {
      Serial.print("0");
    }
    Serial.println(frac,DEC) ;
  }
}

void DisplayText(byte x, byte y, bool color, byte size, String text) {
  if (color == 0) {
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK, WHITE);
  }
  display.setCursor(x, y);
  display.setTextSize(size);
  display.print(text);
}

byte checkButtonPress() {
  byte bP = 0;
  byte rBp = 0;
  for (t = 0; t < (sizeof(buttons)/sizeof(buttons[0])); t++) {
    if (digitalRead(buttons[t]) == 0) { bP = (t + 1); }
  }
  rBp = bP;
  while (bP != 0) { // wait while the button is still down
    bP = 0;
    for (t = 0; t < (sizeof(buttons)/sizeof(buttons[0])); t++) {
      if (digitalRead(buttons[t]) == 0) { bP = (t + 1); }
    }
  }
  return rBp;
}

void callibrationMode(){
  char junk = ' ';
  display.clearDisplay();
  DisplayText(10, 10, 0, 2, "Call Menu");
  display.display();
  while(1){
    Serial.println("MENU"); //Prompt User for input
    while (Serial.available()==0) {             //Wait for user input
    }
    junk = Serial.read() ;

    Serial.print(EEPROM.readInt(0));
    Serial.println("------------------");
    for (byte i=0; i<(sizeof(params)/sizeof(params[0])); i++) {
      aa = EEPROM.readDouble(params[i].aaCell);
      Serial.print((String)"Reg " + params[i].aaCell + " a" + params[i].frequency + " = ");
      printDouble2(aa, 5); 
      bb = EEPROM.readDouble(params[i].bbCell);
      Serial.print((String)"Reg " + params[i].bbCell + " b" + params[i].frequency + " = ");
      printDouble2(bb, 5); 
    }
    Serial.println("------------------");
    Serial.println("Enter Reg");

    while (Serial.available()==0){}
    int tmp1 = Serial.parseInt();
    junk = Serial.read();

    Serial.print("reg ="); Serial.println(tmp1, DEC);

    Serial.println("Enter call=");      //Prompt User for input
    while (Serial.available() == 0) {}
    aa = Serial.parseFloat();
    EEPROM.writeDouble(tmp1,aa);
    Serial.print("= "); Serial.println(aa, DEC);
    while (Serial.available() > 0) {
      // .parseFloat() can leave non-numeric characters
      junk = Serial.read() ; // clear the keyboard buffer
    }
  }
}

void RawMode(){
  analogReference(INTERNAL);
  while (1) {
    sensorValue=1023;
    for (int i = 0; i <= 800; i++) {
      sensorValue2 = analogRead(A0);
      if (sensorValue2 < sensorValue){ sensorValue = sensorValue2; }
    }
    DisplayText(0, 0, 0, 3, String(sensorValue));
    DisplayText(0, 32, 0, 3, String(analogRead(A0)));
    DisplayText(0, 57, 0, 1, "raw max/raw");
    display.display();
    delay(50);
    display.clearDisplay();
  }
}

void FindMode(){
  word i, i2, d=50, dd;
  while (1) {
    i = analogRead(A0);
    dd=150;
    if (i<i2){
      d=i2-i;
      if (d > 100){ d=100; }
        d=d/10;
        if (d==1){dd=80;}
        if (d==2){dd=40;}
        if (d==3){dd=20;}
        if (d==4){dd=10;}
        if (d==5){dd=6;}
        if (d==6){dd=5;}
        if (d==7){dd=4;}
        if (d==8){dd=3;}
        if (d==9){dd=2;}
        if (d==10){dd=1;}
    }
    display.clearDisplay();
    DisplayText(10, 10, 0, 2, "Find Mode");
    DisplayText(40, 30, 1, 2, String(1023-i));
    display.display();
    if (digitalRead(5) == LOW) {
      i2=i;
      display.display();
    }
    BUZ_ON;
    delay(10);
    BUZ_OFF;
    delay(dd);
  }
}

void FillMode(){
  // Fill Mode
  display.clearDisplay();
  while (1) {
    for (int i = 0; i <= 128; i++) {
      display.drawFastVLine (i, (analogRead(A0) / 9), 64, 1); //10
      display.display();
    }
    display.clearDisplay();
  }
}

void setup() {

  pinMode(9, OUTPUT);
  BUZ_ON;
  delay(100);
  BUZ_OFF;
  delay(400);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  delay(500);
  display.clearDisplay();  // Clear the buffer.
  DisplayText(15, 5, 0, 2, "ExpertRC");
  DisplayText(5, 25, 0, 2, "PowerMeter");
  DisplayText(0, 45, 0, 1, "firmware: 0.1.27");
  DisplayText(0, 57, 0, 1, "serial:a000" + String(EEPROM.readInt(0)));
  display.display();
  delay(2000);
  
  // Button
  for (t = 0; t < (sizeof(buttons)/sizeof(buttons[0])); t++) {
    pinMode(buttons[t], INPUT_PULLUP); 
  }
  
  //Ground better
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  pinMode(21, OUTPUT);
  digitalWrite(21, LOW);

  pressedButton = checkButtonPress();
  switch(pressedButton) {
    case 1:
      FindMode();
      break;
    case 2:
      FillMode();
      break;
//    case 3:
//      RawMode();
//      break;
    case 3:
      callibrationMode();
      break;
   }
}


void loop() {
  bool reset = 0;
  ReadSettings();
  ReadCallibrationData();
  analogReference(INTERNAL);

  while (1) {
    display.clearDisplay();
    DisplayText(0, 0, 1, 2, String(params[m1].frequency) + "MHz");
    DisplayText(100, 0, 1, 2, calculationMode[m2]);
    DisplayText(0, 18, 0, 2, "Attn dB=" + String(m3));
    DisplayText(0, 39, 0, 2, units[m4]);

    switch (m2) {
      // Continuous mode
      case 0:
        sensorValue = analogRead(A0);
        break;
      // Average mode
      case 1:
        sensorValue2 = 0;
        for (int i = 0; i < 30; i++) {
          sensorValue2 += analogRead(A0);
        }
        sensorValue = (sensorValue2 / 30 + sensorValue) / 2;
        break;
      // Maximum mode
      case 2:
        sensorValue = 1023;
        for (int i = 0; i <= 800; i++) {
          sensorValue2 = analogRead(A0);
          if (sensorValue2 < sensorValue){ 
            sensorValue = sensorValue2;
          } 
        }
        break;
    }

    double dBm = (sensorValue / aa * -1) + bb + m3;
    double Pm = pow( 10.0, (dBm) / 10.0); //pwr in mW

    pressedButton = checkButtonPress();
    switch(pressedButton) {
      case 1: case 3:
        sensorValue = 1023;
        sensorValue2 = 0;
        dBm = 0;
        Pm = 0;
        reset = 1;
        break;
      case 2:
        sensorValue = 1023;
        sensorValue2 = 0;
        FrequencyMenu();
        CalculationModeMenu();
        AttenuatorMenu();
        UnitsMenu();
        WriteSettings();
        return;
    }

    switch (m4) {
      case 0:
        DisplayText(40, 39, 0, 2, String(dBm));
        DisplayText(0, 57, 0, 1, "Pm=" + String(Pm));
        break;
      case 1:
        DisplayText(40, 39, 0, 2, String(Pm));
        DisplayText(0, 57, 0, 1, "dBm=" + String(dBm));
        break;
    }
 
    display.display();
    if (reset) {
      reset = 0;
      delay(1000);
    }
    delay(50);
    display.clearDisplay();
  }
}

void FrequencyMenu(void) {
  sensorValue = 0;
  sensorValue2 = 0;
  while (1) {
    display.clearDisplay();
    DisplayText(0, 0, 0, 2, "Frequency");
    DisplayText(0, 45, 1, 2, String(params[m1].frequency) + "MHz");
    pressedButton = checkButtonPress();
    if (pressedButton !=0) {
      switch (pressedButton) {
        case 1:
          m1++;
          if (m1 > 7) {
            m1 = 0;
          }
          break;
        case 3:
          if (m1 == 0) {
            m1 = 8;
          }
          m1--;
          break;
        case 2:
          return;
      }
    }
    display.display();
  }
}

void CalculationModeMenu(void) {
  while (1) {
    display.clearDisplay();
    DisplayText(0, 0, 0, 2, "Mode");
    DisplayText(0, 45, 1, 2, calculationMode[m2]);
    pressedButton = checkButtonPress();
    if (pressedButton !=0) {
      switch (pressedButton) {
        case 1:
          m2++;
          if (m2 > 2) {
            m2 = 0;
          }
          break;
        case 3:
          if (m2 == 0) {
            m2 = 3;
          }
          m2--;
          break;
        case 2:
          return;
      }
    }
    display.display();
  }
}


void AttenuatorMenu(void) {
  char c[3];
  while (1) {
    display.clearDisplay();
    sprintf(c, "%3d", m3);
    DisplayText(0, 0, 0, 2, "Attenuator");
    DisplayText(0, 45, 1, 2, c);
    pressedButton = checkButtonPress();
    if (pressedButton !=0) {
      switch (pressedButton) {
        case 1:
          m3++;
          if (m3 > 100) {
            m3 = 0;
          }
          break;
        case 3:
          if (m3 == 0) {
            m3 = 101;
          }
          m3--;
          break;
        case 2:
          return;
      }
    }
    display.display();
  }
}

//db mw
void UnitsMenu(void) {
  while (1) {
    display.clearDisplay();
    DisplayText(0, 0, 0, 2, "Units");
    DisplayText(0, 45, 1, 2, units[m4]);
    pressedButton = checkButtonPress();
    if (pressedButton !=0) {
      switch (pressedButton) {
        case 1:
          m4++;
          if (m4 > 1) {
            m4 = 0;
          }
          break;
        case 3:
          if (m4 == 0) {
            m4 = 2;
          }
          m4--;
          break;
        case 2:
          return;
      }
    }
    display.display();
  }
}

void ReadSettings(){
  m1 = EEPROM.readByte(2);
  m2 = EEPROM.readByte(3);
  m3 = EEPROM.readByte(4);
  m4 = EEPROM.readByte(5);
}
  
void WriteSettings(){
  EEPROM.writeByte(2,m1);
  EEPROM.writeByte(3,m2);
  EEPROM.writeByte(4,m3);
  EEPROM.writeByte(5,m4);
}

void ReadCallibrationData(){
  aa = EEPROM.readDouble(params[m1].aaCell);
  bb = EEPROM.readDouble(params[m1].bbCell);
}
