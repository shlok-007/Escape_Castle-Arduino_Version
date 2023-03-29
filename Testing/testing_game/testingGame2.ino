#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

#define ILI9163_BLACK   0x0000
#define ILI9163_BLUE    0x001F
#define ILI9163_RED     0xF800
#define ILI9163_GREEN   0x07E0
#define ILI9163_CYAN    0x07FF
#define ILI9163_MAGENTA 0xF81F
#define ILI9163_YELLOW  0xFFE0  
#define ILI9163_WHITE   0xFFFF

#define LEFT_BUTTON 2
#define RIGHT_BUTTON 3
#define JUMP_BUTTON 4

// Player properties
int playerX = 20;
int playerY = 100;
int playerWidth = 20;
int playerHeight = 20;
int playerSpeed = 5;
int jumpSpeed = 10;
bool isJumping = false;
int jumpHeight = 0;

// Level properties
int level = 1;
int keyX = 30;
int keyY = 90;
int doorX = 130;
int doorY = 70;


TFT_ILI9163C tft = TFT_ILI9163C(10, 8, 9);

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9163_BLACK);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(JUMP_BUTTON, INPUT_PULLUP);
  loadLevel(level);
}

void loop() {
  // Code for player movement
  if (digitalRead(LEFT_BUTTON)) {
    playerX -= playerSpeed;
  }
  if (digitalRead(RIGHT_BUTTON)) {
    playerX += playerSpeed;
  }
  if (digitalRead(JUMP_BUTTON) && !isJumping) {
    isJumping = true;
  }
  if (isJumping) {
    playerY -= jumpSpeed;
    jumpHeight += jumpSpeed;
    if (jumpHeight >= 60) {
      isJumping = false;
      jumpHeight = 0;
    }
  } else {
    playerY += playerSpeed;
  }
  
  // Code for updating the screen
  updateScreen();
  
  // Code for checking collision
  if (checkCollision()) {
    resetLevel();
  }
  
  // Code for level completion
  if (checkLevelComplete()) {
    level++;
    loadLevel(level);
  }
}

void loadLevel(int level) {
  keyX = 10;
  keyY = 30;
  doorX = 140;
  doorY = 90;
  switch (level) {
    case 1:
      // Generate level 1
      tft.fillRect(0, 100, 160, 20, ILI9163_WHITE);
      tft.fillRect(20, 80, 20, 20, ILI9163_WHITE);
      tft.fillRect(60, 60, 20, 20, ILI9163_WHITE);
      tft.fillRect(100, 40, 20, 20, ILI9163_WHITE);
      tft.fillRect(140, 20, 20, 20, ILI9163_WHITE);
      tft.fillRect(80, 90, 20, 10, ILI9163_WHITE);
      tft.fillRect(120, 70, 20, 10, ILI9163_WHITE);
      break;
    case 2:
      // Generate level 2
      tft.fillRect(0, 100, 160, 20, ILI9163_WHITE);
      tft.fillRect(20, 80, 20, 20, ILI9163_WHITE);
      tft.fillRect(60, 60, 20, 20, ILI9163_WHITE);
      tft.fillRect(100, 40, 20, 20, ILI9163_WHITE);
      tft.fillRect(140, 20, 20, 20, ILI9163_WHITE);
      tft.fillRect(40, 90, 20, 10, ILI9163_WHITE);
      tft.fillRect(80, 70, 20, 10, ILI9163_WHITE);
      tft.fillRect(120, 50, 20, 10, ILI9163_WHITE);
      break;
    case 3:
      // Generate level 3
      tft.fillRect(0, 100, 160, 20, ILI9163_WHITE);
      tft.fillRect(20, 80, 20, 20, ILI9163_WHITE);
      tft.fillRect(60, 60, 20, 20, ILI9163_WHITE);
      tft.fillRect(100, 40, 20, 20, ILI9163_WHITE);
      tft.fillRect(140, 20, 20, 20, ILI9163_WHITE);
      tft.fillRect(40, 90, 20, 10, ILI9163_WHITE);
      tft.fillRect(80, 70, 20, 10, ILI9163_WHITE);
      tft.fillRect(120, 50, 20, 10, ILI9163_WHITE);
      tft.fillRect(100, 80, 20, 20, ILI9163_WHITE);
      break;
    // Code for additional levels
    default:
      // Load default level
      tft.fillRect(0, 100, 160, 20, ILI9163_WHITE);
      tft.fillRect(20, 80, 20, 20, ILI9163_WHITE);
      tft.fillRect(60, 60, 20, 20, ILI9163_WHITE);
      tft.fillRect(100, 40, 20, 20, ILI9163_WHITE);
      tft.fillRect(140, 20, 20, 20, ILI9163_WHITE);
      break;
  }
}

bool checkLevelComplete() {
  if (playerX < keyX + KEY_SIZE && playerX + PLAYER_SIZE > keyX && playerY < keyY + KEY_SIZE && playerY + PLAYER_SIZE > keyY) {
    // Player has collected the key
    keyCollected = true;
  }

  if (playerX < doorX + DOOR_WIDTH && playerX + PLAYER_SIZE > doorX && playerY < doorY + DOOR_HEIGHT && playerY + PLAYER_SIZE > doorY && keyCollected) {
    // Player has reached the door with the key
    keyCollected = false;
    return true;
  }

  return false;
}

void updateScreen() {
  tft.fillScreen(ILI9163_BLACK);
  tft.fillRect(0, 120, 160, 40, ILI9163_WHITE);
  tft.fillRect(keyX, keyY, 20, 20, ILI9163_YELLOW);
  tft.fillRect(doorX, doorY, 20, 20, ILI9163_GREEN);
  tft.fillRect(playerX, playerY, playerWidth, playerHeight, ILI9163_WHITE);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9163_WHITE);
  tft.setTextSize(2);
  tft.print("Level ");
  tft.print(level);
}

bool checkCollision() {
  if (playerX < 0 || playerX + playerWidth > 160 || playerY < 0 || playerY + playerHeight > 120) {
    // Player has collided with screen boundary
    return true;
  }
  if (playerX + playerWidth > keyX && playerX < keyX + 20 && playerY + playerHeight > keyY && playerY < keyY + 20) {
    // Player has collected the key
    tft.fillRect(keyX, keyY, 20, 20, ILI9163_BLACK);
  }
}