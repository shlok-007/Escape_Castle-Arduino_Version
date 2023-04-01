#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include <Wire.h>
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

int ADXL345 = 0x53;

const int N = 20,
          obstacle_radius=4,
          x_offset=5,
          y_offset=4,
          fps=50;

float xAccel = 0; // Accelerometer reading for x-axis
float yAccel = 0; // Accelerometer reading for y-axis
float zAccel = 0; // Accelerometer reading for z-axis
int accelThreshold = 50; // Threshold for detecting motion
int jumpYThreshold = 100; // Threshold for detecting jump
int jumpZThreshold = 35; // Threshold for detecting jump
// int jumpDuration = 500; // Duration of jump (in milliseconds)
float gravity = 1; // Acceleration due to gravity
float jumpVelocity = 2.5; // Initial jump velocity
float yVelocity = 0; // Velocity on y-axis
float xVelocity = 1; // Velocity on x-axis

// Define character variables
// int characterVelocityX = 0; // Character velocity on x-axis
int characterVelocityY = 0; // Character velocity on y-axis

bool firstRender = true;

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
  (*switches)[3],
  // door_pos[2],
  *door_pos;
  // key_pos[2], 

  float 
  *key_pos,
  // plr_pos[2],
  *plr_pos,
  // init_plr_pos[2];
  *init_plr_pos;
  
  bool 
  key_visible, 
  door_visible,
  obstacle_visible, 
  key_picked;

  String txt;

}frame;

frame *initFrame();

frame *prevFrame = initFrame();

TFT_ILI9163C display = TFT_ILI9163C(10, 8, 9);

float pi = 3.1415926;

void setup() {
  display.begin();
  display.setTextSize(1);
  display.setCursor( 5 , 20 );
  display.println("    Calibrating");
  display.println("  Accelerometer...");

  // Calibrate accelerometer
  float xsum=0, ysum = 0, zsum = 0;
  int calib_loops = 1000;
  Wire.begin();
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // Bit D3 High for measuring enable (8dec -> 0000 1000 binary)
  Wire.endTransmission();
  delay(10);

  
  int i;
  i=calib_loops;
  while(i--){  
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    xsum += ( Wire.read() | Wire.read() << 8); // X-axis value
    // X_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
    ysum += ( Wire.read() | Wire.read() << 8); // Y-axis value
    // Y_out = Y_out / 256;
    zsum += ( Wire.read() | Wire.read() << 8); // Z-axis value
    // Z_out = Z_out / 256;
  }


  int xcalib = -(xsum/(calib_loops*4)), ycalib = -(ysum/(calib_loops*4)), zcalib = -((zsum/calib_loops)-256)/4 ;
  //Off-set Calibration
  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);
  Wire.write(xcalib);
  Wire.endTransmission();
  delay(10);
  //Y-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F);
  Wire.write(ycalib);
  Wire.endTransmission();
  delay(10);
  //Z-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20);
  Wire.write(zcalib);
  Wire.endTransmission();
  delay(10);

  display.clearScreen();
  display.setCursor( 5 , 10 );
  display.println("    Calibration");
  display.println("    Complete :D");
  // display.print(xcalib);
  // display.print(" ");
  // display.print(ycalib);
  // display.print(" ");
  // display.print(zcalib);
  delay(1000);
  display.clearScreen();

  String title = "Escape Castle", subtitle = " Can you escape the   castle ??";
  display.setTextSize(3);
  display.setCursor( 5 , 20 );
  display.setTextColor(BROWN);
  display.println(title);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(subtitle);
  display.fillRect(2,  1, 128,  3, CYAN);
  display.fillRect(2,  4,   3, 123, CYAN);
  display.fillRect(2, 125,128,  3, CYAN);
  display.fillRect(125, 4,  5, 123, CYAN);
  delay(2000);
  display.clearScreen();
  // Serial.begin(9600);  
}

void loop(){
  level2();
  // testLevel1();
  // while (true);

}

bool checkCeilCollision(frame *frame){
  if(frame->plr_pos[1] <= 0 && frame->plr_pos[1] - yVelocity/fps <0){
    frame->plr_pos[1] = 0;
    return true;}
  return false;
}

bool checkFloorCollision(frame *frame){
  if(frame->plr_pos[1] <= 10 && frame->plr_pos[1] - yVelocity/fps +1 >= 11){
    frame->plr_pos[1]=10;
    return true;}
  return false;
}

