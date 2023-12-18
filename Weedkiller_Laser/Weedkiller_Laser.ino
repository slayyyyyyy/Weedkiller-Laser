#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte pwm = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int brightness = 100;

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int xPin = A0;
const int yPin = A1;
const int swPin = 2;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); 
byte matrixBrightness = 8;
byte xPos = 4;
byte yPos = 4;
byte xLastPos = 0;
byte yLastPos = 0;
// thresholds for joystick movement detection
const int minThreshold = 400;
const int maxThreshold = 600;
// timing variables for movement updates
const byte movementDelay = 500;
unsigned long lastMoved = 0;
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 100;
bool wasPressed = false;


const byte mapSize = 8;
bool mapChanged = true;
byte gameMap[mapSize][mapSize] =  { };
bool gameStarted = false;

const int blinkDelay = 200;
unsigned long previousBlinkMillis = 0;


bool bulletFired = false; // flag to track if the bullet is currently active
byte bulletXPos;
byte bulletYPos;
const int maxBulletRange = 2;
int bulletRange = 0;
const int bulletMoveDelay = 150; 
unsigned long previousBulletMoveMillis = 0;
const int bulletBlinkDelay = 50; 
unsigned long previousBulletBlinkMillis = 0;


enum Direction { NONE, UP, DOWN, LEFT, RIGHT };
Direction lastDirection = NONE;

enum Menu { START_GAME, SETTINGS, HOW_TO_PLAY, ABOUT};
Menu currentMenu = START_GAME;
bool insideMenuOption = false;

enum Submenu { LCD_BRIGHTNESS, BACK, MATRIX_BRIGHTNESS};
Submenu currentSubmenu = LCD_BRIGHTNESS;
bool insideSubmenu = false;
int submenuOptionNumber = 3;


const char gameName[] = "Weedkiller Laser";
const char creatorGithub[] = "slayyyyyyy";
const char creatorName[] = "Andreea Gurzu";
const char* menuNames[] = {"Start Game", "Settings","How to play", "About"};
const char* submenuNames[] = {"LCD Brightness", "Back", "Game Brightness"};
int displayDuration = 3000;

int lastDebounceTime = 0;
int lastButtonState = HIGH;
int buttonState = HIGH;

const byte firstLevel[mapSize][mapSize] = {
  {0, 1, 0, 0, 1, 1, 0, 0},
  {1, 1, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 1, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 0, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1}
};

const byte secondLevel[mapSize][mapSize] = {
  {1, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 0, 0, 1, 1, 1},
  {0, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 0, 0, 1, 0},
  {1, 1, 1, 1, 0, 0, 1, 1},
  {1, 0, 0, 1, 0, 0, 0, 0}
};

const byte thirdLevel[mapSize][mapSize] = {
  {0, 0, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 1, 1},
  {0, 1, 1, 0, 0, 0, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0}
};

const byte fourthLevel[mapSize][mapSize] = {
  {0, 0, 1, 0, 0, 1, 1, 0},
  {1, 0, 0, 1, 1, 0, 1, 0},
  {1, 1, 0, 1, 0, 0, 0, 1},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 1},
  {1, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 0, 1, 1, 0, 1, 0},
  {0, 1, 0, 1, 0, 0, 1, 1}
};

const byte fifthLevel[mapSize][mapSize] = {
  {1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 1, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {0, 0, 0, 1, 0, 0, 1, 0},
  {1, 1, 1, 1, 0, 1, 1, 0},
  {0, 1, 0, 0, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1, 0}
};

int level = 1;
unsigned long gameStartTime = 0;

byte arrowUp[] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte arrowDown[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10001,
  B01010,
  B00100
};

void generateLevelMap(byte generatedMap[mapSize][mapSize], int level) {
  /* depending on the level, this functions assigns and generate the map for each level*/
  const byte* selectedLevel = nullptr;
  switch (level) {
    case 1:
      selectedLevel = &firstLevel[0][0];
      break;
    case 2:
      selectedLevel = &secondLevel[0][0];
      break;
    case 3:
      selectedLevel = &thirdLevel[0][0];
      break;
    case 4:
      selectedLevel = &fourthLevel[0][0];
      break;
    case 5:
      selectedLevel = &fifthLevel[0][0];
      break;
    default:
      selectedLevel = &firstLevel[0][0];
      break;
  }
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      generatedMap[row][col] = *(selectedLevel + row * mapSize + col);
    }
  }
}


