// C++ Code for CMSC 130 Final Software Project
/* Program Proponents: 
        [HARDWARE AND SOFTWARE] CASQUEJO, Jann Dave Rhodore G. 
        [SOFTWARE] CABAÑERO, Jomi Arielle A. 
        [HARDWARE] DAGOHOY, Dave Laurence 
        [HARDWARE] NEBRIA, Quennie 
        [HARDWARE] ORGANIZA, Trixie Nicole 
        [SOFTWARE] PALARPALAR, Cherlie 
        [SOFTWARE] TACLINDO, Borgy Lance C.
*/
// Submission and Presentation Date: 07 June 2024
#include <LiquidCrystal.h>

/* PIN DEFINITIONS */
#define PIN_BUTTON 2
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 12

/* DISPLAY CONSTANT VALUES for Code Optimization and Clarity */
#define SPRITE_RUN1 1 // Hero Run Position 1
#define SPRITE_RUN2 2 // Hero Run Position 2
#define SPRITE_JUMP 3 // Hero Jump Animation
#define SPRITE_JUMP_UPPER '.' // Use the '.' character for the Hero's head
#define SPRITE_JUMP_LOWER 4 // Hero's Body in Jump
#define SPRITE_TERRAIN_EMPTY ' ' // Assumes a whitespace character for empty terrains
#define SPRITE_TERRAIN_SOLID 5 // The terrain is solid on current hero position
#define SPRITE_TERRAIN_SOLID_RIGHT 6 // The terrain is still on the right side of hero (approaching)
#define SPRITE_TERRAIN_SOLID_LEFT 7 // The terrain has already passed by hero and is on the left side (leaving)

/* CONSTANT POSITION VALUE for Hero */
#define HERO_HORIZONTAL_POSITION 1    // Horizontal position of hero on screen

/* CONSTANT VALUES FOR TERRAIN Display */
#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

/* CONSTANT VALUES FOR HERO STATES (Current Position and Visibility) */
#define HERO_POSITION_OFF 0          // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1  // Hero is on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2  //                      (pose 2)

/* CONSTANT VALUES FOR HERO STATES (Jumping) */
#define HERO_POSITION_JUMP_1 3       // Starting a jump
#define HERO_POSITION_JUMP_2 4       // Half-way up
#define HERO_POSITION_JUMP_3 5       // Jump is on upper row
#define HERO_POSITION_JUMP_4 6       // Jump is on upper row
#define HERO_POSITION_JUMP_5 7       // Jump is on upper row
#define HERO_POSITION_JUMP_6 8       // Jump is on upper row
#define HERO_POSITION_JUMP_7 9       // Half-way down
#define HERO_POSITION_JUMP_8 10      // About to land

/* CONSTANT VALUES FOR HERO STATES (Jumping) */
#define HERO_POSITION_RUN_UPPER_1 11 // Hero is running on upper row (pose 1)
#define HERO_POSITION_RUN_UPPER_2 12 //                              (pose 2)

// Arrays to hold the Terrain Values of the Upper and Lower half of the LCD
static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];

// Boolean variable that holds if there is Button Push Input
static bool buttonPushed = false;

LiquidCrystal lcd(11, 9, 6, 5, 4, 3); // Initializes LCD Display Pins

void initializeGraphics(){
  // Defines all the movements for the Hero in the LCD Displau
  static byte graphics[] = {
    // Run Position 1 
    B01100, B01100, B00000, B01110, B11100, B01100, B11010, B10011,
    
    // Run Position 2
    B01100, B01100, B00000, B01100, B01100, B01100, B01100, B01110,

    // Jump
    B01100, B01100, B00000, B11110, B01101, B11111, B10000, B00000,
    
    // Jump Lower
    B11110, B01101, B11111, B10000, B00000, B00000, B00000, B00000,
    
    // Ground
    B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111,
    
    // Ground Right
    B00011, B00011, B00011, B00011, B00011, B00011, B00011, B00011,
    
    // Ground Left
    B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000,
  };
  
  
  int i;
  // Skip using character 0 for every hero position
  // This allows lcd.print() to be used to quickly draw multiple characters
  for (i = 0; i < 7; ++i) {
	  lcd.createChar(i + 1, &graphics[i * 8]);
  }

  // Sets the upper and lower terrains with no values (no white blocks in display)
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}

