#include <DFPlayerMini_Fast.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include<Wire.h>
#include "RTClib.h"
#include "LedControl.h"
#include <NMEAGPS.h>
#include <Streamers.h>
#include <GPSport.h>

#define PIN            6
#define RELAY_PIN      7
#define NUMPIXELS      18
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
LedControl lc=LedControl(12,10,11,4);
RTC_DS1307 rtc; 
DFPlayerMini_Fast player;
bool debug = true;
unsigned long previousMillis = 0;   
const int MPU_addr=0x69;  // I2C address of the MPU-6050
long imuval[7];
byte digitval[7][7];   
byte keyVal = 20;
byte oldkey = 20;
bool fresh = 0;
bool navActive = 0;
byte action = 0;
bool error = 0;
byte currentaction = 0;
byte verb = 0;
byte verbnew[2];
byte verbold[2];
byte noun = 0;
byte nounnew[2];
byte nounold[2];
byte prog = 0;
byte prognew[2];
byte progold[2];
byte count = 0;
byte mode = 0;
byte oldmode = 0;
bool toggle = 0;
byte togcount = 0;
bool newAct = 0;
int oldSecond = 0;

bool gpsFix = 0;
int lat = 0;
int lon = 0;
int alt = 0;
int spd = 0;
int hdg = 0;
float range = 0;
float wpLongitude = 0;
String wpLat = "N";
String wpLon = "W";
NeoGPS::Location_t base( 348029032L, -867704843L ); 
static NMEAGPS  gps;
static gps_fix  fix;
static void updateGPS( const gps_fix & fix );
static void updateGPS( const gps_fix & fix ){
  if (fix.valid.location) {
    gpsFix = 1;
    lat = (fix.latitude() * 100);
    lon = (fix.longitude() * 100);
    alt = fix.altitude_ft();
    spd = floor(fix.speed_mph());
    hdg = fix.heading();
    range = fix.location.DistanceMiles( base );
    if ( fix.dateTime.seconds < 10 )
    Serial.print( '0' );
    Serial.print( fix.dateTime.seconds );
    Serial.print( ',' );

    // Serial.print( fix.latitude(), 6 ); // floating-point display
     Serial.print( fix.latitudeL() ); // integer display
    //printL( Serial, fix.latitudeL() ); // prints int like a float
    Serial.print( ',' );
    // Serial.print( fix.longitude(), 6 ); // floating-point display
     Serial.print( fix.longitudeL() );  // integer display
    //printL( Serial, fix.longitudeL() ); // prints int like a float

    Serial.print( ',' );
    if (fix.valid.satellites)
      Serial.print( fix.satellites );

    Serial.print( ',' );
    Serial.print( fix.speed(), 6 );
    Serial.print( F(" kn = ") );
    Serial.print( fix.speed_mph(), 6 );
    Serial.print( F(" mph") );

  } else {
    // No valid location data yet!
    Serial.print( '?' );
  }

  Serial.println();
}

static void GPSloop();
static void GPSloop()
{
  while (gps.available( gpsPort ))
    updateGPS( gps.read() );
}
void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  
  randomSeed(analogRead(A7));
  pixels.begin();
  for(int index = 0; index < 4; index++){
  lc.shutdown(index,false); 
  lc.setIntensity(index,15);
  lc.clearDisplay(index); 
  }
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  rtc.begin();    
  Serial.begin(9600);
  player.begin(Serial);
  //Light up PRO, NOUN, VERB and NO ATT
    startUp();

  for (int index = 0; index < 3; index++){delay(300);lampit(0,150,0, index); lampit(100,100,100, 16);}
  player.volume(30);
  
  while (!Serial)
    ;

  Serial.print( F("NMEA.INO: started\n") );
  Serial.print( F("Heading = ") );
  Serial.println( sizeof(fix.heading()) );
  Serial.print( F("  gps object size = ") );
  Serial.println( sizeof(gps) );
  Serial.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  #ifndef NMEAGPS_RECOGNIZE_ALL
    #error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
  #endif

  #ifdef NMEAGPS_INTERRUPT_PROCESSING
    #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
  #endif

  #if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

    Serial.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

  #else
    if (gps.merging == NMEAGPS::NO_MERGING) {
      Serial.print  ( F("\nWARNING: displaying data from ") );
      Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      Serial.print  ( F(" sentences ONLY, and only if ") );
      Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      Serial.println( F(" is enabled.\n"
                            "  Other sentences may be parsed, but their data will not be displayed.") );
    }
  #endif

  Serial.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  Serial.println( F(" sentence is received.\n"
                        "  You should confirm this with NMEAorder.ino\n") );
  trace_header( Serial );
  Serial.flush();
  gpsPort.begin(9600);

 }

uint32_t timer = millis();
void loop() {
 if (prog == 62){eagleHasLanded();}
 if (prog == 70){haveAProblem();}
 if (prog == 69){weChoose();}  
 if (mode == 0) {mode0();}
 if (mode == 1) {mode1();}
 if (mode == 2) {mode2();}
 if (mode == 3) {mode3();}
 if (mode == 4) {mode4();}
 

 if (togcount == 4) {togcount = 0;if (toggle == 0) {toggle = 1;}else{toggle = 0;}}
 togcount++;
 if (action == 3){ togcount = 4;} else {}
 

 if(action == 1) {action1();} // V16N17 ReadIMU Gyro
 if(action == 2) {action2();compTime();} // V16N36 ReadTime
 if(action == 3) {action3();} // V16N43 GPS POS & ALT
 if(action == 4) {action4();} // V16N87 READ IMU WITH RANDOM 1202 ALARM
 if(action == 5) {action5();} // V21N36 Set The Time
 if(action == 6) {action6();} // V21N37 Set The Date
 if(action == 7) {action7();} // V16N46 GPS VEL & ALT
 if(action == 8) {action8();} // V16N18 ReadIMU Accel
 if(action == 9) {action9();} // V16N19 Read Temp Date & Time
 if(action == 10) {action10();} // V16N68 Apollo 11 Decent & Landing
 
  GPSloop();
}