void setMatrixState(byte state) {
  //lights up or turns off the entire matrix
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      lc.setLed(0, row, col, state); 
    }
  }
}

void setup() {
  pinMode(swPin, INPUT_PULLUP);

  lc.shutdown(0, false);
  lc.setIntensity(0, EEPROM.get(10, matrixBrightness));
  lc.clearDisplay(0); 

  lcd.begin(16,2);
  analogWrite(pwm,EEPROM.get(0,brightness));
  lcd.createChar(2, arrowUp);
  lcd.createChar(3, arrowDown);

  displayGreeting(gameName);
  lcd.print(menuNames[currentMenu]);
  lcd.setCursor(15, 0);
  lcd.write(2);
  lcd.setCursor(15, 1);
  lcd.write(3);
}

void loop() {
  if (gameStarted) {
    printTimeAndLevel();
    gameLogic();
  } else {
    navigateMainMenu();
  }
}

void printTimeAndLevel(){
  //prints the time and the level during the game
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Time:");
    lcd.setCursor(strlen("Time:"), 0);
    lcd.print((millis() - gameStartTime) / 1000);
    lcd.setCursor(0,1);
    lcd.print("Level:");
    lcd.setCursor(strlen("Level:"), 1);
    lcd.print(level);
}

void gameLogic(){
  //function implementing how the gameplay works
  markPlayer();
  markBullet();

  if (millis() - lastMoved > movementDelay) {
    updatePlayerPosition();
    lastMoved = millis();
  }

  unsigned long currentMillis = millis();
  bool buttonState = digitalRead(swPin) == LOW;

  // checks if the button is pressed continously
  if (buttonState && !wasPressed && !bulletFired && (currentMillis - lastButtonPressTime) > debounceDelay) {
    lastButtonPressTime = currentMillis;
    shootBullet();
  }
  wasPressed = buttonState;

  if (mapChanged) {
    updateMap();
    mapChanged = false;
  }

  if (bulletFired) {
    moveBullet();
    updateMap();
  }

  if(checkGameEnded(level)){
    gameStarted = false;
    level = 1;
    xPos = 4;
    yPos = 4;
    displayGameEndedMessage();
    bulletFired = false;
  }

  if(checkLevelEnded()){
    level++;
    generateLevelMap(gameMap, level);
  }
}


bool buttonWasPressed() {
  //function for handling button presses and debounce
  int reading = digitalRead(swPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        return true;
      }
    }
  }

  lastButtonState = reading;
  return false;
}

void navigateMainMenu(){
  gameStartTime = millis();
  int yValue = analogRead(yPin);
  if(buttonWasPressed()){
    switch(currentMenu){
      case START_GAME:
        gameMap[xPos][yPos] = 1; // lights up the initial player position
        generateLevelMap(gameMap, level);
        gameStarted = true;
        break;
      case ABOUT:
        displayAbout(gameName, creatorName, creatorGithub);
        navigateMainMenu();
        break;
      case SETTINGS:
        currentSubmenu = LCD_BRIGHTNESS;
        insideSubmenu = true;
        lcd.clear();
        lcd.print(submenuNames[currentSubmenu]);
        lcd.setCursor(15, 0);
        lcd.write(2);
        lcd.setCursor(15, 1);
        lcd.write(3);
        if(insideSubmenu == true){
        navigateSettingsMenu();
        }
        break;
      case HOW_TO_PLAY:
        displayHowToPlay();
        navigateMainMenu();
        break;
    }
  }
  else {
    if (yValue < minThreshold) { //moving up
      lcd.clear();
      currentMenu = (currentMenu == START_GAME) ? ABOUT : (Menu)(currentMenu - 1);
      lcd.setCursor(0, 0);
      lcd.print(menuNames[currentMenu]);
      lcd.setCursor(15, 0);
      lcd.write(2);
      lcd.setCursor(15, 1);
      lcd.write(3);
      delay(250); 
  } else if (yValue > maxThreshold) { //moving down
      lcd.clear();
      currentMenu = (currentMenu == ABOUT) ? START_GAME : (Menu)(currentMenu + 1);
      lcd.setCursor(0, 0);
      lcd.print(menuNames[currentMenu]);
      lcd.setCursor(15, 0);
      lcd.write(2);
      lcd.setCursor(15, 1);
      lcd.write(3);
      delay(250);
  }

  // loop around if going beyond the defined menu options
  if (currentMenu < START_GAME) {
    currentMenu = ABOUT;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuNames[currentMenu]);
    delay(250);
  } else if (currentMenu > ABOUT) {
    currentMenu = START_GAME;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuNames[currentMenu]);
    delay(250);
    }
  }
}


