// DOMINIK WUJEK - M00879927

#include <Wire.h>
#include <LiquidCrystal_I2C.h>


//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); //create the lcd object
int displayLightTime = 110; //startup display light timer value
int displayTimeAdjust = 250;
bool wakeupDone;

//LED OUTPUTS
const int blue1pin = 7;
const int green1pin = 8;
const int blue2pin = 10;
const int green2pin = 9;
const int selectLEDpin = 13;

//CONTROL BUTTON PINS
const int upButtonPin = 2;
const int downButtonPin = 5;
const int select1ButtonPin = 4;
const int select2ButtonPin = 3;

const int beepPin = 6;
int beepLevel = 255;

int serialMsgTime = 100;
int lcdMsgTime = 100;

//WATER PUMP PINS
const int ledPin = 12;
const int ledPin2 = 11;

float base = 5;  // base for the logs used to make progress bar more linear
int sensorCalibration = 326; //offset 

int modes[] = {0, 95, 100, 115, 130};
int modeIndex[] = {2, 2};

int selectedPlant = 0;

const int sensor1Pin = A3;
const int sensor2Pin = A2;


void setup() {
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(select1ButtonPin, INPUT_PULLUP);
  pinMode(select2ButtonPin, INPUT_PULLUP);
  pinMode(beepPin, OUTPUT);
  pinMode(blue1pin, OUTPUT);
  pinMode(green1pin, OUTPUT);
  pinMode(blue1pin, OUTPUT);
  pinMode(green2pin, OUTPUT);
  pinMode(selectLEDpin, OUTPUT);

  lcd.init();       // Initialize the lcd
  lcd.backlight();  // turn on the backlight

  Serial.begin(9600);
}

void serialMessage(){
}



void modeSet(int mode1, int mode2) {
  // Update the modeIndex array with the desired modes for plant 1 and plant 2
  modeIndex[0] = min(max(mode1, 0), 4); // Ensure mode1 is within the valid range [0, 4]
  modeIndex[1] = min(max(mode2, 0), 4); // Ensure mode2 is within the valid range [0, 4]
  Serial.println("modeSet triggered");
}
void setBeep(int x){
  beepLevel = x;
  Serial.print("beep value set to ");
  Serial.println(x);
}


void processSerialInput() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();

    switch (incomingByte) {
      case 'S':
  
        int mode1 = Serial.parseInt();
        // Skip the comma
        while (Serial.available() > 0 && Serial.peek() != ',') {
          Serial.read();
        }
        // Skip the comma
        if (Serial.available() > 0 && Serial.peek() == ',') {
          Serial.read();
        }
        int mode2 = Serial.parseInt();

        modeSet(mode1, mode2);
        break;
      case 'B': 

        int beepLevel = Serial.parseInt();
        setBeep(beepLevel);
        break;
    }
  }
}

void selectPlant(int x) {
  selectedPlant = x;
}

void adjustLevel(int direction) {
  modeIndex[selectedPlant] += direction;
  modeIndex[selectedPlant] = min(max(modeIndex[selectedPlant], 0), 4);
}

void water(int x) {
  digitalWrite(x, HIGH);
  if (displayLightTime > 0){
  analogWrite(beepPin, beepLevel);
  }
  delay(130);
  //if ?
  digitalWrite(beepPin, LOW);
  digitalWrite(x, LOW);
}


void printScreen(int state1, int state2, float target1, float target2) {
  displayLightTime -= 10;

  if (wakeupDone == false) {
    wakeUp();
    wakeupDone = true;
  }

  if (selectedPlant == 0) {
    lcd.setCursor(0, 0);
    lcd.print("X");
  } else if (selectedPlant == 1) {
    lcd.setCursor(0, 1);
    lcd.print("X");
  }

  float minState = log(306.0) / log(base);
  float maxState = log(500.0) / log(base);
  float logState1 = log(state1) / log(base);
  float logState2 = log(state2) / log(base);
  float normalizedState1 = (logState1 - minState) / (maxState - minState);
  float normalizedState2 = (logState2 - minState) / (maxState - minState);
  int squares1 = round(normalizedState1 * 8);
  int squares2 = round(normalizedState2 * 8);

  //Serial.print(squares1);Serial.print(" ");Serial.println(squares2);

  lcd.setCursor(1, 0);
  lcd.print(modeIndex[0]);
  //draw water level bars for each plant
  for (int i = 0; i < squares1; i++) {
    lcd.print((char)255);   // Print a solid block
  }
  lcd.setCursor(1, 1);
  lcd.print(modeIndex[1]);
    for (int i = 0; i < squares2; i++) {
    lcd.print((char)255);  // Print a solid block
  }
}


void setLEDs(int state1, int state2){
  int displayValue1 = round(state1 / 10.0);
  int displayValue2 = round(state2 / 10.0);

  analogWrite(blue1pin, (80 - displayValue1) * 4 );
  analogWrite(green1pin, displayValue1 *3 );
  analogWrite(blue2pin, (80 - displayValue2) * 4 + 90 );
  analogWrite(green2pin, displayValue2 *3 );

  if (displayLightTime > 0){
    if (selectedPlant == 0) {
      digitalWrite(selectLEDpin, LOW);
    } else if (selectedPlant == 1) {
      digitalWrite(selectLEDpin, HIGH);
    }
  }

}

void wakeUp(){
  for (int i = 2; i > 0; i--){
    digitalWrite(selectLEDpin, LOW);
    delay(50);
    digitalWrite(selectLEDpin, HIGH);
    delay(50);
  }
}
œ
void screenTimeSet(int x){
  displayTimeAdjust = x;
}


void loop() {
  processSerialInput();


  if (digitalRead(select1ButtonPin) == LOW) {
    selectPlant(0);
  } else if (digitalRead(select2ButtonPin) == LOW) {
    selectPlant(1);
  }

  if (digitalRead(upButtonPin) == LOW) {
    adjustLevel(1);
  } else if (digitalRead(downButtonPin) == LOW) {
    adjustLevel(-1);
  }

//hand over control panel detected
  if (analogRead(A1) > 150 == false) {
    displayLightTime = displayTimeAdjust;
  }


  int state1 = analogRead(sensor1Pin);
  int state2 = analogRead(sensor2Pin);
  float target1 = sensorCalibration * abs(modes[modeIndex[0]] / 100.0);
  float target2 = sensorCalibration * abs(modes[modeIndex[1]] / 100.0);

//update or switch off the display depending on the abount of time left
  if (displayLightTime == 0) {
    wakeupDone = false;
    lcd.noBacklight();
    delay(50);
  } else {
    lcd.backlight();
    printScreen(state1, state2, target1, target2);
  }

  setLEDs(state1, state2); //update LEDs 

  if (state1 < target1) {
    water(ledPin);
  } else if (state2 < target2) {
    water(ledPin2);
  }

serialMessage();
  delay(280);
  lcd.clear(); 
}