void mode0() {//no action set just reading the kb
  if (newAct == 1) {validateAct();} else {
  if(error == 1){flasher();}
  keyVal = readkb();
  processkey0();  
}
}

void mode1() {//inputing the verb
 if (timer > millis())  timer = millis();
   lampit(0,0,0, 2);
   flashkr();
 if(error == 1){flasher();}
  if (millis() - timer > 500) { 
    timer = millis(); // reset the timer
   lampit(0,150,0, 2); 
  }
 keyVal = readkb();
 processkey1(); 
}

void mode2() {//inputing the noun
 if (timer > millis())  timer = millis();
   lampit(0,0,0, 0);
   flashkr();
   if(error == 1){flasher();}
  if (millis() - timer > 500) { 
    timer = millis(); // reset the timer
   lampit(0,150,0, 0); 
  }
 keyVal = readkb();
 processkey2(); 
}

void mode3() {//inputing the program
 if (timer > millis())  timer = millis();
   flashkr();
   if(error == 1){flasher();}
     lampit(0,0,0, 1);
  if (millis() - timer > 500) { 
    timer = millis(); // reset the timer
   lampit(0,150,0, 1); 
  }
 keyVal = readkb();
 processkey3();
}
void mode4() {
  for (int index = 0; index < 4; index++){lampit(0,0,0, index);delay(200);lampit(0,150,0, index);}
  for (int index = 4; index < 18; index++) {if(index < 11){lampit(100,100,0, index);}if(index <= 12){lampit(100,100,100, 23-index);}delay(50);}
//for (int index = 11; index < 18; index++) {lampit(100,100,100, index);delay(50);}
  for (int index = 0; index < 4; index++) {
    for (int indexb = 0; indexb < 6; indexb++){
      setdigits(index,indexb,8);
      delay(25);
    }
  }
  delay(5000);
  for (int index = 3; index < 11; index++) {lampit(0,0,0, index);}
  for (int index = 11; index < 18; index++) {if(index != 16){lampit(0,0,0, index);}}
  for (int index = 0; index < 4; index++) {lc.clearDisplay(index); }
  verbnew[0] = verbold[0]; verbnew[1] = verbold[1];
  verb = ((verbold[0] * 10) + verbold[1]);
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {lc.setRow(0,2,0);lc.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
  keyVal = 20;
  mode = 0;
  validateAct(); 
    for (int index = 3; index < 18; index++) {if(index != 16){lampit(0,0,0, index);}}
  }

void action1() {
  readimuGyro();
}

void action2() { // Reads Time from RTC
  DateTime now = rtc.now();
  if(oldSecond < now.second())
  {
    compTime();
    oldSecond = now.second();
  } 
  imuval[4] = (now.hour());
  imuval[5] = (now.minute());
  imuval[6] = (now.second());
  setDigits();  
}

void compTime() {
  if (millis() - timer > 1000) { 
    timer = millis(); // reset the timer
    lampit(0,150,0, 3);
  }
    lampit(0,0,0, 3);    
}

void action3(){
   if (gpsFix == 1)
   {
    lampit(0,0,0, 8);
    lampit(100,100,0, 9);
    lampit(100,100,0, 10);
   }
   else{
    lampit(100,100,100, 16);
    lampit(100,100,0, 8);
   }
   if (fix.dateTime.seconds > 30) {
    imuval[4] = lat;
    imuval[5] = lon;
    imuval[6] = alt;
    setDigits();  
   }
   else {
    imuval[4] = hdg;
    imuval[5] = spd;
    imuval[6] = range;
    setDigits();  
   }
}

//
//void action3(){     //Read GPS
//  delay(20);
//  byte data[83];
//  while((gps.available()) > 0) {int x =  gps.read(); }
//  while((gps.available()) < 1) {int x = 1; }
//  delay(6);
//  int index = 0;
//  while(gps.available() > 0){
//  data[index] = gps.read();
//  delayMicroseconds(960);
//  index++;
//  }
//  int heading = 0;
//  int lat = 0;
//  int lon = 0;
//  int alt = 0;
//  int fix =  data[81] - 48;
//  if (count < 10){
//    count++;
// lat = (((data[18] - 48) * 1000) + ((data[19] -48) * 100) + ((data[20] - 48) * 10) + ((data[21] - 48)));
// lon = (((data[30] - 48) * 10000) + ((data[31] - 48) * 1000) + ((data[32] -48) * 100) + ((data[33] - 48) * 10) + ((data[34] - 48)));
// alt = (((data[52] -48) * 100) + ((data[53] - 48) * 10) + ((data[54] - 48)));
// if(data[index - 20] == 44){
// heading = ((data[index - 18] - 48) * 10) + (data[index - 17] - 48);
// }
// else {
// heading = (((data[index - 20] -48) * 100) + ((data[index - 19] - 48) * 10) + ((data[index - 18] - 48)));
// }
//
//  }
//  else {
//    count++;
// lat = (((data[21] - 48) * 10000) + ((data[23] - 48) * 1000) + ((data[24] -48) * 100) + ((data[25] - 48) * 10) + ((data[26] - 48)));
// lon = (((data[34] - 48) * 10000) + ((data[36] - 48) * 1000) + ((data[37] -48) * 100) + ((data[38] - 48) * 10) + ((data[39] - 48)));
// alt = (((data[52] -48) * 100) + ((data[53] - 48) * 10) + ((data[54] - 48)));
// if(data[index - 20] == 44){
// heading = ((data[index - 18] - 48) * 10) + (data[index - 17] - 48);
// }
// else {
// heading = (((data[index - 20] -48) * 100) + ((data[index - 19] - 48) * 10) + ((data[index - 18] - 48)));
// }
//  if (count > 25) {count = 0;}
// if (data[28] != 78) {lat = ((lat - (lat + lat)));}
// if (data[41] != 69) {lon = ((lon - (lon + lon)));} 
//   imuval[4] = lat;
//   imuval[5] = lon;
//   imuval[6] = heading;
//   setDigits();  
//}   
//delay(20);
//Serial.println(heading);
//
//delay(20);
//}


void action4() { // IMU XYZ Delta
  imu_1202(); 
}

void action5() { // Sets Time To RTC
DateTime now = rtc.now();
  int NYR = now.year();
  int NMO = now.month();
  int NDY = now.day();
  int NHR = now.hour();
  int NMI = now.minute();
  int NSE = now.second();
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NHR++;}
   if(keyVal == 13) {NHR--;}
   if( NHR > 23) {NHR = 0;}
   if(NHR < 0) {NHR = 23;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
   setDigits();
   delay(200);
   lc.clearDisplay(1);
   delay(50); 
  }
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NMI++;}
   if(keyVal == 13) {NMI--;}
   if( NMI > 59) {NMI = 0;}
   if(NMI < 0) {NMI = 59;} 
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
   setDigits(); 
   delay(200);
   lc.clearDisplay(2);
   delay(50); 
  }
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NSE++;}
   if(keyVal == 13) {NSE--;} 
   if( NSE > 59) {NSE = 0;}
   if(NSE < 0) {NSE = 59;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
   setDigits();
   delay(200);
   lc.clearDisplay(3);
   delay(50);  
  }
 rtc.adjust(DateTime(NYR,NMO,NDY,NHR,NMI,NSE));
 action = 2; 
 setdigits(0, 0, 1);
 setdigits(0, 1, 6);
 verbold[0] = 1; 
 verbold[1] = 6; 
 verb = 16; 
}