void navigateSettingsMenu() {
  while (insideSubmenu == true) {
    int yValue = analogRead(yPin);
      switch (currentSubmenu) {
        case LCD_BRIGHTNESS:
          if (buttonWasPressed()) {
            insideMenuOption = true;
            setLCDBrightness();
            lcd.clear();
            lcd.print(submenuNames[currentMenu]);
            lcd.setCursor(15, 0);
            lcd.write(2);
            lcd.setCursor(15, 1);
            lcd.write(3);
          }
          break;
        case MATRIX_BRIGHTNESS:
          if (buttonWasPressed()) {
            insideMenuOption = true;
            setMatrixBrightness();
            lcd.clear();
            lcd.print(submenuNames[currentMenu]);
            lcd.setCursor(15, 0);
            lcd.write(2);
            lcd.setCursor(15, 1);
            lcd.write(3);
          }
          break;
        case BACK:
          if (buttonWasPressed()) {
            insideSubmenu = false;
            navigateMainMenu();
            currentMenu = START_GAME;
            lcd.clear();
            lcd.print(menuNames[currentMenu]);
            lcd.setCursor(15, 0);
            lcd.write(2);
            lcd.setCursor(15, 1);
            lcd.write(3);
          }
          break;
      
    }
    if (yValue < minThreshold) { // moving up
      currentSubmenu = (currentSubmenu + 1) % submenuOptionNumber;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(submenuNames[currentSubmenu]);
      lcd.setCursor(15, 0);
      lcd.write(2);
      lcd.setCursor(15, 1);
      lcd.write(3);
      delay(250); 
    } else if (yValue > maxThreshold) { // moving down
      currentSubmenu = (currentSubmenu - 1 + submenuOptionNumber) % submenuOptionNumber;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(submenuNames[currentSubmenu]);
      lcd.setCursor(15, 0);
      lcd.write(2);
      lcd.setCursor(15, 1);
      lcd.write(3);
      delay(250); 
    }
  }
}

void updateMap() {
  //updates the matrix display every time needed
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      lc.setLed(0, row, col, gameMap[row][col]); 
    }
  }
}