// Slide the terrain to the left in half-character increments
void advanceTerrain(char* terrain, byte newTerrain){
  char current, next;
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    current = terrain[i];
    next = (i == TERRAIN_WIDTH-1) ? newTerrain : terrain[i+1];

    // Cases for Terrain States
    switch (current){
      case SPRITE_TERRAIN_EMPTY: // If the terrain is empty on the right, a terrain will be created that moves towards the left (the hero can run through the empty display)
        terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID_RIGHT : SPRITE_TERRAIN_EMPTY;
        break;
      case SPRITE_TERRAIN_SOLID: // If a terrain already exists at the current position, then the incoming terrains are either moved to the left or extended to the right
        terrain[i] = (next == SPRITE_TERRAIN_EMPTY) ? SPRITE_TERRAIN_SOLID_LEFT : SPRITE_TERRAIN_SOLID;
        break;
      case SPRITE_TERRAIN_SOLID_RIGHT: // If a terrain is still at the right side, then it sets the terrain array at the  
        terrain[i] = SPRITE_TERRAIN_SOLID; 
        break;
      case SPRITE_TERRAIN_SOLID_LEFT: // If the terrain has passed through hero, then the rterrain at the hero's left side is cleared
        terrain[i] = SPRITE_TERRAIN_EMPTY;
        break;
    }
  }
}

bool drawHero(byte position, char* terrainUpper, char* terrainLower, unsigned int score) {
  bool collide = false;
  char upperSave = terrainUpper[HERO_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
    case HERO_POSITION_OFF:
      upper = lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_LOWER_1:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN1;
      break;
    case HERO_POSITION_RUN_LOWER_2:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN2;
      break;
    case HERO_POSITION_JUMP_1:
    case HERO_POSITION_JUMP_8:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_JUMP;
      break;
    case HERO_POSITION_JUMP_2:
    case HERO_POSITION_JUMP_7:
      upper = SPRITE_JUMP_UPPER;
      lower = SPRITE_JUMP_LOWER;
      break;
    case HERO_POSITION_JUMP_3:
    case HERO_POSITION_JUMP_4:
    case HERO_POSITION_JUMP_5:
    case HERO_POSITION_JUMP_6:
      upper = SPRITE_JUMP;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_UPPER_1:
      upper = SPRITE_RUN1;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_UPPER_2:
      upper = SPRITE_RUN2;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
  }
  if (upper != ' ') {
    terrainUpper[HERO_HORIZONTAL_POSITION] = upper;
    collide = (upperSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  if (lower != ' ') {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  
  // Draw the scene
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  char temp = terrainUpper[16-digits];
  terrainUpper[16-digits] = '\0';
  lcd.setCursor(0,0);
  lcd.print(terrainUpper);
  terrainUpper[16-digits] = temp;  
  lcd.setCursor(0,1);
  lcd.print(terrainLower);
  
  lcd.setCursor(16 - digits,0);
  lcd.print(score);

  terrainUpper[HERO_HORIZONTAL_POSITION] = upperSave;
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}

// Handle the button push as an interrupt
void buttonPush() {
  buttonPushed = true;
}

void setup(){
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);
  
  // Digital pin 2 maps to interrupt 0
  attachInterrupt(0/*PIN_BUTTON*/, buttonPush, FALLING); 
  
  initializeGraphics();
  
  lcd.begin(16, 2);
}

void loop(){
  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = TERRAIN_EMPTY;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  
  if (!playing) {
    drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainUpper, terrainLower, distance >> 3);
    if (blink) {
      lcd.setCursor(0,0);
      lcd.print("Press Start");
    }
    delay(250);
    blink = !blink;
    if (buttonPushed) {
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
    }
    return;
  }

  // Shift the terrain to the left
  advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
  advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
  
  // Make new terrain to enter on the right
  if (--newTerrainDuration == 0) {
    if (newTerrainType == TERRAIN_EMPTY) {
      newTerrainType = (random(3) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK;
      newTerrainDuration = 2 + random(10);
    } else {
      newTerrainType = TERRAIN_EMPTY;
      newTerrainDuration = 10 + random(10);
    }
  }
    
  if (buttonPushed) {
    if (heroPos <= HERO_POSITION_RUN_LOWER_2) heroPos = HERO_POSITION_JUMP_1;
    buttonPushed = false;
  }  

  if (drawHero(heroPos, terrainUpper, terrainLower, distance >> 3)) {
    playing = false; // The hero collided with something. Too bad.
  } else {
    if (heroPos == HERO_POSITION_RUN_LOWER_2 || heroPos == HERO_POSITION_JUMP_8) {
      heroPos = HERO_POSITION_RUN_LOWER_1;
    } else if ((heroPos >= HERO_POSITION_JUMP_3 && heroPos <= HERO_POSITION_JUMP_5) && terrainLower[HERO_HORIZONTAL_POSITION] != SPRITE_TERRAIN_EMPTY) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else if (heroPos >= HERO_POSITION_RUN_UPPER_1 && terrainLower[HERO_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY) {
      heroPos = HERO_POSITION_JUMP_5;
    } else if (heroPos == HERO_POSITION_RUN_UPPER_2) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else {
      ++heroPos;
    }
    ++distance;
    
    digitalWrite(PIN_AUTOPLAY, terrainLower[HERO_HORIZONTAL_POSITION + 2] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
  }
  delay(50);
}


//Sound SectionM