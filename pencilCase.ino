#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define PIN_BUTTON 2
#define PIN_PLAYING 1
#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.'
#define SPRITE_JUMP_LOWER 4
#define SPRITE_GROUND_EMPTY ' '
#define SPRITE_GROUND_SOLID 5
#define SPRITE_GROUND_SOLID_RIGHT 6
#define SPRITE_GROUND_SOLID_LEFT 7
#define HUMAN_HORIZONTAL_POSITION 1    
#define GROUND_WIDTH 16
#define GROUND_EMPTY 0
#define GROUND_LOWER_BLOCK 1
#define GROUND_UPPER_BLOCK 2
#define HUMAN_POSITION_OFF 0          
#define HUMAN_POSITION_RUN_LOWER_1 1  
#define HUMAN_POSITION_RUN_LOWER_2 2  
#define HUMAN_POSITION_JUMP_1 3       
#define HUMAN_POSITION_JUMP_2 4       
#define HUMAN_POSITION_JUMP_3 5       
#define HUMAN_POSITION_JUMP_4 6       
#define HUMAN_POSITION_JUMP_5 7       
#define HUMAN_POSITION_JUMP_6 8       
#define HUMAN_POSITION_JUMP_7 9       
#define HUMAN_POSITION_JUMP_8 10      
#define HUMAN_POSITION_RUN_UPPER_1 11 
#define HUMAN_POSITION_RUN_UPPER_2 12 
#define BUZZER 7
#define VIBE 9
LiquidCrystal_I2C lcd(0x27, 16, 2);
static char groundUpper[GROUND_WIDTH + 1];
static char groundLower[GROUND_WIDTH + 1];
static bool buttonPushed = false;
void initializeDisplay() {
  static byte dot_pic[] = {
    // Run position 1
    B01100,
    B01100,
    B00000,
    B01110,
    B11100,
    B01100,
    B11010,
    B10011,
    // Run position 2
    B01100,
    B01100,
    B00000,
    B01100,
    B01100,
    B01100,
    B01100,
    B01110,
    // Jump
    B01100,
    B01100,
    B00000,
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    // Jump lower
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,
    // Ground
    B11111,
    B10101,
    B10101,
    B11011,
    B11011,
    B10101,
    B10101,
    B11111,
    // Ground right
    B11111,
    B10101,
    B10101,
    B11011,
    B11011,
    B10101,
    B10101,
    B11111,
    // Ground left
    B11111,
    B10101,
    B10101,
    B11011,
    B11011,
    B10101,
    B10101,
    B11111,
  };
  int i;
  for (i = 0; i < 7; ++i) {
    lcd.createChar(i + 1, &dot_pic[i * 8]);
  }
  for (i = 0; i < GROUND_WIDTH; ++i) {
    groundUpper[i] = SPRITE_GROUND_EMPTY;
    groundLower[i] = SPRITE_GROUND_EMPTY;
  }
}

void forwardGround(char* ground, byte newGround) {
  for (int i = 0; i < GROUND_WIDTH; ++i) {
    char current = ground[i];
    char next;
    if (i == GROUND_WIDTH - 1) next = newGround;
    else next = ground[i + 1];
    switch (current) {
      case SPRITE_GROUND_EMPTY:
        if(next == SPRITE_GROUND_SOLID) ground[i] = SPRITE_GROUND_SOLID_RIGHT;
        else ground[i] = SPRITE_GROUND_EMPTY;
        break;
      case SPRITE_GROUND_SOLID:
        if(next == SPRITE_GROUND_EMPTY) ground[i] = SPRITE_GROUND_SOLID_LEFT;
        else ground[i] = SPRITE_GROUND_SOLID;      
        break;
      case SPRITE_GROUND_SOLID_RIGHT:
        ground[i] = SPRITE_GROUND_SOLID;
        break;
      case SPRITE_GROUND_SOLID_LEFT:
        ground[i] = SPRITE_GROUND_EMPTY;
        break;
    }
  }
}