void updatePlayerPosition() {
  int xValue = analogRead(xPin); 
  int yValue = analogRead(yPin); 

  xLastPos = xPos;
  yLastPos = yPos;

  //updates the direction even if the player doesn't move
  if (xValue < minThreshold){
    lastDirection = RIGHT;
  } else if (xValue > maxThreshold){
    lastDirection = LEFT;
  } else if (yValue < minThreshold){
    lastDirection = UP;
  } else if (yValue > maxThreshold){
    lastDirection = DOWN;
}

  // actually moves the player dot on the map
  if (xValue < minThreshold && gameMap[(xPos + 1) % mapSize][yPos] != 1) {
    xPos = (xPos + 1) % mapSize;
  } else if (xValue > maxThreshold && gameMap[xPos - 1][yPos] != 1) {
    xPos = (xPos > 0) ? xPos - 1 : mapSize - 1;
  }
  if (yValue < minThreshold && gameMap[xPos][(yPos - 1) % mapSize] != 1) {
    yPos = (yPos > 0) ? yPos - 1 : mapSize - 1;
  } else if (yValue > maxThreshold && gameMap[xPos][(yPos + 1) % mapSize] != 1) {
    yPos = (yPos + 1) % mapSize;
  }


  // checks if the position has changed and update the map accordingly
  if (xPos != xLastPos || yPos != yLastPos) {
    mapChanged = true;
    gameMap[xLastPos][yLastPos] = 0; // turn off the last position
    gameMap[xPos][yPos] = 1; // turn on the new position
  }
  if(checkGameEnded(level)){
    gameMap[xLastPos][yLastPos] = 0;
  }
}

void markPlayer() {
  static bool isOn = true; 
  unsigned long currentBlinkMillis = millis();
  
  if (currentBlinkMillis - previousBlinkMillis >= blinkDelay) {
    previousBlinkMillis = currentBlinkMillis; 
    gameMap[xPos][yPos] = isOn ? 0 : 1;
    mapChanged = true;
    isOn = !isOn;
  }
}

void markBullet() {
  static bool isBulletFired = true;
  unsigned long currentBulletBlinkMillis = millis();

  if (bulletFired && (currentBulletBlinkMillis - previousBulletBlinkMillis >= bulletBlinkDelay)) {
    previousBulletBlinkMillis = currentBulletBlinkMillis;
    gameMap[bulletXPos][bulletYPos] = isBulletFired ? 0 : 1;
    mapChanged = true;
    isBulletFired = !isBulletFired;
  }
}

void shootBullet() {
    // shoots a bullet from the current player position
    bulletXPos = xPos;
    bulletYPos = yPos;
    bulletFired = true;
    mapChanged = true;
}


void moveBullet() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousBulletMoveMillis >= bulletMoveDelay) { // makes the movement animation longer
    previousBulletMoveMillis = currentMillis;

    gameMap[bulletXPos][bulletYPos] = 0; // clears the bullet from the current position
    // checks the last direction and move the bullet accordingly
    switch (lastDirection) {
      case UP:
        bulletYPos = (bulletYPos > 0) ? bulletYPos - 1 : 0;
        break;
      case DOWN:
        bulletYPos = (bulletYPos < mapSize - 1) ? bulletYPos + 1 : mapSize - 1;
        break;
      case LEFT:
        bulletXPos = (bulletXPos > 0) ? bulletXPos - 1 : 0;
        break;
      case RIGHT:
        bulletXPos = (bulletXPos < mapSize - 1) ? bulletXPos + 1 : mapSize - 1;
        break;
      default:
        break;
    }
    
    // checks if the bullet exceeded its maximum range
    if (bulletRange >= maxBulletRange) {
      bulletFired = false;
      bulletRange = 0;
    } else {
      gameMap[bulletXPos][bulletYPos] = 1; // turns on the bullet at the new position
      bulletRange++;
      mapChanged = true;
    }
  }
}

bool checkLevelEnded() {
  // iterate through the game map and check if any LED other than the player's position is still on
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      if (!(row == xPos && col == yPos) && gameMap[row][col] == 1) {
        return false; // level is not over yet
      }
    }
  }
  return true; // level ended condition: all LEDs are off except the player's position
}

bool checkGameEnded(int level) {
  bool allLEDsOff = true;
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      if (!(row == xPos && col == yPos) && gameMap[row][col] == 1) {
        allLEDsOff = false; // LEDs other than the player's position are still on
        break;
      }
    }
    if (!allLEDsOff) {
      break;
    }
  }
  // check if all LEDs are off and the level has reached 5
  if (allLEDsOff && level >= 5) {
    return true; // game ended
  } else {
    return false; 
  }
}

