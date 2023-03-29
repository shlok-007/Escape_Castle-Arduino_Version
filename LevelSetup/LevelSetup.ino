#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include<string.h>
#include<stdlib.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF
#define BROWN   0xFB00
#define DARKBROWN 0x8041

const int N = 20,
          obstacle_radius=4;

typedef struct Frame{

  int 
  n_platforms, 
  // platforms[N][4], 
  (*platforms)[4], 
  n_obstacles,
  // obstacles[N][2],
  (*obstacles)[2],
  n_boxes,
  // boxes[N][2],
  (*boxes)[2],
  n_switches,
  // switches[N][2],
  (*switches)[2],
  // key_pos[2], 
  *key_pos,
  // door_pos[2], 
  *door_pos,
  // plr_pos[2],
  *plr_pos,
  // init_plr_pos[2];
  *init_plr_pos;
  
  bool 
  key_visible, 
  door_visible, 
  key_picked;

  String txt;

}frame;

TFT_ILI9163C display = TFT_ILI9163C(10, 8, 9);

float pi = 3.1415926;

void setup() {
  display.begin();
  String title = "Tricky Castle", subtitle = " Can you escape the   castle ??";
  display.setCursor( 5 , 20 );
  display.setTextSize(3);
  display.setTextColor(BROWN);
  display.println(title);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(subtitle);
  display.fillRect(2,  1, 128,  3, CYAN);
  display.fillRect(2,  4,   3, 123, CYAN);
  display.fillRect(2, 125,128,  3, CYAN);
  display.fillRect(125, 4,  3, 123, CYAN);
  delay(500);
  display.clearScreen();

}

void loop(){
  level1();
  while (true);

}


void applyPhysics();

void render( frame frame ){

  //rendering platforms
  display.clearScreen();
  //drawing borders
  display.fillRect(2,  1, 128,  3, RED);
  display.fillRect(2,  4,   3, 123, RED);
  display.fillRect(2, 127,128,  3, RED);
  display.fillRect(125, 4,  3, 123, RED);
  // display.drawFastHLine(2, 1, 128, RED);
  // display.drawFastHLine(2, 127, 128, RED);
  // display.drawFastVLine(2, 1, 128, RED);
  // display.drawFastVLine(127, 1, 128, RED);
  display.drawPixel(2, 1, WHITE);
  display.drawPixel(2, 127, WHITE);
  display.drawPixel(128, 1, WHITE);
  display.drawPixel(2, 1, WHITE);
  int i;
  for(i=0;i<frame.n_platforms;i++){
    display.fillRect(9 + frame.platforms[i][0]*11,
                    9 + frame.platforms[i][1]*11,
                    frame.platforms[i][2]*11,
                    frame.platforms[i][3]*11,
                    random(0x0010,0xFFFF));
  }
  if(frame.key_visible){
    display.fillCircle(9 + frame.key_pos[0]*11,
     9 + frame.key_pos[1]*11,
     2,
     YELLOW);
  }
  if(frame.door_visible){
    display.fillRect(9 + frame.door_pos[0]*11,
     9 + frame.door_pos[1]*11,
     6,11,DARKBROWN);
  }
  for(i=0;i<frame.n_obstacles;i++){
    display.drawCircle(9 + frame.obstacles[i][0]*11,
     9 + frame.obstacles[i][1]*11,
     obstacle_radius,
     MAGENTA);
  }
  for(i=0;i<frame.n_boxes;i++){
    display.fillRect(9 + frame.boxes[i][0]*11,
     9 + frame.boxes[i][1]*11,
     9,
     5,
     BROWN);
  }
  for(i=0;i<frame.n_switches;i++){
    display.fillRect(9+ 2 + frame.switches[i][0]*11,
     9+ 5 + frame.switches[i][1]*11,
     2,
     5,
     WHITE);
  }
  display.fillCircle(9+6 + frame.plr_pos[0]*11,
     9+6 + frame.plr_pos[1]*11,
     5,
     RED);
  if(frame.key_picked){
    display.fillCircle(9+6 + frame.plr_pos[0]*11,
     9+6 + frame.plr_pos[1]*11,
     2,
     YELLOW);
  }
  int offset = frame.txt.length()/2 ;
  display.setCursor( 63 - offset*6 , 15 );
  display.print(frame.txt);

}

bool keyFound();

bool reachedDoor();

bool isDead();

frame *initFrame(){
  frame *frame_ptr = (frame*)malloc(sizeof(frame));
  frame_ptr->n_platforms = 0;
  frame_ptr->n_obstacles = 0;
  frame_ptr->n_boxes = 0;
  frame_ptr->n_switches = 0;
  frame_ptr->key_visible = false;
  frame_ptr->door_visible = false;
  frame_ptr->key_picked = false;
  frame_ptr->txt = "";
  frame_ptr->platforms = (int (*)[4])malloc(N*4*sizeof(int));
  frame_ptr->obstacles = (int (*)[2])malloc(N*2*sizeof(int));
  frame_ptr->boxes = (int (*)[2])malloc(N*2*sizeof(int));
  frame_ptr->switches = (int (*)[2])malloc(N*2*sizeof(int));
  frame_ptr->key_pos = (int *)malloc(2*sizeof(int));
  frame_ptr->door_pos = (int *)malloc(2*sizeof(int));
  frame_ptr->plr_pos = (int *)malloc(2*sizeof(int));
  frame_ptr->init_plr_pos = (int *)malloc(2*sizeof(int));
  return frame_ptr;
}

void level1(){
  frame *frame_ptr  = initFrame();
  frame_ptr->n_platforms = 2;
  int arr1[] = {3,4,4,1};
  frame_ptr->platforms[0][0]= 3;frame_ptr->platforms[0][1]= 10;frame_ptr->platforms[0][2]= 4;frame_ptr->platforms[0][3]= 1;
  frame_ptr->platforms[1][0]= 2;frame_ptr->platforms[1][1]= 1;frame_ptr->platforms[1][2]= 3;frame_ptr->platforms[1][3]= 1;
  frame_ptr->n_obstacles = 1;
  frame_ptr->obstacles[0][0] = 3;frame_ptr->obstacles[0][1]=5;
  frame_ptr->n_boxes = 1;
  frame_ptr->boxes[0][0] = 5;frame_ptr->boxes[0][1] = 3;
  frame_ptr->n_switches = 1;
  frame_ptr->switches[0][0] = 9;frame_ptr->switches[0][1] =9;
  frame_ptr->key_visible = true;
  frame_ptr->key_pos[0] = 3;frame_ptr->key_pos[1] = 3;
  frame_ptr->door_visible = true;
  frame_ptr->door_pos[0] = 9;frame_ptr->door_pos[1] = 9;
  frame_ptr->plr_pos[0] = 3;frame_ptr->plr_pos[1] = 0;
  frame_ptr->init_plr_pos = 3;frame_ptr->init_plr_pos = 0;
  frame_ptr->txt = "Level 1";
  render(*frame_ptr);
  
}