bool drawHUMAN(byte position, char* groundUpper, char* groundLower, unsigned int score) {
  bool dead = false;
  char upperSave = groundUpper[HUMAN_HORIZONTAL_POSITION];
  char lowerSave = groundLower[HUMAN_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
    case HUMAN_POSITION_OFF:
      upper = lower = SPRITE_GROUND_EMPTY;
      break;
    case HUMAN_POSITION_RUN_LOWER_1:
      upper = SPRITE_GROUND_EMPTY;
      lower = SPRITE_RUN1;
      break;
    case HUMAN_POSITION_RUN_LOWER_2:
      upper = SPRITE_GROUND_EMPTY;
      lower = SPRITE_RUN2;
      break;
    case HUMAN_POSITION_JUMP_1:
    case HUMAN_POSITION_JUMP_8:
      upper = SPRITE_GROUND_EMPTY;
      lower = SPRITE_JUMP;
      break;
    case HUMAN_POSITION_JUMP_2:
    case HUMAN_POSITION_JUMP_7:
      upper = SPRITE_JUMP_UPPER;
      lower = SPRITE_JUMP_LOWER;
      break;
    case HUMAN_POSITION_JUMP_3:
    case HUMAN_POSITION_JUMP_4:
    case HUMAN_POSITION_JUMP_5:
    case HUMAN_POSITION_JUMP_6:
      upper = SPRITE_JUMP;
      lower = SPRITE_GROUND_EMPTY;
      break;
    case HUMAN_POSITION_RUN_UPPER_1:
      upper = SPRITE_RUN1;
      lower = SPRITE_GROUND_EMPTY;
      break;
    case HUMAN_POSITION_RUN_UPPER_2:
      upper = SPRITE_RUN2;
      lower = SPRITE_GROUND_EMPTY;
      break;
  }
  if (upper != ' ') {
    groundUpper[HUMAN_HORIZONTAL_POSITION] = upper;
    if(upperSave == SPRITE_GROUND_EMPTY) dead = false;
    else dead = true;
  }
  if (lower != ' ') {
    groundLower[HUMAN_HORIZONTAL_POSITION] = lower;
    if(lowerSave == SPRITE_GROUND_EMPTY) dead |= false;
    else dead |= true;    
  }

  byte digits;
  if(score > 9999) digits = 5;
  else if(score > 999) digits = 4;
  else if(score > 99) digits = 3;
  else if(score > 9) digits = 2;
  else digits = 1;

  // Draw the scene
  groundUpper[GROUND_WIDTH] = '\0';
  groundLower[GROUND_WIDTH] = '\0';
  char temp = groundUpper[16 - digits];
  groundUpper[16 - digits] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(groundUpper);
  groundUpper[16 - digits] = temp;
  lcd.setCursor(0, 1);
  lcd.print(groundLower);

  lcd.setCursor(16 - digits, 0);
  lcd.print(score);

  groundUpper[HUMAN_HORIZONTAL_POSITION] = upperSave;
  groundLower[HUMAN_HORIZONTAL_POSITION] = lowerSave;
  return dead;
}

void buttonPush() {
  buttonPushed = true;
}
void Sound_Start()
{
  tone (BUZZER, 329 );
  delay(150);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 329 );
  delay(200);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 329 );
  delay(150);
  noTone(BUZZER);
  delay(200);

  tone (BUZZER, 261 );
  delay(100);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 329 );
  delay(200);
  noTone(BUZZER);
  delay(200);

  tone (BUZZER, 392 );
  delay(400);
  noTone(BUZZER);
  delay(250);

  tone (BUZZER, 196 );
  delay(350);
  noTone(BUZZER);
  delay(500);
}
void Sound_GameOver()
{
  tone (BUZZER, 493 / 2 );
  delay(100);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 698 / 2 );
  delay(250);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 698 / 2 );
  delay(150);
  noTone(BUZZER);
  delay(50);
  tone (BUZZER, 698 / 2 );
  delay(150);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 659 / 2 );
  delay(150);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 587 / 2 );
  delay(150);
  noTone(BUZZER);
  tone (BUZZER, 523 / 2 );
  delay(110);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 329 / 2 );
  delay(110);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 196 / 2 );
  delay(110);
  noTone(BUZZER);
  tone (BUZZER, 329 / 2 );
  delay(110);
  noTone(BUZZER);
  delay(50);

  tone (BUZZER, 261 / 2 );
  delay(150);
  noTone(BUZZER);
  delay(50);
}
void setup() {
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(PIN_PLAYING, OUTPUT);
  digitalWrite(PIN_PLAYING, HIGH);
  pinMode(BUZZER, OUTPUT);

  // Digital pin 2 maps to interrupt 0
  attachInterrupt(0, buttonPush, FALLING);

  initializeDisplay();

  lcd.init();
  lcd.backlight();
}