void displayGameEndedMessage() {
  //displays a message informing the player the game ended; needs a button press to return to main menu
  lcd.clear();
  int congratsPosition = (16 - strlen("Congrats, you")) / 2;
  lcd.setCursor(congratsPosition, 0);
  lcd.print("Congrats, you");
  int gameBeatenPosition = (16 - strlen("beat the game!")) / 2;
  lcd.setCursor(gameBeatenPosition, 1);
  lcd.print("beat the game!");

  bool returnToMenu = false;

  while (!returnToMenu) {
    if (buttonWasPressed()) {
      returnToMenu = true; // set the flag to exit the loop and return to the main menu
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(menuNames[currentMenu]);
    }
  }
}

void displayGreeting(const char *message) {
  //displays a greeting message for a number of seconds when powering up the game
  unsigned long startTime = millis();
  bool displayActive = true;

  lcd.clear(); 

  int welcomePosition = (16 - strlen("Welcome to")) / 2;
  lcd.setCursor(welcomePosition, 0);
  lcd.print("Welcome to");

  int gameNamePosition = (16 - strlen(message)) / 2;
  lcd.setCursor(gameNamePosition, 1); 
  lcd.print(message); 

  while (displayActive) {
    unsigned long currentTime = millis();
    if (currentTime - startTime >= displayDuration) {
      displayActive = false;
      lcd.clear(); 
    }
  }
}

void displayAbout(const char *gameName, const char *creatorName, const char *creatorGithub) {
  //displays information about the creator of the game for number of seconds.
  unsigned long startTime = millis();
  bool displayActive = true;

  lcd.clear(); 

  int gameNamePosition = (16 - strlen("Game Name")) / 2;
  lcd.setCursor(gameNamePosition, 0);
  lcd.print("Game Name:");
  int namePosition = (16 - strlen(gameName)) / 2;
  lcd.setCursor(namePosition, 1);
  lcd.print(gameName);
  delay(1500);
  lcd.clear();

  int creatorPosition = (16 - strlen("Creator:")) / 2;
  lcd.setCursor(creatorPosition, 0); 
  lcd.print("Creator:"); 
  int creatorNamePosition = (16 - strlen(creatorName)) / 2;
  lcd.setCursor(creatorNamePosition, 1);
  lcd.print(creatorName);
  delay(1500);
  lcd.clear();

  int githubPosition = (16 - strlen("Github user:")) / 2;
  lcd.setCursor(githubPosition, 0); 
  lcd.print("Github:"); 
  int githubUserPosition = (16 - strlen(creatorGithub)) / 2;
  lcd.setCursor(githubUserPosition, 1);
  lcd.print(creatorGithub);
  delay(1500);
  lcd.clear();


   while (displayActive) {
    unsigned long currentTime = millis();
    if (currentTime - startTime >= displayDuration) {
      displayActive = false;
      lcd.clear(); 
      lcd.print(menuNames[currentMenu]);
    }
  }
}

void displayHowToPlay() {
  unsigned long startTime = millis();
  bool displayActive = true;
  lcd.setCursor(0, 0);

  // Print the message:
  lcd.print("Move up, down,");
  lcd.setCursor(0, 1);
  lcd.print("left, right with");
  delay(2000); 

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("the joystick.");
  lcd.setCursor(0, 1);
  lcd.print("Shoot a laser");
  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("bullet with the");
  lcd.setCursor(0, 1);
  lcd.print("button to wipe");
  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("out the weeds.");
  delay(2000);

  while (displayActive) {
    unsigned long currentTime = millis();
    if (currentTime - startTime >= displayDuration) {
      displayActive = false;
      lcd.clear(); 
      lcd.print(menuNames[currentMenu]);
    }
  }
}



int loadLCDBrightnessFromEEPROM() {
  int storedBrightness = EEPROM.get(0, brightness);
  return storedBrightness;
}

void saveLCDBrightnessToEEPROM(int value) {
  EEPROM.put(0, value);
}