void action6(){ //Set Date
DateTime now = rtc.now();
  int NYR = now.year();
  int NMO = now.month();
  int NDY = now.day();
  int NHR = now.hour();
  int NMI = now.minute();
  int NSE = now.second();
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NMO++;}
   if(keyVal == 13) {NMO--;}
   if( NMO > 12) {NMO = 0;}
   if(NMO < 0) {NMO = 12;} 
   }
   imuval[4] =  NMO; imuval[5] = (NDY); imuval[6] = NYR;
   setDigits(); 
   delay(200);
   lc.clearDisplay(1);
   delay(50); 
  }
  
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NDY++;}
   if(keyVal == 13) {NDY--;} 
   if( NDY > 31) {NDY = 0;}
   if(NDY < 0) {NDY = 31;}
   }
   imuval[4] =  NMO; imuval[5] = (NDY); imuval[6] = NYR;
   setDigits();
   delay(200);
   lc.clearDisplay(2);
   delay(50);  
  }
  
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NYR++;}
   if(keyVal == 13) {NYR--;}
   }
   imuval[4] =  NMO; imuval[5] = (NDY); imuval[6] = NYR;
   setDigits();
   delay(200);
   lc.clearDisplay(3);
   delay(50); 
  }
  
 rtc.adjust(DateTime(NYR,NMO,NDY,NHR,NMI,NSE)); 
 action = 9; 
 setdigits(0, 0, 1);
 setdigits(0, 1, 6);
 setdigits(0, 4, 1);
 setdigits(0, 5, 9);
 verbold[0] = 1; 
 verbold[1] = 6; 
 nounold[0] = 1; 
 nounold[1] = 9; 
 verb = 16;
 noun = 19; 
}

//$GPRMC,194509.000,A,4042.6142,N,07400.4168,W,2.03,221.11,160412,,,A*77
void action7(){     //Read GPS Heading
  byte data[83];
  while((Serial.available()) > 0) {int x =  Serial.read(); }
  while((Serial.available()) < 1) {int x = 1; }
  delay(6);
  int index = 0;
  while(Serial.available() > 0){
  data[index] = Serial.read();
  delayMicroseconds(960);
  index++;
  }
  int heading = 0;
  int bearing = 0;
  int spd = 0;
   
   if(data[index] == 36 && data[index + 5] == 67){
     heading = (((data[43] -48) * 100) + ((data[44] - 48) * 10) + ((data[45] - 48)));
     //lon = (((data[30] - 48) * 10000) + ((data[31] - 48) * 1000) + ((data[32] -48) * 100) + ((data[33] - 48) * 10) + ((data[34] - 48)));
     spd = (((data[37] -48) * 100) + ((data[38] - 48) * 10) + ((data[39] - 48)));
    } 
   imuval[4] = heading;
   imuval[5] = bearing;
   imuval[6] = spd;
   setDigits();  
  delay(500);
}

void action8(){
  readimuAccel();
}

void action9(){
  tempDateTime();
}

void action10() //V16N68 Apollo 11 Decent & Landing
{
  eagleHasLanded();
  lunarDecentSim();
}

void mode11() {
 compAct(); 
}

int readkb() { 
int value_row1 = analogRead(A0);
int value_row2 = analogRead(A1);
int value_row3 = analogRead(A2);
 if ((value_row1 > 930)&&(value_row2 > 930)&&(value_row3 > 930)) {return 20 ;}// no key
 else if (value_row1 < 225) return 10 ; // Verb
 else if (value_row1 < 370) return 12 ; // +
 else if (value_row1 < 510) return 7 ;
 else if (value_row1 < 650) return 8 ;
 else if (value_row1 < 790) return 9 ;
 else if (value_row1 < 930) return 18 ;  // Clear
 
 else if (value_row2 < 200) return 11 ;  // Noun
 else if (value_row2 < 330) return 13 ;  // -
 else if (value_row2 < 455) return 4 ;
 else if (value_row2 < 577) return 5 ;
 else if (value_row2 < 700) return 6 ;
 else if (value_row2 < 823) return 14 ;  // Program
 else if (value_row2 < 930) return 15 ;  // Enter
 
 else if (value_row3 < 225) return 0 ; 
 else if (value_row3 < 370) return 1 ;
 else if (value_row3 < 510) return 2 ;
 else if (value_row3 < 650) return 3 ;
 else if (value_row3 < 790) return 16 ; // Key Rel
 else if (value_row3 < 930) return 17 ; // Reset
}