bool start = true;
bool over = true;
void loop() {
  static byte HUMANPos = HUMAN_POSITION_RUN_LOWER_1;
  static byte newGroundType = GROUND_EMPTY;
  static byte newGroundDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  while (start && !buttonPushed) {
    lcd.setCursor(0, 0);
    lcd.print("Press Start!");
  }
  if (start && buttonPushed) start = !start;
  if (!playing) {
    byte pos;
    if(blink) pos = HUMAN_POSITION_OFF;
    else pos = HUMANPos;
    drawHUMAN(pos, groundUpper, groundLower, distance >> 3);
    if (blink) {
      if (over) {
        analogWrite(VIBE, 150);
        lcd.setCursor(0, 0);
        lcd.print("Game Over!");
        delay(300);
        analogWrite(VIBE, 0);
      }
      lcd.setCursor(0, 0);
      lcd.print("Game Over!");
      if (over) Sound_GameOver();
      over = false;
      lcd.setCursor(0, 1);
      lcd.print("Try Again!");
      delay(1500);
    }
    delay(100);
    blink = !blink;
    if (buttonPushed) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("READY..");
      Sound_Start();
      initializeDisplay();
      HUMANPos = HUMAN_POSITION_RUN_LOWER_1;
      playing = true;
      over = true;
      buttonPushed = false;
      distance = 0;
    }
    return;
  }

  byte newground;
  if(newGroundType == GROUND_LOWER_BLOCK) newground = SPRITE_GROUND_SOLID;
  else newground =SPRITE_GROUND_EMPTY;
  forwardGround(groundLower, newground);
  
  if(newGroundType == GROUND_UPPER_BLOCK) newground = SPRITE_GROUND_SOLID;
  else newground =SPRITE_GROUND_EMPTY;
  forwardGround(groundUpper, newground);

  if (--newGroundDuration == 0) {
    if (newGroundType == GROUND_EMPTY) {
      if(random(3) == 0) newGroundType = GROUND_UPPER_BLOCK;
      else newGroundType = GROUND_LOWER_BLOCK;
      newGroundDuration = 10 + random(6);
    } else {
      newGroundType = GROUND_EMPTY;
      newGroundDuration = 10 + random(6);
    }
  }

  if (buttonPushed) {
    if (HUMANPos <= HUMAN_POSITION_RUN_LOWER_2) HUMANPos = HUMAN_POSITION_JUMP_1;
    buttonPushed = false;

  }

  if (drawHUMAN(HUMANPos, groundUpper, groundLower, distance >> 3)) {
    playing = false; // The HUMAN died.
    for (int i = 0; i <= 2; i++) {
    }
  } else {
    if (HUMANPos == HUMAN_POSITION_RUN_LOWER_2 || HUMANPos == HUMAN_POSITION_JUMP_8) {
      HUMANPos = HUMAN_POSITION_RUN_LOWER_1;
    } else if ((HUMANPos >= HUMAN_POSITION_JUMP_3 && HUMANPos <= HUMAN_POSITION_JUMP_5) && groundLower[HUMAN_HORIZONTAL_POSITION] != SPRITE_GROUND_EMPTY) {
      HUMANPos = HUMAN_POSITION_RUN_UPPER_1;
    } else if (HUMANPos >= HUMAN_POSITION_RUN_UPPER_1 && groundLower[HUMAN_HORIZONTAL_POSITION] == SPRITE_GROUND_EMPTY) {
      HUMANPos = HUMAN_POSITION_JUMP_5;
    } else if (HUMANPos == HUMAN_POSITION_RUN_UPPER_2) {
      HUMANPos = HUMAN_POSITION_RUN_UPPER_1;
    } else {
      ++HUMANPos;
    }
    ++distance;
    if(groundLower[HUMAN_HORIZONTAL_POSITION + 2] == SPRITE_GROUND_EMPTY) digitalWrite(PIN_PLAYING, HIGH);
    else digitalWrite(PIN_PLAYING, LOW);
  }
}
