#include <Wire.h>  // Wire library - used for I2C communication

int ADXL345 = 0x53; // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;  // Outputs
float roll,pitch,rollF,pitchF=0;
float xsum=0, ysum=0, zsum=0;
int calib_loops=5000;

void setup() {
  Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor
 
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
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
  Serial.print("XC= ");
  Serial.print(xcalib);
  Serial.print("   YC= ");
  Serial.print(ycalib);
  Serial.print("   ZC= ");
  Serial.println(zcalib);

  delay(5000);
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
}

void loop() {
  // === Read acceleromter data === //
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read() | Wire.read() << 8); // X-axis value
  Y_out = ( Wire.read() | Wire.read() << 8); // Y-axis value
  Z_out = ( Wire.read() | Wire.read() << 8); // Z-axis value

  // X_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  // Y_out = Y_out / 256;
  // Z_out = Z_out / 256;

  Serial.print("Xa= ");
  Serial.print(X_out);
  Serial.print("   Ya= ");
  Serial.print(Y_out);
  Serial.print("   Za= ");
  Serial.println(256 - Z_out);

  // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
  // roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
  // pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;

  // Low-pass filter
  // rollF = 0.94 * rollF + 0.06 * roll;
  // pitchF = 0.94 * pitchF + 0.06 * pitch;

  // Serial.print(rollF);
  // Serial.print("/");
  // Serial.println(pitchF);
}