void processkey0() {
  if(keyVal != oldkey) { fresh = 1; oldkey = keyVal; }
  if((keyVal == 10) && (fresh == 1)){mode = 1; fresh = 0; byte keeper = verb; 
  for(int index = 0; keeper >= 10; keeper = (keeper - 10)) { index ++; verbold[0] = index; }
  for(int index = 0;keeper >= 1; keeper = (keeper - 1)) { index ++; verbold[1] = index; }}//verb
  if((keyVal == 11) && (fresh == 1)){mode = 2;fresh = 0; byte keeper = noun;
  for(int index = 0; keeper >= 10; keeper = (keeper - 10)) { index ++; nounold[0] = index; }
  for(int index = 0;keeper >= 1; keeper = (keeper - 1)) { index ++; nounold[1] = index; }}//noun
  if((keyVal == 14) && (fresh == 1)){mode = 3; fresh = 0;}//program
  if((keyVal == 17) && (fresh == 1)){error = 0; fresh = 0;} //resrt reeor
}

void processkey1() {
  lampit(0,150,0, 2);
  if(keyVal == oldkey){fresh = 0;} else { fresh = 1; oldkey = keyVal; 
  if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15 || keyVal == 11) && (fresh == 1)) {
  fresh = 0;
  verb = ((verbnew[0] * 10) + (verbnew[1]));
  if((verb != 16) && (verb != 21) && (verb != 35) && (verb != 0)) {error = 1;verb = ((verbold[0] * 10) + verbold[1]);}
  else {
    lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,150,0, 2);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);count = 0; fresh = 0;
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);} else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}}//verb 
  if((keyVal == 11) && (fresh == 1)){mode = 2;count = 0; fresh = 0;}//noun
  if((keyVal == 14) && (fresh == 1)){mode = 3;count = 0; fresh = 0;}//program
  if((keyVal < 10)&&(count < 2)) { verbnew[count] = keyVal;setdigits(0, count, keyVal);count++;fresh = 0;}
}
}

void processkey2() {
  lampit(0,150,0, 0);

  if(keyVal == oldkey){fresh = 0;} else { fresh = 1; oldkey = keyVal; 
if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15 || keyVal == 10) && (fresh == 1)) {fresh = 0;
    noun = ((nounnew[0] * 10) + (nounnew[1]));fresh = 0;
  if((noun != 17) && (noun != 18) && (noun != 19) && (noun != 36) && (noun != 33) && (noun != 37) && (noun != 46) && (noun != 43) && (noun != 68) && (noun != 87)&& (noun != 0)) {error = 1; noun = ((nounold[0] * 10) + nounold[1]);  }
  else {
    lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,150,0, 0);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);count = 0; fresh = 0;
  if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);} else{setdigits(0, 4,nounold[0]);setdigits(0, 5,nounold[1]);}}//verb 
  if((keyVal == 10) && (fresh == 1)){mode = 1;count = 0; fresh = 0;}//verb
  if((keyVal == 14) && (fresh == 1)){mode = 3;count = 0; fresh = 0;}//program
  if((keyVal < 10)&&(count < 2)) {nounnew[count] = keyVal;setdigits(0, (count + 4), keyVal);count++;}
  if(verb == 16 && noun == 87) {prog = 11; setdigits(0, 2, 1);setdigits(0, 3, 1);}
  if(verb == 16 && noun == 68) {prog = 63; setdigits(0, 2, 6);setdigits(0, 3, 3);} 
}
}

void processkey3() {
  lampit(0,150,0, 1);
if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15) && (fresh == 1)) {prog = ((prognew[0] * 10) + (prognew[1]));fresh = 0;
  if((prog != 16) && (prog != 21) && (prog != 35) && (prog != 11) && (prog != 62) && (prog != 69) &&(prog != 70) && (prog != 0)) {error = 1;} else {
    progold[0] = prognew[0]; progold[1] = prognew[1];lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,150,0, 1);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  
 if(keyVal != oldkey) { fresh = 1; oldkey = keyVal; }
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);count = 0; fresh = 0;}//verb 
  if((keyVal == 11) && (fresh == 1)){mode = 2;count = 0; fresh = 0;}//noun
  if((keyVal == 10) && (fresh == 1)){mode = 1;count = 0; fresh = 0;}//verb
  if((keyVal < 10)&&(count < 2)) {prognew[count] = keyVal;setdigits(0, (count + 2), keyVal);count++;}
}

void compAct(){
  int randNumb = random(10, 30); 
  if ((randNumb == 15) || (randNumb == 20) || randNumb == 4 || randNumb == 17) {lampit(0,150,0,3);}
  else {lampit(0,0,0,3);}
  if (randNumb == 9 || randNumb == 17) {lampit(90,90,90,17);}
  else {lampit(0,0,0,17);}
}

void rangeAct(){
int randNumb = random(10, 60); 
    if ((randNumb == 11) || (randNumb == 20)) {lampit(255,193,8,10);}
    else {lampit(0,0,0,10);}
}

void lampit(byte r, byte g, byte b , int lamp) {
    pixels.setPixelColor(lamp, pixels.Color(r,g,b)); // Set it the way we like it.
    pixels.show(); // This sends the updated pixel color to the hardware.
  
}

void setdigits(byte maxim, byte digit, byte value){
   lc.setDigit(maxim,digit,value,false);
}

void flashkr() {
   if(toggle == 0) {lampit(100,100,100, 14);} else {lampit(0,0,0, 14);}
   
}
void flasher() {
   if(toggle == 0) {lampit(100,100,100, 13);} else {lampit(0,0,0, 13);}
}

