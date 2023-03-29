#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

//Color 
#define TFT_BLACK   0x0000
#define TFT_BLUE    0x001F
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW  0xFFE0  
#define TFT_WHITE   0xFFFF


// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

// Player dimensions
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8

// Gate dimensions
#define GATE_WIDTH 16
#define GATE_HEIGHT 16

// Platform dimensions
#define PLATFORM_WIDTH 80
#define PLATFORM_HEIGHT 16

// Player position
int player_x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
int player_y = SCREEN_HEIGHT - PLAYER_HEIGHT - PLATFORM_HEIGHT;

// Gate position
int gate_x = SCREEN_WIDTH - GATE_WIDTH - 4;
int gate_y = SCREEN_HEIGHT - GATE_HEIGHT - PLATFORM_HEIGHT;

// Platform position
int platform_x = SCREEN_WIDTH / 2 - PLATFORM_WIDTH / 2;
int platform_y = SCREEN_HEIGHT - PLATFORM_HEIGHT;

// Variables to control player movement
unsigned long start_time = 0;
bool move_player = false;
bool stop_player = false;

// Setup TFT display
TFT_ILI9163C tft = TFT_ILI9163C(10, 8, 9);

void setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  draw_platform();
  draw_gate();
  draw_player();
}

void draw_platform() {
  tft.fillRect(platform_x, platform_y, PLATFORM_WIDTH, PLATFORM_HEIGHT, TFT_WHITE);
}

void draw_player() {
  tft.fillRect(player_x, player_y, PLAYER_WIDTH, PLAYER_HEIGHT, TFT_RED);
}

void draw_gate() {
  tft.fillRect(gate_x, gate_y, GATE_WIDTH, GATE_HEIGHT, TFT_GREEN);
}

void clear_player() {
  tft.fillRect(player_x, player_y, PLAYER_WIDTH, PLAYER_HEIGHT, TFT_BLACK);
}

void clear_gate() {
  tft.fillRect(gate_x, gate_y, GATE_WIDTH, GATE_HEIGHT, TFT_BLACK);
}

void loop() {
  if (!move_player && !stop_player) {
    // Player needs to stand still for 6 seconds
    if (millis() - start_time >= 6000) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 10);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.print("MOVE NOW");
      move_player = true;
      start_time = millis();
    }
  }
  else if (move_player && !stop_player) {
    // Player needs to move for 2 seconds
    if (millis() - start_time >= 2000) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 10);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.print("STOP");
      stop_player = true;
      start_time = millis();
    }
    else {
      clear_player();
      player_x += 1;
      draw_player();
    }
  }
  else if (stop_player) {
    // Player needs to stop for 2 seconds
    if (millis() - start_time >= 2000) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 10);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.print("MOVE NOW");
      move_player = true;
      stop_player = false;
      start_time = millis();
    }
  }
}