bool checkJumpCollision(frame *frame){
  float plrLx, plrRx, plrY;
  int pltLx, pltRx, pltY;
  for(int i=0; i<frame->n_platforms; i++){
    plrLx = frame->plr_pos[0], 
    plrRx = frame->plr_pos[0] + 1, 
    plrY = frame->plr_pos[1];
    pltLx = frame->platforms[i][0], 
    pltRx = frame->platforms[i][0] + frame->platforms[i][2], 
    pltY = frame->platforms[i][1] + frame->platforms[i][3];

    if( plrRx > pltLx && plrLx < pltRx && plrY>= pltY && plrY-yVelocity/fps <= pltY ){ 
      frame->plr_pos[1] = pltY;
      return true;}
  }
  return false;
}

bool checkFallCollision(frame *frame){
  float plrLx, plrRx, plrY;
  int pltLx, pltRx, pltY;
  for(int i=0; i<frame->n_platforms; i++){
    plrLx = frame->plr_pos[0], 
    plrRx = frame->plr_pos[0] + 1, 
    plrY = frame->plr_pos[1]+1;
    pltLx = frame->platforms[i][0], 
    pltRx = frame->platforms[i][0] + frame->platforms[i][2] , 
    pltY = frame->platforms[i][1];
    if( plrRx > pltLx && plrLx < pltRx && plrY <= pltY && plrY-yVelocity/fps >= pltY ) {
      frame->plr_pos[1]=pltY-1;  
      return true;}
  }
  return false;
}

bool checkLeftWallCollision(frame *frame){
  if(frame->plr_pos[0] <= 0) return true;
  return false;
}

bool checkRightWallCollision(frame *frame){
  if(frame->plr_pos[0] >= 10) return true;
  return false;
}

bool checkLeftCollision(frame *frame){
  float plrTy, plrBy, plrX;
  int pltTy, pltBy, pltX;
  for(int i=0; i<frame->n_platforms; i++){
    plrTy = frame->plr_pos[1], 
    plrBy = frame->plr_pos[1] + 1, 
    plrX = frame->plr_pos[0] + 1;
    pltTy = frame->platforms[i][1], 
    pltBy = frame->platforms[i][1] + frame->platforms[i][3], 
    pltX = frame->platforms[i][0];
    if( plrBy > pltTy && plrTy < pltBy && plrX <= pltX && plrX + xVelocity/fps > pltX) return true;
  }
  return false;
}

bool checkRightCollision(frame *frame){
  float plrTy, plrBy, plrX;
  int pltTy, pltBy, pltX;
  for(int i=0; i<frame->n_platforms; i++){
    plrTy = frame->plr_pos[1], 
    plrBy = frame->plr_pos[1] + 1, 
    plrX = frame->plr_pos[0];
    pltTy = frame->platforms[i][1], 
    pltBy = frame->platforms[i][1] + frame->platforms[i][3], 
    pltX = frame->platforms[i][0] + frame->platforms[i][2];
    if( plrBy > pltTy && plrTy < pltBy && plrX >= pltX && plrX - xVelocity/fps < pltX ) return true;
  }
  return false;
}

void applyPhysics(frame *frame){
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  xAccel = ( Wire.read() | Wire.read() << 8); // X-axis value
  yAccel = ( Wire.read() | Wire.read() << 8); // Y-axis value
  zAccel = ( Wire.read() | Wire.read() << 8); // Z-axis value
  zAccel = 256 - zAccel;

  // frame->txt = checkRightCollision(frame);
  if(xAccel > accelThreshold){
    if(!checkLeftCollision(frame) && !checkRightWallCollision(frame)) frame->plr_pos[0] += xVelocity/fps;
    else if(checkRightWallCollision(frame))    frame->plr_pos[0] = 0;
  }
  else if(xAccel < -accelThreshold){
    if(!checkRightCollision(frame) && !checkLeftWallCollision(frame))  frame->plr_pos[0] -= xVelocity/fps;
    else if(checkLeftWallCollision(frame)) frame->plr_pos[0] = 10;
  }

  //
  // frame->txt[0] = (char)frame->plr_pos[1];
  


  if(yAccel < -jumpYThreshold && (checkFallCollision(frame) || frame->plr_pos[1]==10)){
    // Serial.println("yes");
    yVelocity = jumpVelocity;}
  else{
    if(!checkFallCollision(frame) && !checkFloorCollision(frame))  yVelocity -= gravity/fps;
    else  yVelocity = 0;}
  // debug((String)yVelocity);
  if(yVelocity >= 0){
    if(!checkCeilCollision(frame) && !checkJumpCollision(frame)){
      frame->plr_pos[1] -= yVelocity/fps;}
    else{
      yVelocity = -yVelocity/2;
      // if(!checkFallCollision(frame) && !checkFloorCollision(frame)) frame->plr_pos[1] -= yVelocity;
      // else  yVelocity = 0;
      }
  }
  else if(yVelocity < 0){
    if(!checkFallCollision(frame) && !checkFloorCollision(frame)){
      frame->plr_pos[1] -= yVelocity/fps;
      }
    else  yVelocity = 0;

    // if(frame->plr_pos[0]>10)  frame->plr_pos[0]=10;
    // if(frame->plr_pos[0]<0)  frame->plr_pos[0]=0;
    if(frame->plr_pos[1]>10)  frame->plr_pos[1]=10;
    if(frame->plr_pos[1]<0)  frame->plr_pos[1]=0;
  }

}