void validateAct(){
 if(verb == 35) {mode = 4; newAct = 0;}// Lamp Test
 else if((verb == 16) && (noun == 17)) {action = 1;newAct = 0;}//Display IMU Gyro
else if((verb == 16) && (noun == 36)) {action = 2;newAct = 0;}//Display RTC Time 
 else if((verb == 16) && (noun == 43)) {action = 3;newAct = 0;}//Display current GPS
  else if((verb == 16) && (noun == 87)) {action = 4;newAct = 0;}//Display IMU With 1202
 else if((verb == 21) && (noun == 36)) {action = 5;newAct = 0;}//set time
 else if((verb == 21) && (noun == 37)) {action = 6;newAct = 0;}//set date
 else if((verb == 16) && (noun == 46)) {action = 7;newAct = 0;count = 0;}//Display current ALT and Speed
  else if((verb == 16) && (noun == 18)) {action = 8;newAct = 0;}//Display IMU Accel
 else if((verb == 16) && (noun == 19)) {action = 9;newAct = 0;}//Display Temp/Time/Date
 else if((verb == 16) && (noun == 68)) {action = 10;newAct = 0;prog == 63;} //Apollo 11 Decent & Landing
 else if(prog == 11) {action = 8;newAct = 0; verb = 16; noun = 33;}
 else{newAct = 0;action = 0;}
}

void readimuGyro(){
  //Extinguish NO ATT
  lampit(0,0,0, 16);
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
      imuval[4]=Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
      imuval[5]=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
      imuval[6]=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
      Serial.print("GyX = "); Serial.print(imuval[4]);
      Serial.print(" | GyY = "); Serial.print(imuval[5]);
      Serial.print(" | GyZ = "); Serial.println(imuval[6]);
    setDigits();      
 }