void setLCDBrightness() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Brightness Level");
  lcd.setCursor(1, 1);
  lcd.print("1  2  3  4  5");

  bool selectedBrightness = false;
  int brightnessLevels[] = {10, 25, 50, 75, 100}; // corresponding brightness levels from 1 to 5
  int index = 1;
  lcd.setCursor(index, 1);
  lcd.cursor(); 

  while (!selectedBrightness) {
    int xValue = analogRead(xPin);
    if (xValue < minThreshold && index < 13) {
      lcd.setCursor(index + 3, 1);
      index += 3;
    } else if (xValue > maxThreshold && index > 1) {
      lcd.setCursor(index - 3, 1);
      index -= 3;
    }

    if (buttonWasPressed()) {
      switch (index) {
        case 1:
          analogWrite(pwm, brightnessLevels[0]);
          saveLCDBrightnessToEEPROM(brightnessLevels[0]);
          selectedBrightness = true;
          break;
        case 4:
          analogWrite(pwm, brightnessLevels[1]);
          saveLCDBrightnessToEEPROM(brightnessLevels[1]);
          selectedBrightness = true;
          break;
        case 7:
          analogWrite(pwm, brightnessLevels[2]);
          saveLCDBrightnessToEEPROM(brightnessLevels[2]);
          selectedBrightness = true;
          break;
        case 10:
          analogWrite(pwm, brightnessLevels[3]);
          saveLCDBrightnessToEEPROM(brightnessLevels[3]);
          selectedBrightness = true;
          break;
        case 13:
          analogWrite(pwm, brightnessLevels[4]);
          saveLCDBrightnessToEEPROM(brightnessLevels[4]);
          selectedBrightness = true;
          break;
        default:
          break;
      }
    }
    delay(250);
  }
  lcd.noCursor(); // hide the cursor after selection
}


int loadMatrixBrightnessFromEEPROM() {
  int storedMatrixBrightness;
  EEPROM.get(10, storedMatrixBrightness); 
  return storedMatrixBrightness;
}

void saveMatrixBrightnessToEEPROM(int value) {
  EEPROM.put(10, value);
}

void setMatrixBrightness() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Brightness Level");
  lcd.setCursor(1, 1);
  lcd.print("1  2  3  4  5");

  bool selectedMatrixBrightness = false;
  int matrixBrightnessLevels[] = {2, 4, 6, 8, 10}; // corresponding brightness levels from 1 to 5
  int index = 1;
  lcd.setCursor(index, 1);
  lcd.cursor(); 

  while (!selectedMatrixBrightness) {
    setMatrixState(1); //lights up the matrix so you can check the change
    int xValue = analogRead(xPin);
    if (xValue < minThreshold && index < 13) {
      lcd.setCursor(index + 3, 1);
      index += 3;
    } else if (xValue > maxThreshold && index > 1) {
      lcd.setCursor(index - 3, 1);
      index -= 3;
    }

    if (buttonWasPressed()) {
      switch(index){
        case 1:
          lc.setIntensity(0, matrixBrightnessLevels[0]);
          saveMatrixBrightnessToEEPROM(matrixBrightnessLevels[0]);
          selectedMatrixBrightness = true;
          break;
        case 4:
          lc.setIntensity(0, matrixBrightnessLevels[1]);
          saveMatrixBrightnessToEEPROM(matrixBrightnessLevels[1]);
          selectedMatrixBrightness = true;
          break;
        case 7:
          lc.setIntensity(0, matrixBrightnessLevels[2]);
          saveMatrixBrightnessToEEPROM(matrixBrightnessLevels[2]);
          selectedMatrixBrightness = true;
          break;
        case 10:
          lc.setIntensity(0, matrixBrightnessLevels[3]);
          saveMatrixBrightnessToEEPROM(matrixBrightnessLevels[3]);
          selectedMatrixBrightness = true;
          break;
        case 13:
          lc.setIntensity(0, matrixBrightnessLevels[4]);
          saveMatrixBrightnessToEEPROM(matrixBrightnessLevels[4]);
          selectedMatrixBrightness = true;
          break;
        default:
          break;
      }
    }
    delay(250);
  }
    lcd.noCursor();
    setMatrixState(0);
}