bool keyFound();

bool reachedDoor(){
  display.clearScreen();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("   Stage");
  display.setTextColor(GREEN);
  display.println("  Cleared");
  while(1);
}

bool isDead();

void updateScreen();

void debug(String s){
  display.fillRect(55, 15, 6*6, 15, BLACK);
  display.setCursor(55, 15);
  // display.clearScreen();
  display.print(s);
}





void render( frame frame ){

  if(firstRender){

    firstRender=false;
    display.clearScreen();
    //drawing borders
    display.fillRect(2,  1, 128,  1, CYAN);
    display.fillRect(2,  2,   1, 125, CYAN);
    display.fillRect(2, 127,128,  1, CYAN);
    display.fillRect(127, 2,  1, 125, CYAN);

    //rendering platforms
    int i;
    for(i=0;i<frame.n_platforms;i++){
      display.fillRect(x_offset + frame.platforms[i][0]*11,
                      y_offset + frame.platforms[i][1]*11,
                      frame.platforms[i][2]*11,
                      frame.platforms[i][3]*11,
                      random(0x0010,0xFFFF));
    }
    //rendering key
    if(frame.key_visible){
      display.fillCircle(
        x_offset + frame.key_pos[0]*11,
        y_offset + frame.key_pos[1]*11,
        2,
        YELLOW);
    }
    //rendering door
    if(frame.door_visible){
      display.fillRect(
        x_offset + frame.door_pos[0]*11,
        y_offset + frame.door_pos[1]*11,
        6,11,DARKBROWN);
    }
    //rendering obstacles
    if(frame.obstacle_visible){
        for(i=0;i<frame.n_obstacles;i++){
        display.drawCircle(
          x_offset + frame.obstacles[i][0]*11,
          y_offset + frame.obstacles[i][1]*11,
          obstacle_radius,
          MAGENTA);
      }
    }
    //rendering boxes
    for(i=0;i<frame.n_boxes;i++){
      display.fillRect(
        x_offset + frame.boxes[i][0]*11,
        y_offset + frame.boxes[i][1]*11,
        9,
        5,
        BROWN);
    }
    //rendering switches
    for(i=0;i<frame.n_switches;i++){
      if(frame.switches[i][2]){
        display.fillRect(
        x_offset+ 2 + frame.switches[i][0]*11,
        y_offset+ 5 + frame.switches[i][1]*11,
        2,
        5,
        WHITE);
      }
      else{
        display.fillRect(
        x_offset+ 2 + frame.switches[i][0]*11,
        y_offset+ 5 + frame.switches[i][1]*11,
        2,
        5,
        BLUE);
      }
    }
    //rendering player
    display.fillCircle(
        x_offset+6 + frame.plr_pos[0]*11,
        y_offset+5 + frame.plr_pos[1]*11,
        5,
        RED);
    if(frame.key_picked){
      display.fillCircle(
        x_offset+6 + frame.plr_pos[0]*11,
        y_offset+6 + frame.plr_pos[1]*11,
        2,
        YELLOW);
    }
    //rendering text
    int offset = frame.txt.length()/2 ;
    display.setCursor( 63 - offset*6 , 15 );
    // display.setCursor( 56 , 70 );
    display.println(frame.txt);
  }



//for re-rendering
  else{
    // if(frame.key_visible!=prevFrame->key_visible){
      if(!frame.key_visible){
        display.fillCircle(
        x_offset + frame.key_pos[0]*11,
        y_offset + frame.key_pos[1]*11,
        2,
        BLACK);
      }
      else{
        display.fillCircle(
        x_offset + frame.key_pos[0]*11,
        y_offset + frame.key_pos[1]*11,
        2,
        YELLOW);
      }
    

    // if(frame.door_visible!=prevFrame->door_visible){
      if(frame.door_visible){
        display.fillRect(
        x_offset + frame.door_pos[0]*11,
        y_offset + frame.door_pos[1]*11,
        6,11,DARKBROWN);
      }
      else{
        display.fillRect(
        x_offset + frame.door_pos[0]*11,
        y_offset + frame.door_pos[1]*11,
        6,11,BLACK);
      }
    

    if(frame.obstacle_visible!=prevFrame->obstacle_visible){
      if(frame.obstacle_visible){
        int i;
        for(i=0;i<frame.n_obstacles;i++){
          display.drawCircle(
            x_offset + frame.obstacles[i][0]*11,
            y_offset + frame.obstacles[i][1]*11,
            obstacle_radius,
            MAGENTA);
        }
      }
      else{
        int i;
        for(i=0;i<frame.n_obstacles;i++){
          display.drawCircle(
            x_offset + frame.obstacles[i][0]*11,
            y_offset + frame.obstacles[i][1]*11,
            obstacle_radius,
            BLACK);
        }
      }
    }

    for(int i=0;i<frame.n_switches;i++){
      // if(frame.switches[i][2]!=prevFrame->switches[i][2]){
        if(frame.switches[i][2]){
          display.fillRect(
          x_offset+ 2 + frame.switches[i][0]*11,
          y_offset+ 5 + frame.switches[i][1]*11,
          2,
          5,
          WHITE);
        }
        else{
          display.fillRect(
          x_offset+ 2 + frame.switches[i][0]*11,
          y_offset+ 5 + frame.switches[i][1]*11,
          2,
          5,
          BLUE);
        }
    }

    // if( frame.txt != prevFrame->txt ){
    //   int offset = frame.txt.length()/2 ;
    //   display.fillRect( 63 - offset*6 , 15 , 6*prevFrame->txt.length() , 10 , BLACK );
    //   display.setCursor( 63 - offset*6 , 15 );
    //   display.print(frame.txt);
    // }

    if(frame.plr_pos[0]!=prevFrame->plr_pos[0] || frame.plr_pos[1]!=prevFrame->plr_pos[1]){
      display.fillCircle(
        x_offset+6 + prevFrame->plr_pos[0]*11,
        y_offset+5 + prevFrame->plr_pos[1]*11,
        5,
        BLACK);
      display.fillCircle(
        x_offset+6 + frame.plr_pos[0]*11,
        y_offset+5 + frame.plr_pos[1]*11,
        5,
        RED);
      if(frame.key_picked){
        display.fillCircle(
          x_offset+6 + prevFrame->plr_pos[0]*11,
          y_offset+5 + prevFrame->plr_pos[1]*11,
          2,
          BLACK);
        display.fillCircle(
          x_offset+6 + frame.plr_pos[0]*11,
          y_offset+5 + frame.plr_pos[1]*11,
          2,
          YELLOW);
      }
    }
  }

}