void readimuAccel(){
  //Extinguish NO ATT
  lampit(0,0,0, 16);
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
      imuval[4]=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
      imuval[5]=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      imuval[6]=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
      Serial.print("AcX = "); Serial.print(imuval[4]);
      Serial.print(" | AcY = "); Serial.print(imuval[5]);
      Serial.print(" | AcZ = "); Serial.print(imuval[6]);
    setDigits();      
 }

 void tempDateTime(){
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
      
       DateTime now = rtc.now();
       byte MM[2];
       byte DD[2];
       byte HH[2];
       byte mm[2];
       int temp = Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
       temp = (temp/340.00+36.53) * 2;
       MM[0] = now.month() / 10;
       MM[1] = now.month() % 10;
       DD[0] = now.day() / 10;
       DD[1] = now.day() % 10;
       HH[0] = now.hour() / 10;
       HH[1] = now.hour() % 10;
       mm[0] = now.minute() / 10;
       mm[1] = now.minute() % 10;

        
        setdigits(1,1,MM[0]);      
        setdigits(1,2,MM[1]);      
        lc.setRow(1,3,B00000000);
        setdigits(1,4,(DD[0]));      
        setdigits(1,5,DD[1]);
        
        setdigits(2,1,0);
        setdigits(2,2,HH[0]);      
        setdigits(2,3,HH[1]);      
        setdigits(2,4,(mm[0]));      
        setdigits(2,5,mm[1]);
        
        setdigits(3,0,0);
        setdigits(3,1,0);
        setdigits(3,2,0);      
        setdigits(3,3,0);      
        setdigits(3,4,(temp / 10));      
        setdigits(3,5,(temp % 10));
        
      Serial.print("Time = ");Serial.print(HH[0]);Serial.println(HH[1]);Serial.print(mm[0]);Serial.println(mm[1]);
      Serial.print(" | Date = "); Serial.print(MM[0]);Serial.print(MM[1]);Serial.print(DD[0]);Serial.println(DD[1]);
      Serial.print(" | Temp = "); Serial.print(temp);
 }

 void imu_1202(){
  //Extinguish NO ATT
  lampit(0,0,0, 16);
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
       int randNumb = random(10, 700); 
        if (randNumb == 121 || randNumb == 677) {
        playAlarm();         
              keyVal = readkb();
              imuval[4]= 1202;
              imuval[5]= 1202;
              setDigits(); 
            while(keyVal != 18){
            lampit(100,100,0,6);
            keyVal = readkb();
             for(int i=1;i<4;i++) {
                lc.setRow(i,0,B00000000);
                lc.setRow(i,1,B00000000); 
                if(i == 3){
                   for(int d=0;d<6;d++) {
                     lc.setRow(i,d,B00000000);
                   }
                }
             }
            if(keyVal != oldkey) {
              for(int i=0;i<7;i++) {
              player.pause();
              lc.setRow(1,i,B00000000);
                lc.setRow(2,i,B00000000); 
                lc.setRow(3,i,B00000000); 
                lc.setRow(4,i,B00000000); 
              }
            oldkey = keyVal;              
           }
         }
        }
        else {
          lampit(0,0,0,6);
          imuval[4]=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
          imuval[5]=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
          imuval[6]=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
          setDigits();      
        }
 }

  void lunarDecentSim(){
  lampit(0,0,0, 16);
  int totalSeconds = 213;
  uint32_t timer2 = millis();
  int i=0;
  toggle = 0;
  while(i < totalSeconds){
    if(toggle == 1)
    {
      lampit(100,100,0,6);
    }    
  /////////////////////////
  //     1201 Alarm     //
  /////////////////////// 
      if(i > 9 && i < 19){
        toggle = 1;
        if(i == 10) {
           for(int t=1;t<4;t++) {
            for(int d=0;d<6;d++) {
                lc.setRow(t,d,B00000000);
              }
          }
      }
       if(i > 11 && i < 19){
          imuval[4]= 1201;
          imuval[5]= 1201;
          setDigits();
           for(int i=1;i<4;i++) {
                lc.setRow(i,0,B00000000);
                lc.setRow(i,1,B00000000); 
                if(i == 3){
                   for(int d=0;d<6;d++) {
                     lc.setRow(i,d,B00000000);
                   }
                }     
            }
            delay(4000);
            toggle = 0;
            i = 19;        
         }
      }
  /////////////////////////
  //       END 1201     //
  ///////////////////////   

   ////////////////////////
  //     1202 Alarm     //
  /////////////////////// 
      if(i > 50 && i < 63){
        toggle = 1; 
        lampit(100,100,0,6);       
        if(i == 51) {
           for(int t=1;t<4;t++) {
            for(int d=0;d<6;d++) {
                lc.setRow(t,d,B00000000);
              }
          }
      }
       if(i > 52 && i < 63){
          imuval[4]= 1202;
          imuval[5]= 1202;
          setDigits();
           for(int i=1;i<4;i++) {
                lc.setRow(i,0,B00000000);
                lc.setRow(i,1,B00000000); 
                if(i == 3){
                   for(int d=0;d<6;d++) {
                     lc.setRow(i,d,B00000000);
                   }
                }     
            }
            delay(4000);
            toggle = 0;
            i = 63;
         }
      }
  /////////////////////////
  //       END 1202     //
  ///////////////////////   

   ////////////////////////
  //  ALT & VEL LIGHTS  //
 //////////////////////// 
      if(i > 131 && i < 200){
        lampit(100,100,0,9);
        lampit(100,100,0,10);
      }
  /////////////////////////
  //   END ALT & VEL    //
 ////////////////////////   
      if(toggle == 0 && i < totalSeconds + 10)
      {
        lampit(0,0,0,6);
        if (timer2 > millis())  timer2 = millis();
        if (millis() - timer2 >= 300) {
          int randNumb = random(0, 30); 
          imuval[4]= 3120 + randNumb;
          imuval[5]= 6390 + randNumb;
          imuval[6]= 3910 + randNumb;
          setDigits();            
        timer2 = millis(); // reset the timer
        }
      }
      compAct();
      if (timer > millis())  timer = millis();
      if (millis() - timer >= 1000) {
          i++;
          timer = millis(); // reset the timer
      }
    }
    action = 0;
    mode = 0;
    prog = 0; 
 }
 
 void setDigits(){
     for (int indexa = 0; indexa < 8; indexa ++){
      for (int index = 0; index < 7; index++) {
        digitval[indexa][index]=0;      
      }
     }
 for (int indexa = 0; indexa < 7; indexa ++){
  if (imuval[indexa] < 0) {imuval[indexa] = (imuval[indexa] - (imuval[indexa] + imuval[indexa])); digitval[indexa][0] = 1;}
  else {digitval[indexa][0] = 0;}
  for(int index = 0; imuval[indexa] >= 100000; imuval[indexa] = (imuval[indexa] - 100000)) {index++;}
  for(int index = 0; imuval[indexa] >= 10000; imuval[indexa] = (imuval[indexa] - 10000)) {index ++;  digitval[indexa][1] = index; }
  for(int index = 0; imuval[indexa] >= 1000; imuval[indexa] = (imuval[indexa] - 1000)) { index ++; digitval[indexa][2] = index; }
  for(int index = 0; imuval[indexa] >= 100; imuval[indexa] = (imuval[indexa] - 100)) { index ++; digitval[indexa][3] = index; }
  for(int index = 0; imuval[indexa] >= 10; imuval[indexa] = (imuval[indexa] - 10)) { index ++; digitval[indexa][4] = index; }
  for(int index = 0; imuval[indexa] >= 1; imuval[indexa] = (imuval[indexa] - 1)) { index ++; digitval[indexa][5] = index; }
 } 
  for(int index = 0; index < 3; index ++){
  // lc.clearDisplay(index+1);  
  for(int i=0;i<6;i++) {
    if (i == 0){
    if (digitval[(index +4)][i] == 1) {lc.setRow(index+1,i,B00100100);}
      else {lc.setRow(index+1,i,B01110100);}
    }
    else {
    lc.setDigit(index+1,i,digitval[index + 4][i],false);
    }
 } 
 }
 }

 void weChoose()
    {
      player.play(4);   
      prog=11;   
    }
    
 void eagleHasLanded()
    {
      player.play(1);      
      prog=11;
    }

 void playAlarm()
    {
      player.play(3);  
      prog=11;
    }
    
 void haveAProblem()
    {
      player.play(2);
      prog=11;
    }