frame *initFrame(){
  frame *frame_ptr = (frame*)malloc(sizeof(frame));
  frame_ptr->n_platforms = 0;
  frame_ptr->n_obstacles = 0;
  frame_ptr->n_boxes = 0;
  frame_ptr->n_switches = 0;
  frame_ptr->key_visible = true;
  frame_ptr->door_visible = false;
  frame_ptr->key_picked = false;
  frame_ptr->obstacle_visible = true;
  // frame_ptr->txt = (char *)malloc(50*sizeof(char));
  frame_ptr->txt = "";
  
  frame_ptr->platforms = (int (*)[4])malloc(N*4*sizeof(int));
  frame_ptr->obstacles = (int (*)[2])malloc(N*2*sizeof(int));
  frame_ptr->boxes = (int (*)[2])malloc(N*2*sizeof(int));
  frame_ptr->switches = (int (*)[3])malloc(N*3*sizeof(int));
  frame_ptr->key_pos = (float *)malloc(2*sizeof(float));
  frame_ptr->door_pos = (int *)malloc(2*sizeof(int));
  frame_ptr->plr_pos = (float *)malloc(2*sizeof(float));
  frame_ptr->init_plr_pos = (float *)malloc(2*sizeof(float));
  return frame_ptr;
}





void copyFrame(frame *frame_ptr){
  prevFrame->key_visible = frame_ptr->key_visible;
  prevFrame->key_picked = frame_ptr->key_picked;
  prevFrame->key_pos[0] = frame_ptr->key_pos[0]; prevFrame->key_pos[1] = frame_ptr->key_pos[1];

  prevFrame->door_visible = frame_ptr->door_visible;
  prevFrame->door_pos[0] = frame_ptr->door_pos[0]; prevFrame->door_pos[1] = frame_ptr->door_pos[1];

  prevFrame->obstacle_visible = frame_ptr->obstacle_visible;
  prevFrame->n_obstacles = frame_ptr->n_obstacles;
  for(int i=0;i<frame_ptr->n_obstacles;i++){
    prevFrame->obstacles[i][0] = frame_ptr->obstacles[i][0];
    prevFrame->obstacles[i][1] = frame_ptr->obstacles[i][1];
  }

  prevFrame->n_switches = frame_ptr->n_switches;
  for(int i=0;i<frame_ptr->n_switches;i++){
    prevFrame->switches[i][0] = frame_ptr->switches[i][0];
    prevFrame->switches[i][1] = frame_ptr->switches[i][1];
    prevFrame->switches[i][2] = frame_ptr->switches[i][2];
  }

  prevFrame->plr_pos[0] = frame_ptr->plr_pos[0]; prevFrame->plr_pos[1] = frame_ptr->plr_pos[1];

  prevFrame->init_plr_pos[0] = frame_ptr->init_plr_pos[0];  prevFrame->init_plr_pos[1] = frame_ptr->init_plr_pos[1];

  prevFrame->txt = frame_ptr->txt;
}





void testLevel1(){
  frame *frame_ptr  = initFrame();
  firstRender =true;
  frame_ptr->n_platforms = 7;
  frame_ptr->platforms[5][0]= 3;frame_ptr->platforms[5][1]= 10;frame_ptr->platforms[5][2]= 4;frame_ptr->platforms[5][3]= 1;
  frame_ptr->platforms[4][0]= 8;frame_ptr->platforms[4][1]= 2;frame_ptr->platforms[4][2]= 3;frame_ptr->platforms[4][3]= 1;
  frame_ptr->platforms[0][0]= 1;frame_ptr->platforms[0][1]= 2;frame_ptr->platforms[0][2]= 2;frame_ptr->platforms[0][3]= 1;
  frame_ptr->platforms[1][0]= 8;frame_ptr->platforms[1][1]= 2;frame_ptr->platforms[1][2]= 2;frame_ptr->platforms[1][3]= 1;
  frame_ptr->platforms[2][0]= 1;frame_ptr->platforms[2][1]= 7;frame_ptr->platforms[2][2]= 2;frame_ptr->platforms[2][3]= 1;
  frame_ptr->platforms[3][0]= 8;frame_ptr->platforms[3][1]= 7;frame_ptr->platforms[3][2]= 2;frame_ptr->platforms[3][3]= 1;
  frame_ptr->platforms[6][0]= 4;frame_ptr->platforms[6][1]= 5;frame_ptr->platforms[6][2]= 2;frame_ptr->platforms[6][3]= 1;
  frame_ptr->door_visible = true;
  frame_ptr->key_pos[0] = 5; frame_ptr->key_pos[1] = 3;

  frame_ptr->door_pos[0] = 10;frame_ptr->door_pos[1] = 10;
  frame_ptr->plr_pos[0] = 3;frame_ptr->plr_pos[1] = 0;
  frame_ptr->init_plr_pos[0] = 3;frame_ptr->init_plr_pos[1] = 0;
  frame_ptr->key_visible=true;
  
  // frame_ptr->txt = "Hello";
  // debug("hi2");
  render(*frame_ptr);
   while(1)
  {
    copyFrame(frame_ptr);
    applyPhysics(frame_ptr);
    if(frame_ptr->door_visible==true && frame_ptr->plr_pos[0]==10 && frame_ptr->plr_pos[1]==10 && frame_ptr->key_picked)
    {
      // frame_ptr->txt = "Hii1";
      reachedDoor();
    }
    render(*frame_ptr);
    // delay(1000/fps);
  }
}