void startUp() {
  for (int index = 0; index < 4; index++){lampit(0,0,0, index);delay(200);lampit(0,150,0, index);}
  for (int index = 4; index < 18; index++) {
    if(index < 11){lampit(100,100,0, index);}
    if(index <= 12){lampit(100,100,100, 23-index);}
    delay(50);
  }
//for (int index = 11; index < 18; index++) {lampit(100,100,100, index);delay(50);}
  for (int index = 0; index < 4; index++) {
    for (int indexb = 0; indexb < 6; indexb++){
      setdigits(index,indexb,8);
      delay(25);
    }
  }
  delay(1000); 
  for (int index = 3; index < 11; index++) {lampit(0,0,0, index);}
  for (int index = 11; index < 18; index++) {if(index != 16){lampit(0,0,0, index);}}
  for (int index = 0; index < 4; index++) {lc.clearDisplay(index); }
  verbnew[0] = verbold[0]; verbnew[1] = verbold[1];
  verb = ((verbold[0] * 10) + verbold[1]);
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {lc.setRow(0,2,0);lc.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
  
  keyVal = 20;
  mode = 0;
  validateAct(); 
  }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////
           //    ####################    T H E   B O N E   Y A R D    ##########################      //
          /////////////////////////////////////////////////////////////////////////////////////////////

//void action7(){     //Read GPS VEL & ALT
// if(navActive == 0){
//  lc.setRow(1,0,B00000000);
//  lc.setRow(1,1,B00000000);
//  lc.setRow(1,5,B00000000);
//  lc.setRow(2,0,B00000000);
//  lc.setRow(2,3,B00000000);
//  lc.setRow(2,4,B00000000);
//  lc.setRow(2,5,B00000000);
//  lc.setRow(3,0,B00000000);
// while(keyVal == 15){ keyVal = readkb();}
// count = 0; fresh = 0;
//  while(keyVal != 15 && fresh == 0){
//  lc.setRow(1,2,B00001110);//L
//  lc.setRow(1,3,B01110111);//A
//  lc.setRow(1,4,B00001111);//t
//
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//     oldkey = keyVal;
//     if(keyVal == 12){
//          lc.setRow(2,0,B01110100);
//          wpLat = "N";
//        }
//        if(keyVal == 13){
//          lc.setRow(2,0,B00100100);
//          wpLat = "S";
//        }
//      if((keyVal < 10) && (count < 2)) {
//          wpLatDDNew[count] = keyVal;
//          setdigits(2, count+1, keyVal);
//          count++;
//      }
//      if(keyVal == 18) {
//        fresh = 1;
//      }
//      if(fresh == 1){
//        lc.clearDisplay(2);
//        count = 0;
//      }
//   }
// }
//int wpLatitudeDD = ((wpLatDDNew[0] * 10) + wpLatDDNew[1]);
//while(keyVal == 15){ keyVal = readkb();}
// count = 0; fresh = 0;
//  while(keyVal != 15 && fresh == 0){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//     oldkey = keyVal;
//      if((keyVal < 10) && (count < 5)) {
//          wpLatMMNew[count] = keyVal;
//          setdigits(3, count+1, keyVal);
//          count++;
//      }
//      if(keyVal == 18) {
//        fresh = 1;
//      }
//      if(fresh == 1){
//        lc.clearDisplay(3);
//        count = 0;
//      }
//   }
//  }
//int wpLatitudeMM = ((wpLatMMNew[0] *1000) + (wpLatMMNew[1] * 100) + (wpLatMMNew[2] * 10) + wpLatMMNew[3]);
//wpLatitude = (float) wpLatitudeDD + (float) wpLatitudeMM;
//        lc.clearDisplay(1);
//        lc.clearDisplay(2);
//        lc.clearDisplay(3);
//        lc.setRow(1,2,B00001110); //L
//        lc.setRow(1,3,B01111110);// O
//        lc.setRow(1,4,B01110110);//N
// while(keyVal == 15){ keyVal = readkb();}
// count = 0; fresh = 0;
//  while(keyVal != 15 && fresh == 0){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//     oldkey = keyVal;
//     if(keyVal == 12){
//          lc.setRow(2,0,B01110100);
//          wpLon = "W";
//        }
//        if(keyVal == 13){
//          lc.setRow(2,0,B00100100);
//          wpLon = "E";
//        }
//      if((keyVal < 10) && (count < 2)) {
//          wpLonDDNew[count] = keyVal;
//          setdigits(2, count+1, keyVal);
//          count++;
//      }
//      if(keyVal == 18) {
//        fresh = 1;
//      }
//      if(fresh == 1){
//        lc.clearDisplay(2);
//        count = 0;
//      }
//   }
// }
// 
//int wpLongitudeDD = ((wpLonDDNew[0] * 10) + wpLonDDNew[1]);
//while(keyVal == 15){ keyVal = readkb();}
// count = 0; fresh = 0;
//  while(keyVal != 15 && fresh == 0){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//     oldkey = keyVal;
//      if((keyVal < 10) && (count < 5)) {
//          wpLonMMNew[count] = keyVal;
//          setdigits(3, count+1, keyVal);
//          count++;
//      }
//      if(keyVal == 18) {
//        fresh = 1;
//      }
//      if(fresh == 1){
//        lc.clearDisplay(3);
//        lc.clearDisplay(3);
//        count = 0;
//      }
//   }
//  }
//int wpLongitudeMM = ((wpLonMMNew[0] *1000) + (wpLonMMNew[1] * 100) + (wpLonMMNew[2] * 10) + wpLonMMNew[3]);
//wpLongitude = (float) wpLongitudeDD + (float) wpLongitudeMM;
//navActive = 1;
//for(int i=2;i<4;i++) {
//    lc.setRow(i,0,B00100100);
//    lc.setChar(i,1,'-',false);
//    lc.setChar(i,2,'-',false);
//    lc.setChar(i,3,'-',false);
//    lc.setChar(i,4,'-',false);
//    lc.setChar(i,5,'-',false);
//    setDigits(); 
//    }
// }
//  //  executeNav();
//}
//void action3(){     //Read GPS POS & ALT
//  digitalWrite(RELAY_PIN, HIGH);
//  delay(20);
//  useInterrupt(true);
//  delay(1000);
//  Serial.println(PMTK_Q_RELEASE);
//  int lat = 0;
//  int lon = 0;
//  int alt = 0;
//  delay(20);
//  byte data[83];
//  int index = 0;
//  if (GPS.newNMEAreceived()) {
//    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
//      return;  // we can fail to parse a sentence in which case we should just wait for another
//  } 
//  if (timer > millis())  timer = millis();
//  if (millis() - timer > 1000) { 
//    timer = millis(); // reset the timer
//    if (GPS.fix) {
//      data[index] = GPS.read();
//      delayMicroseconds(960);
//      index++;
//      if(index >= 72) {index = 71; }
//      lat = GPS.latitude;
//      lon = GPS.longitude;
//      alt = GPS.altitude;
//      imuval[4] = lat;
//      imuval[5] = lon;
//      imuval[6] = alt;
//      digitalWrite(RELAY_PIN, LOW);
//      setDigits();  
//    }
//  }
//}

//void executeNav(){ 
//  if (!GPS.fix) {
//   lampit(255,0,0, 16);
//   lampit(255,200,59, 15);
//  }
//  Serial.println(wpLat + "" + wpLatitude + " " + wpLon + wpLongitude);
// if (GPS.newNMEAreceived()) {
//    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
//      return;  // we can fail to parse a sentence in which case we should just wait for another
//  }
//  if (timer > millis())  timer = millis();
//  if (millis() - timer > 2000) { 
//    timer = millis(); // reset the timer
//    if (GPS.fix) {
//       compAct(); 
//      lampit(0,0,0, 16);
//      lampit(0,0,0, 15);
//      lampit(0,150,0, 9);
//      lampit(0,150,0, 10);
//      wpGPS = input2string (wpLat, wpLatitude, wpLon, wpLongitude);
//      here = gps2string ((String) GPS.lat, GPS.latitude, (String) GPS.lon, GPS.longitude);
//      range = (haversine(string2lat(here), string2lon(here), string2lat(wpGPS), string2lon(wpGPS)))*0.000621371;  // Miles ("*0.000621371 converted form meters to miles)
//      rangeft = range*5280;                  // convert the range to feet
//      bearing = (bearingcalc(string2lat(here), string2lon(here), string2lat(wpGPS), string2lon(wpGPS)));  // Determins the angle in radians of the bearing to desired location
//      float rangeDisplay = range;
//      float bearingDisplay = bearing * 100 ;
//      if(range < 1){
//        rangeDisplay = rangeft;
//      }
//      if(bearingDisplay < 0){
//        bearingDisplay = bearingDisplay + 359;
//      }
//       if(bearingDisplay == 0){
//        bearingDisplay = 360;
//      }
//  delay(20);
//      imuval[4] = GPS.angle;
//      imuval[5] = bearingDisplay;
//      imuval[6] = rangeDisplay;
//      setDigits(); 
//    }
//  }
//}

//void action8() { // T-Minus countdown timer
//DateTime now = rtc.now();
//  int NHR = 0;
//  int NMI = 0;
//  int NSE = 0;
//  unsigned long currentMillis = millis();
//  while(keyVal == 15){ keyVal = readkb();}
//  while(keyVal != 15){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//     oldkey = keyVal;
//     if(keyVal == 12) {NHR++;}
//     if(keyVal == 13) {NHR--;}
//     if( NHR > 23) {NHR = 0;}
//     if(NHR < 0) {NHR = 23;}
//   }
//   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
//   if (timer > millis())  timer = millis();
//      if (millis() - timer >= 200) {    // save the last time you blinked the LED
//        timer = millis(); // reset the timer
//      // if the LED is off turn it on and vice-versa:
//      if (displayOn) {      // this makes Pin2 blinks off-on
//        lc.clearDisplay(1);
//        displayOn = false;
//        } 
//      else {
//        displayOn = true;
//        setDigits();
//      }
//    }
//  }
//  while(keyVal == 15){ keyVal = readkb();}
//  while(keyVal != 15){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//    oldkey = keyVal;
//   if(keyVal == 12) {NMI++;}
//   if(keyVal == 13) {NMI--;}
//   if( NMI > 59) {NMI = 0;}
//   if(NMI < 0) {NMI = 59;} 
//   }
//   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
// if (timer > millis())  timer = millis();
//      if (millis() - timer >= 200) {    // save the last time you blinked the LED
//        timer = millis(); // reset the timer
//      // if the LED is off turn it on and vice-versa:
//      if (displayOn) {      // this makes Pin2 blinks off-on
//        lc.clearDisplay(2);
//        displayOn = false;
//        } 
//      else {
//        displayOn = true;
//        setDigits();
//      } 
//    }
//  } 
//  while(keyVal == 15){ keyVal = readkb();}
//  while(keyVal != 15){
//   keyVal = readkb();
//   if(keyVal != oldkey) {
//    oldkey = keyVal;
//   if(keyVal == 12) {NSE++;}
//   if(keyVal == 13) {NSE--;} 
//   if( NSE > 59) {NSE = 0;}
//   if(NSE < 0) {NSE = 59;}
//   }
//   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
//   if (timer > millis())  timer = millis();
//      if (millis() - timer >= 200) {    // save the last time you blinked the LED
//        timer = millis(); // reset the timer
//      // if the LED is off turn it on and vice-versa:
//      if (displayOn) {      // this makes Pin2 blinks off-on
//        lc.clearDisplay(3);
//        displayOn = false;
//        } 
//      else {
//        displayOn = true;
//        setDigits();
//      }
//    }
//  }
// startCountdown(NHR , NMI , NSE);
//}
//void startCountdown(int HOURS, int MINUTES, int SECONDS){
// action = 8; setdigits(0, 0, 1);setdigits(0, 1, 6);setdigits(0, 4, 3);setdigits(0, 5, 3);verbold[0] = 1; verbold[1] = 6; verb = 16; noun = 33; nounold[0] = 3; nounold[1] = 3; 
// int totalSeconds = (HOURS * 60) * 60 + (MINUTES * 60) + SECONDS;
//  imuval[4] = HOURS;
//  imuval[5] = MINUTES;
//  imuval[6] = SECONDS;
//    compAct();  
// 
// for(int i = 0; i < totalSeconds;){ 
// keyVal = 15;
//   oldkey = readkb();
//   if (oldkey != keyVal){
//
//    if (timer > millis())  timer = millis();
//      if (millis() - timer > 1000) { 
//        compAct();  
//        timer = millis(); // reset the timer
//        if(SECONDS < 1 && MINUTES > 0){
//          SECONDS = 59;
//          MINUTES--;
//        }
//        if(MINUTES < 1 && HOURS > 0){
//          MINUTES = 59;
//          HOURS --;
//        }
//        imuval[4] = - HOURS;
//        imuval[5] = - MINUTES;
//        imuval[6] = - SECONDS;
//        i++;
//        SECONDS--;
//        setDigits();  
//       }    
//  }
//    //ACTIVATE BUZZER
// }
//}
//