void level2(){
  frame *frame_ptr  = initFrame();

  firstRender = true;

  frame_ptr->n_platforms = 4;
  // frame_ptr->platforms[0][0]= 0;frame_ptr->platforms[0][1]= 10;frame_ptr->platforms[0][2]= 11;frame_ptr->platforms[0][3]= 1;
  frame_ptr->platforms[0][0]= 0;frame_ptr->platforms[0][1]= 7;frame_ptr->platforms[0][2]= 2;frame_ptr->platforms[0][3]= 1;
  frame_ptr->platforms[1][0]= 4;frame_ptr->platforms[1][1]= 4;frame_ptr->platforms[1][2]= 2;frame_ptr->platforms[1][3]= 1;
  frame_ptr->platforms[2][0]= 4;frame_ptr->platforms[2][1]= 8;frame_ptr->platforms[2][2]= 2;frame_ptr->platforms[2][3]= 1;
  frame_ptr->platforms[3][0]= 9;frame_ptr->platforms[3][1]= 7;frame_ptr->platforms[3][2]= 2;frame_ptr->platforms[3][3]= 1;
  
  frame_ptr->n_obstacles = 0;
  frame_ptr->n_boxes = 0;
  
  frame_ptr->n_switches = 4;
  frame_ptr->switches[0][0] = 1;frame_ptr->switches[0][1] =6; frame_ptr->switches[0][2] =0; //above platform 1
  frame_ptr->switches[1][0] = 5;frame_ptr->switches[1][1] =3; frame_ptr->switches[1][2] =0;//above platform 2
  frame_ptr->switches[2][0] = 5;frame_ptr->switches[2][1] =7; frame_ptr->switches[2][2] =0;//above platform 3
  frame_ptr->switches[3][0] = 10;frame_ptr->switches[3][1] =6; frame_ptr->switches[3][2] =0;//above platform 4
  
  frame_ptr->key_visible = false;
  frame_ptr->key_pos[0] = 4;frame_ptr->key_pos[1] = 3;
  
  frame_ptr->door_visible = true;
  frame_ptr->door_pos[0] = 10;frame_ptr->door_pos[1] = 10;
  
  frame_ptr->plr_pos[0] = 3;frame_ptr->plr_pos[1] = 0;
  frame_ptr->init_plr_pos[0] = 3;frame_ptr->init_plr_pos[1] = 0;
  // frame_ptr->txt = "Level 2: not all are correct";
  frame_ptr->plr_pos[0]=1;
  frame_ptr->plr_pos[1]=2;
  display.clearScreen();
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.setTextColor(WHITE);
  display.println("   Level");
  display.setTextColor(GREEN);
  display.println("     1");
  delay(2000);
  render(*frame_ptr);  
  // switch state assumed to be 0 and 1
  while(1){
    copyFrame(frame_ptr);
    applyPhysics(frame_ptr);
    // render()
    // frame_ptr->plr_pos[0]++;
    // frame_ptr->plr_pos[1]++;
    // record_pos takes the position after the click of the button
    int record_pos_x=frame_ptr->plr_pos[0], record_pos_y=frame_ptr->plr_pos[1];
    if(frame_ptr->key_visible==0){
      for(int i=0;i<4;i++){ // updates the switch state
        if(record_pos_x==frame_ptr->switches[i][0] && record_pos_y==frame_ptr->switches[i][1]){
          frame_ptr->switches[i][2]=1-frame_ptr->switches[i][2];
        }
      }
    }
    if(frame_ptr->switches[2][2] && !frame_ptr->key_picked){
      frame_ptr->key_visible = true;
      // frame_ptr->plr_pos[0]=4;
      // frame_ptr->plr_pos[1]=4;
    }

    if(frame_ptr->key_visible==true){
      int record_keypos_x=frame_ptr->plr_pos[0], record_keypos_y=frame_ptr->plr_pos[1];
      if(record_keypos_x==frame_ptr->key_pos[0] && record_keypos_y==frame_ptr->key_pos[1]){
        frame_ptr->key_picked=true;
        frame_ptr->key_visible = false;
      }
    }
    
    if(frame_ptr->key_picked==true){
      int record_doorPos_x=frame_ptr->plr_pos[0], record_doorPos_y=frame_ptr->plr_pos[1];
      if(record_doorPos_x==frame_ptr->door_pos[0] && record_doorPos_y==frame_ptr->door_pos[1]){
      reachedDoor();
      break;
      }   
    }
    render(*frame_ptr);
  }
}

