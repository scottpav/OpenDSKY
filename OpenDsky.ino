#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "LedControl.h"
#include "RTClib.h"
#include "RTClib.h"
#include <TinyGPS++.h>
#include<Wire.h>
#include <timer.h>
#include "Sound.h"
#include "Program.h"
#define PIN            6
#define NUMPIXELS      18
#define GPS_SW         7 

Adafruit_NeoPixel neoPixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
LedControl ledControl = LedControl(12,10,11,4);
RTC_DS1307 realTimeClock;
const int MPU_addr=0x69;  // I2C address of the MPU-6050
auto timer = timer_create_default();
TinyGPSPlus gps;
uint32_t activityTimer = millis();
uint32_t compActivityTimer = millis();

void lampit(byte r, byte g, byte b , int lamp) {
    neoPixels.setPixelColor(lamp, neoPixels.Color(r,g,b)); // Set it the way we like it.
    neoPixels.show(); // This sends the updated pixel color to the hardware.
  
}

void validateAction()
{
    if (verb == verbLampTest) {
        mode = modeLampTest;
        //noun = noun_old;
        newAction = false;
    }
    else if ((verb == verbExecuteMajorMode) && (noun == nounIdleMode) && (noun < 1)) {
        action = idleMode;
        newAction = false;
    }
    else if ((verb == verbExecuteMajorMode) && (noun == nounPleasePerform)) {
        action = pleasePerform;
        newAction = false;
    }    
    else if ((verb == verbDisplayDecimal) && (noun == nounApollo13StartUp)) {
        action = apollo13Startup;
        newAction = false;
    }
    else if ((verb == verbDisplayDecimal) && (noun == nounIMUAttitude)) {
        action = displayIMUAttitude;
        newAction = false;
    }
    else if ((verb == verbDisplayDecimal) && (noun == nounIMUgyro)) {
        action = displayIMUGyro;
        newAction = false;
    }
     else if ((verb == verbDisplayDecimal) && (noun == nounCountUpTimer)) {
        timerSeconds = 0;
        timerMinutes = 0;
        timerHours   = 0;
        action = countUpTimer;
        newAction = false;
    }
    else if ((verb == verbDisplayDecimal) && (noun == nounClockTime)) {
        action = displayRealTimeClock;
        newAction = false;
    }
    else if ((verb == verbDisplayDecimal) && (noun == nounLatLongAltitude)) {
        // Display current GPS
        action = displayGPS;
        newAction = false;
        count = 0;
    }
    else if ((verb == verbDisplayDecimal) && (noun == nounRangeTgoVelocity)) {
       // Display Range With 1202 ERROR
       action = lunarDecent;
       newAction = false;
    }
    else if ((verb == verbSetComponent) && (noun == nounClockTime)) {
        action = setTime;
        newAction = false;
    }
    else if ((verb == verbSetComponent) && (noun == nounDate)) {
        action = setDate;
        newAction = false;
    }
    else if ((verb == verbSetComponent) && (noun == nounSelectAudioclip)) {
        action = PlayAudioclip;
        newAction = false;
    }
    else {
        // not (yet) a valid verb/noun combination
        action = none;
        newAction = false;
    }
}

    

    
void illuminateWithRGBAndLampNumber(byte r, byte g, byte b, int lamp) {
    neoPixels.setPixelColor(lamp, neoPixels.Color(r,g,b));
    neoPixels.show();   // show the updated pixel color on the hardware
}

void turnOffLampNumber(int lampNumber) {
    illuminateWithRGBAndLampNumber(0, 0, 0, lampNumber);
}

void setLamp(int color, int lampNumber)
{
    /*  green                   = 1,
        white                   = 2,
        yellow                  = 3,
        orange                  = 4,
        blue                    = 5,
        red                     = 6,
        off                     = 7
    */
    switch (color)
    {
        case green:
            // Statement(s)
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(0,100,0));
            neoPixels.show();   // show the updated pixel color on the hardware
            break;
        case white:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(100,100,100));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        case yellow:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(100,100,0));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        case orange:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(255,165,0));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        case blue:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(0,0,100));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        case red:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(100,0,0));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        case off:
            neoPixels.setPixelColor(lampNumber, neoPixels.Color(0,0,0));
            neoPixels.show();   // show the updated pixel color on the hardware
            // Statement(s)
            break;
        default:
            // Statement(s)
            break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
    }
}

void setDigits(){
     for (int indexa = 0; indexa < 8; indexa ++){
      for (int index = 0; index < 7; index++) {
        digitValue[indexa][index]=0;      
      }
     }
 for (int indexa = 0; indexa < 7; indexa ++){
  if (valueForDisplay[indexa] < 0) {valueForDisplay[indexa] = (valueForDisplay[indexa] - (valueForDisplay[indexa] + valueForDisplay[indexa])); digitValue[indexa][0] = 1;}
  else {digitValue[indexa][0] = 0;}
  for(int index = 0; valueForDisplay[indexa] >= 100000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 100000)) {index++;}
  for(int index = 0; valueForDisplay[indexa] >= 10000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 10000)) {index ++;  digitValue[indexa][1] = index; }
  for(int index = 0; valueForDisplay[indexa] >= 1000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 1000)) { index ++; digitValue[indexa][2] = index; }
  for(int index = 0; valueForDisplay[indexa] >= 100; valueForDisplay[indexa] = (valueForDisplay[indexa] - 100)) { index ++; digitValue[indexa][3] = index; }
  for(int index = 0; valueForDisplay[indexa] >= 10; valueForDisplay[indexa] = (valueForDisplay[indexa] - 10)) { index ++; digitValue[indexa][4] = index; }
  for(int index = 0; valueForDisplay[indexa] >= 1; valueForDisplay[indexa] = (valueForDisplay[indexa] - 1)) { index ++; digitValue[indexa][5] = index; }
 } 
  for(int index = 0; index < 3; index ++){
    bool dpBool = false;
    for(int i=0;i<6;i++) {
    if (i == 0){
    if (digitValue[(index +4)][i] == 1) {ledControl.setRow(index+1,i,B00100100);}
      else {ledControl.setRow(index+1,i,B01110100);}
    }
    else {
      if(action == 3 && index == 2 && i == 3){
        ledControl.setDigit(index+1,i,digitValue[index + 4][i],true);
      }
      else{
        ledControl.setDigit(index+1,i,digitValue[index + 4][i],false);
      }
    }
   }
  } 
}

//void setDigits()
//{
//    for (int indexa = 0; indexa < 8; indexa ++) {
//        for (int index = 0; index < 7; index++) {
//            digitValue[indexa][index] = 0;
//        }
//    }
//
//    for (int indexa = 0; indexa < 7; indexa ++) {
//        if (valueForDisplay[indexa] < 0) {
//            valueForDisplay[indexa] = (valueForDisplay[indexa] - (valueForDisplay[indexa] + valueForDisplay[indexa]));
//            digitValue[indexa][0] = 1;
//        }
//        else {
//            digitValue[indexa][0] = 0;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 100000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 100000)) {
//            index++;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 10000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 10000)) {
//            index++;
//            digitValue[indexa][1] = index;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 1000; valueForDisplay[indexa] = (valueForDisplay[indexa] - 1000)) {
//            index++;
//            digitValue[indexa][2] = index;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 100; valueForDisplay[indexa] = (valueForDisplay[indexa] - 100)) {
//            index++;
//            digitValue[indexa][3] = index;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 10; valueForDisplay[indexa] = (valueForDisplay[indexa] - 10)) {
//            index++;
//            digitValue[indexa][4] = index;
//        }
//        for (int index = 0; valueForDisplay[indexa] >= 1; valueForDisplay[indexa] = (valueForDisplay[indexa] - 1)) {
//            index++;
//            digitValue[indexa][5] = index;
//        }
//    }
//
//    for (int index = 0; index < 3; index++) {
//        // ledControl.clearDisplay(index+1);
//        for (int i = 0; i < 6; i++) {
//            if (i == 0) {
//                if (digitValue[(index+4)][i] == 1) {
//                    ledControl.setRow(index+1, i, B00100100);
//                }
//                else {
//                    ledControl.setRow(index+1, i, B01110100);
//                }
//            }
//            else {
//                ledControl.setDigit(index+1, i, digitValue[index + 4][i], false);
//            }
//        }
//    }
//}

void clearRegister(int dregister)
{
    ledControl.clearDisplay(dregister);
    //ledControl.setRow(dregister,0,B00000000);
    //ledControl.setRow(dregister,1,B00000000);
    //ledControl.setRow(dregister,2,B00000000);
    //ledControl.setRow(dregister,3,B00000000);
    //ledControl.setRow(dregister,4,B00000000);
    //ledControl.setRow(dregister,5,B00000000);
}

void printRegister(int dregister, long number = 0, bool leadzero = true, bool blink = false, bool alarm = false)
{   // Print the Register 1, 2, or 3, the number, if you want leading zeros if you want to blink it, check if it is an alarm
    // Setdigit: Register 0 - 3, plus sign 0, 1-5 numbers
    //num4 = (fm_station / 10) % 10;
    //num3 = (fm_station / 100) % 10;
    //num2 = (fm_station / 1000) % 10;
    //num1 = (fm_station / 10000) % 10;
    int one = 0;
    int ten = 0;
    int hundred = 0;
    long thousand = 0;
    long tenthousand = 0;
    // first, check if the number is positive or negative and set the plus or minus sign
    if (number < 0)
    {
        number = -number;
        // Set the minus sign 
        ledControl.setRow(dregister, 0, B00100100);
    }
    else 
    {
        // Set the plus sign
        ledControl.setRow(dregister, 0, B01110100);
    }
    // now seperate the number
    if (number == 0)
    {
        one = int(number);
    }
    else if ((number > 0) && (number < 10))
    {
        one = int(number);
    }
    else if ((number >= 10) && (number < 100))
    {   
        one = number % 10;
        ten = (number - one) / 10;
    }
    else if ((number >= 100) && (number < 1000))
    {
        one = number % 10;
        ten = (number / 10) % 10;
        hundred = (number / 100) % 10;

    }
    else if ((number >= 1000) && (number < 10000))
    {
        one = number % 10;
        ten = (number / 10) % 10;
        hundred = (number / 100) % 10;
        thousand = (number / 1000) % 10;
    }
    else if ((number >= 10000) && (number < 100000))
    {
        one = number % 10;
        ten = (number / 10) % 10;
        hundred = (number / 100) % 10;
        thousand = (number / 1000) % 10;
        tenthousand = (number / 10000) % 10;
    }
    // show the number
    if (blink == false)
    {
        if (number >= 100000)
        {
            //ledControl.setRow(dregister,0,B00000000);
            //ledControl.setRow(dregister,1,B01001111);
            //ledControl.setRow(dregister,2,B00000000);
            //ledControl.setRow(dregister,3,B00000000);
            //ledControl.setRow(dregister,4,B00000000);
            //ledControl.setRow(dregister,5,B00000000);
            ledControl.setRow(dregister,0,B00000000);
            ledControl.setChar(dregister,1,' ',false);
            ledControl.setChar(dregister,2,'1', false);
            ledControl.setChar(dregister,3,'3', false);
            ledControl.setChar(dregister,4,'0', false);
            ledControl.setChar(dregister,5,'5', false);
        }
        else 
        {
            ledControl.setDigit(dregister, 5, one, false);
            ledControl.setDigit(dregister, 4, ten, false);
            ledControl.setDigit(dregister, 3, hundred, false);
            ledControl.setDigit(dregister, 2, thousand, false);
            ledControl.setDigit(dregister, 1, tenthousand, false);
        }
    }
    if (blink == true)
    {
        if ((toggle600 == true) && (printregtoggle == true))
        {   
            printregtoggle = false;
            ledControl.setDigit(dregister, 5, one, false);
            ledControl.setDigit(dregister, 4, ten, false);
            ledControl.setDigit(dregister, 3, hundred, false);
            ledControl.setDigit(dregister, 2, thousand, false);
            ledControl.setDigit(dregister, 1, tenthousand, false);
        }
        else if ((toggle600 == false) && (printregtoggle == false))
        {
            printregtoggle = true;
            ledControl.clearDisplay(dregister);
            //ledControl.setRow(dregister,0,B00000000);
            //ledControl.setRow(dregister,1,B00000000);
            //ledControl.setRow(dregister,2,B00000000);
            //ledControl.setRow(dregister,3,B00000000);
            //ledControl.setRow(dregister,4,B00000000);
            //ledControl.setRow(dregister,5,B00000000);
        }
    }
}

void printProg(int prog, bool blink = false)
{  // Print the Progam PROG
    int one = 0;
    int ten = 0;
    if (blink == false)
    {
        if (prog == 0)
        {
            ledControl.setRow(0,2,B00000000);
            ledControl.setRow(0,3,B00000000);
        }
        else if ((prog > 0) && (prog < 10))
        {
            ledControl.setDigit(0, 2, 0, false);
            ledControl.setDigit(0, 3, prog, false);
        }
        else if (prog >= 10)
        {   
            one = prog % 10;
            ten = (prog - one) / 10;
            ledControl.setDigit(0, 2, ten, false);
            ledControl.setDigit(0, 3, one, false);
        }
    }
    else if (blink == true)
    {
        ledControl.setRow(0,2,B00000000);
        ledControl.setRow(0,3,B00000000);
    }
}

void printVerb(int verb, bool blink = false)
{  // Print the verb VERB
    int one = 0;
    int ten = 0;
    if (blink == false)
    {
        if (verb == verbNone)
        {
            ledControl.setRow(0,0,B00000000);
            ledControl.setRow(0,1,B00000000);
        }
        else if ((verb > 0) && (verb < 10))
        {
            ledControl.setDigit(0, 0, 0, false);
            ledControl.setDigit(0, 1, verb, false);
        }
        else if (verb >= 10)
        {   
            one = verb % 10;
            ten = (verb - one) / 10;
            ledControl.setDigit(0, 0, ten, false);
            ledControl.setDigit(0, 1, one, false);
        }
    }
    else if (blink == true)
    {
        ledControl.setRow(0,0,B00000000);
        ledControl.setRow(0,1,B00000000);
    }
}

void printNoun(int noun, bool blink = false)
{  // Print the noun NOUN
    int one = 0;
    int ten = 0;
    if (blink == false)
    {
        if (noun == nounNone)
        {
            ledControl.setRow(0,4,B00000000);
            ledControl.setRow(0,5,B00000000);
        }
        else if ((noun > 0) && (noun < 10))
        {
            ledControl.setDigit(0, 4, 0, false);
            ledControl.setDigit(0, 5, noun, false);
        }
        else if (noun >= 10)
        {   
            one = noun % 10;
            ten = (noun - one) / 10;
            ledControl.setDigit(0, 4, ten, false);
            ledControl.setDigit(0, 5, one, false);
        }
    }
    else if (blink == true)
    {
        ledControl.setRow(0,4,B00000000);
        ledControl.setRow(0,5,B00000000);
    }
}


void setDigits(byte maximum, byte digit, byte value)
{//Serial.println("setDigits(byte ...)");
    ledControl.setDigit(maximum, digit, value, false);
}

void flasher()
{
    if (verb_error == true)
    {
        setLamp(orange, lampVerb);
    }
    if (noun_error == true)
    {
        setLamp(orange, lampNoun);
    }
    if (toggle == false) {
        setLamp(white,  lampOprErr);
    } else {
        setLamp(off, lampOprErr);
    }
}

void setdigits(byte maxim, byte digit, byte value){
   ledControl.setDigit(maxim,digit,value,false);
}


void processIdleMode()
{
    if (keyValue != oldKey) {
        fresh = true;
        oldKey = keyValue;
    }
    if (fresh == true) {
        if (keyValue == keyVerb) {
            // verb
            mode = modeInputVerb;
            fresh = false;
            byte keeper = verb;
            for (int index = 0; keeper >= 10 ; keeper = (keeper - 10)) {
                index++;
                verbOld[0] = index;
            }
            for (int index = 0; keeper >= 1; keeper = (keeper - 1)) {
                index++;
                verbOld[1] = index;
            }
        }
        else if (keyValue == keyNoun) {
            // noun
            mode = modeInputNoun;
            fresh = false;
            byte keeper = noun;
            for (int index = 0; keeper >= 10; keeper = (keeper - 10)) {
                index++; nounOld[0] = index;
            }
            for (int index = 0;keeper >= 1; keeper = (keeper - 1)) {
                index++; nounOld[1] = index;
            }
        }
        else if (keyValue == keyProceed) {
            // program
            if(action != pleasePerform)
            {
              mode = modeInputProgram;
              fresh = false;
            }
        }
        else if (keyValue == keyReset) {
            // resrt reeor
            error = 0;
            turnOffLampNumber(13);
            fresh = false;
        }
    }
}

void executeIdleMode()
{   // no action set just reading the kb
    if (newAction == true) {
        validateAction();
    }
    else {
        if (error == 1) {
            flasher();
        }
        keyValue = readKeyboard();
        processIdleMode();
    }
}

void toggleKeyReleaseLamp()
{
    if (toggle == false) {
        setLamp(white, lampKeyRelease);
    }
    else {
        setLamp(off, lampKeyRelease);
    }
}

void processVerbInputMode()
{
    if (keyValue == oldKey) {
        fresh = false;
    }
    else {
        fresh = true;
        oldKey = keyValue;
        if ((error == 1) && (keyValue == keyReset) && (fresh == true))
        {
            error = 0; 
            verb_error = false;
            //turnOffLampNumber(lampOprErr);
            setLamp(green, lampVerb);
            ledControl.setRow(0,0,0);
            ledControl.setRow(0,1,0);
            //verb = ((verbOld[0] * 10) + verbOld[1]);
            fresh = false;
        } //resrt reeor
        if ((keyValue == keyEnter) && (fresh == true)) {
            fresh = false;
            //das vorherige verb in verb_old speichern, mann weiss ja nie
            verb_old2 = verb_old;
            verb_old = verb;
            verb = ((verbNew[0] * 10) + (verbNew[1]));
            if (verb != verb_old)
            {
                // es wurde ein neues Verb eingegeben, daher muss noun auf 0 gesetzt werden
                noun_old2 = noun_old;
                noun_old = noun;
                //noun = 0;
                printNoun(noun);
            }
            // wenn das neue Verb ein anderes als das neue Verb is, dann muss noun auf 0 gesetzt werden

            if ((verb != verbDisplayDecimal)
                && (verb != verbExecuteMajorMode)
                && (verb != verbSetComponent)
                && (verb != verbLampTest)
                && (verb != verbNone)) {
                error = 1;
                verb_error = true;
                verb = ((verbOld[0] * 10) + verbOld[1]);    // restore prior verb
                setLamp(green, lampVerb);
            }
            else {
                turnOffLampNumber(lampOprErr);
                turnOffLampNumber(lampKeyRelease);
                //turnOffLampNumber(lampVerb);
                setLamp(green, lampVerb);
                mode = modeIdle;
                count = 0;
                fresh = false;
                error = 0;
                verb_error = false;
                newAction = true;
            }
        }

        if (fresh == true) {
            if (keyValue == keyRelease) {
                mode = oldMode;
                turnOffLampNumber(lampKeyRelease);
                //turnOffLampNumber(lampVerb);
                setLamp(green, lampVerb);
                count = 0;
                fresh = false;
                if (verb == verbNone) {
                    ledControl.setRow(0,0,0);
                    ledControl.setRow(0,1,0);
                }
                else {
                    setDigits(0, 0, verbOld[0]);
                    setDigits(0, 1, verbOld[1]);
                }
            }
            else if (keyValue == keyNoun) {
                mode = modeInputNoun;
                //turnOffLampNumber(lampVerb);
                setLamp(green, lampVerb);
                count = 0;
                fresh = false;
            }
            else if (keyValue == keyProceed) {
                //program
                if((action != pleasePerform))
                {
                  mode = modeInputProgram;
                  //turnOffLampNumber(lampVerb);
                  setLamp(green, lampVerb);
                  count = 0;
                  fresh = false;
                }
            }
        }

        if ((keyValue <= keyNumber9) && (count < 2)) {
            verbNew[count] = keyValue;
            setDigits(0, count, keyValue);
            count++;
            fresh = false;
        }
    }
}

void executeVerbInputMode()
{
    // inputting the verb
    setLamp(yellow, lampVerb);
    toggleKeyReleaseLamp();
    if (error == 1) {
        flasher();
    }
    keyValue = readKeyboard();
    processVerbInputMode();
}

void processNounInputMode()
{
    if (keyValue == oldKey) {
        fresh = false;
    }
    else {
        fresh = true;
        oldKey = keyValue;
        if ((error == 1) && (keyValue == keyReset) && (fresh == true)) {
            error = 0;
            noun_error = false;
            setLamp(green, lampNoun);
            setLamp(off,lampOprErr);
            fresh = false;
        } //resrt reeor

        if ((keyValue == keyEnter) && (fresh == true)) {
            fresh = false;
            noun_old2 = noun_old;
            noun_old = noun;
            noun = ((nounNew[0] * 10) + (nounNew[1]));
            fresh = false;
            if ((noun != nounIMUAttitude)
                && (noun != nounIdleMode)
                && (noun != nounPleasePerform)
                && (noun != nounIMUgyro)
                && (noun != nounCountUpTimer)
                && (noun != nounClockTime)
                && (noun != nounLatLongAltitude)
                && (noun != nounRangeTgoVelocity)
                && (noun != nounSelectAudioclip)
                && (noun != nounNone)) {
                noun = ((nounOld[0] * 10) + nounOld[1]);    // restore prior noun
                error = 1;
                noun_error = true;
                setLamp(green, lampNoun);
            }
            else {
                turnOffLampNumber(lampOprErr);
                turnOffLampNumber(lampKeyRelease);
                setLamp(green, lampNoun);
                mode = modeIdle;
                count = 0;
                fresh = false;
                error = 0;
                noun_error = false;
                newAction = true;
            }
        }

        if ((keyValue == keyRelease) && (fresh == true)) {
            mode = oldMode;
            turnOffLampNumber(lampKeyRelease);
            setLamp(green, lampNoun);
            count = 0;
            fresh = false;
            if (noun == 0) {
                //verb
                printNoun(noun);
                //ledControl.setRow(0, 4, 0);
                //ledControl.setRow(0, 5, 0);
            }
            else {
                printNoun(noun);
                //setDigits(0, 4, nounOld[0]);
                //setDigits(0, 5, nounOld[1]);
            }
        }
        if ((keyValue == keyVerb) && (fresh == true)) {
            //verb
            mode = modeInputVerb;
            setLamp(green, lampNoun);
            count = 0;
            fresh = false;
        }
        if ((keyValue == keyProceed) && (fresh == true)) {
            if(action != pleasePerform)
            {
              mode = modeInputProgram;
              setLamp(green, lampNoun);
              count = 0;
              fresh = false;
              //program
            }
        }
        if ((keyValue <= keyNumber9)
            && (count < 2)) {
            nounNew[count] = keyValue;
            setDigits(0, (count + 4), keyValue);
            count++;

        }
    }
}

void executeNounInputMode()
{ // inputting the noun
    //Serial.println("Begin exexuteNounInputMode");
    setLamp(yellow, lampNoun);
    toggleKeyReleaseLamp();
    if (error == 1) {
        flasher();
    }
    keyValue = readKeyboard();
    //Serial.println("End exexuteNounInputMode");
    processNounInputMode();
}


void processProgramInputMode()
{
    if (keyValue == oldKey) {
        fresh = false;
    }
    else
    {
        fresh = true;
        oldKey = keyValue;
        if ((error == 1) && (keyValue == keyClear) && (fresh == true))
        {
            error = 0;
            prog = 0;
            newProg = false;
            mode = modeInputProgram;
            count = 0;
            fresh = false;
            turnOffLampNumber(lampOprErr);
            printProg(prog);
        }
        if ((keyValue == keyEnter) && (fresh == true)) 
        {
            fresh = false;
            prog_old2 = prog_old;
            prog_old = currentProgram;
            currentProgram = ((progNew[0] * 10) + (progNew[1]));
            prog = currentProgram;
            fresh = false;
            if ((currentProgram != programNone) && (currentProgram != programJFKAudio) && (currentProgram != programApollo11Audio) && (currentProgram != programApollo13Audio))
            {
                currentProgram = ((progOld[0] * 10) + progOld[1]);    // restore prior noun
                prog = prog_old;
                error = 1;
            }
            else
            {
                turnOffLampNumber(lampOprErr);
                turnOffLampNumber(lampKeyRelease);
                setLamp(green, lampProg);
                mode = modeIdle;
                count = 0;
                fresh = false;
                error = 0;
                newProg = true;
            }
        }
        if ((keyValue == keyRelease) && (fresh == true))
        {
            mode = oldMode;
            prog = prog_old;
            turnOffLampNumber(lampKeyRelease);
            setLamp(green, lampProg);
            count = 0;
            fresh = false;
            printProg(prog);
        }
        if ((keyValue <= keyNumber9) && (count < 2))
        { // now the actual prog values are read, stored and printed
            progNew[count] = keyValue;
            setDigits(0, count+2, keyValue);
            count++;
            fresh = false;
        }
    }
}

void executeProgramInputMode()
{ // inputting the program
    setLamp(yellow, lampProg);
    toggleKeyReleaseLamp();
    if (error == 1) {
        flasher();
    }
    keyValue = readKeyboard();
    processProgramInputMode();
}

//void processkeytime() {
//}

void executeLampTestModeWithDuration(int durationInMilliseconds)
{
  for (int index = 3; index < 18; index++) {
    if(index == 3){setLamp(green, lampCompActy);}
    if(index < 11 && index != 3){lampit(100,100,0, index);}
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
  delay(1500);
  verb = verb_old;
  noun = noun_old;
  for (int index = 0; index < 4; index++) 
  {
    //Off
    ledControl.clearDisplay(0);
    setLamp(off, lampKeyRelease);
    setLamp(off, lampOprErr);
    delay(500);
    
    //On
    if(prog == 0)
    {
      setdigits(0,3,8);
      setdigits(0,2,8);
    }
    else
    {
      printProg(prog);
    }
    if (verb == 0) 
    {
        setdigits(0,0,8);
        setdigits(0,1,8);
        setdigits(0,4,8);
        setdigits(0,5,8);
    }
    else
    {
      printVerb(verb);
      printNoun(noun);
    }
    setLamp(white, lampKeyRelease);
    setLamp(white, lampOprErr);
    delay(500);
  }

  for (int index = 3; index < 11; index++) {lampit(0,0,0, index);}
  for (int index = 11; index < 18; index++) {if(index != 16){lampit(0,0,0, index);}}
    ledControl.clearDisplay(0);
    ledControl.clearDisplay(1);
    ledControl.clearDisplay(2);
    ledControl.clearDisplay(3);
  if (verb == 0) {ledControl.setRow(0,0,0);ledControl.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {ledControl.setRow(0,2,0);ledControl.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {ledControl.setRow(0,4,0);ledControl.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
    delay(1000); 
    setLamp(green, lampVerb);
    setLamp(green, lampNoun);
    setLamp(green, lampProg);
    printProg(prog);
    printVerb(verb);
    printNoun(noun);
    keyValue = keyNone;
    mode = modeIdle;
    validateAction();
}
//void executeLampTestModeWithDuration(int durationInMilliseconds)
//{
//    for (int index = 11; index < 18; index++) {
//        // Uplink Acty, No Att, Stby, Key Rel, Opr Err, --, --
//        delay(200);
//        illuminateWithRGBAndLampNumber(100, 100, 60, index);    // less blue = more white
//    }
//
//    for (int index = 4; index < 11; index++) {
//        // Temp, Gimbal Loc, Prog, Restart, Tracker, Alt, Vel
//        delay(200);
//        illuminateWithRGBAndLampNumber(120, 110, 0, index);     // more yellow
//    }
//
//    for (int lampNumber = 0; lampNumber < 4; lampNumber++) {
//        // Comp Acty, Prog, Verb, Noun
//        delay(200);
//        illuminateWithRGBAndLampNumber(0, 150, 0, lampNumber);
//    }
//
//    int lampTestDigitValue = 8;
//    // passes number "8" to all the 7-segment numeric displays
//    for (int row = 0; row < 4; row++) {
//        // row 0 = Prog/Verb/Noun
//        // row 1 = Register 1
//        // row 2 = Register 2
//        // row 3 = Register 3
//        // ... each has six positions
//        // note: 'digit' # 0 in the three registers is the plus/minus sign
//        for (int digitPosition = 0; digitPosition < 6; digitPosition++) {
//            delay(200);
//            setDigits(row, digitPosition, lampTestDigitValue);
//        }
//    }

//    delay(durationInMilliseconds);
//
//    // reset all lamps
//    for (int index = 0; index < 4; index++) {
//        delay(200);
//        turnOffLampNumber(index);
//    }
//    for (int index = 4; index < 11; index++) {
//        delay(200);
//        turnOffLampNumber(index);
//    }
//    for (int index = 11; index < 18; index++) {
//        delay(200);
//        turnOffLampNumber(index);
//    }
//    for (int index = 0; index < 4; index++) {
//        delay(200);
//        ledControl.clearDisplay(index);
//    }
//
//    // restore previously-displayed values for Verb and Noun
//    setLamp(green, lampVerb);
//    setLamp(green, lampNoun);
//    setLamp(green, lampProg);
//    verb = verb_old;
//    printVerb(verb);
//    printProg(prog);
//    noun = noun_old;
//    printNoun(noun);
//    keyValue = keyNone;
//    mode = modeIdle;
//    validateAction();
//}

void startupsequence(int durationInMilliseconds)
{
    for (int index = 11; index < 18; index++) {
        // Uplink Acty, No Att, Stby, Key Rel, Opr Err, --, --
        delay(50);
        illuminateWithRGBAndLampNumber(100, 100, 60, index);    // less blue = more white
    }

    for (int index = 4; index < 11; index++) {
        // Temp, Gimbal Loc, Prog, Restart, Tracker, Alt, Vel
        delay(50);
        illuminateWithRGBAndLampNumber(120, 110, 0, index);     // more yellow
    }

    for (int lampNumber = 0; lampNumber < 4; lampNumber++) {
        // Comp Acty, Prog, Verb, Noun
        delay(50);
        illuminateWithRGBAndLampNumber(0, 150, 0, lampNumber);
    }

    int lampTestDigitValue = 8;
    // passes number "8" to all the 7-segment numeric displays
    for (int row = 0; row < 4; row++) {
        for (int digitPosition = 0; digitPosition < 6; digitPosition++) {
            delay(50);
            setDigits(row, digitPosition, lampTestDigitValue);
        }
    }

    delay(durationInMilliseconds);

    // reset all lamps
    for (int index = 0; index < 4; index++) {
        delay(50);
        turnOffLampNumber(index);
    }
    for (int index = 4; index < 11; index++) {
        delay(50);
        turnOffLampNumber(index);
    }
    for (int index = 11; index < 18; index++) {
        delay(50);
        turnOffLampNumber(index);
    }
    for (int index = 0; index < 4; index++) {
        delay(50);
        ledControl.clearDisplay(index);
    }

    // restore previously-displayed values for Verb and Noun
    setLamp(green, lampVerb);
    setLamp(green, lampNoun);
    setLamp(green, lampProg);
    keyValue = keyNone;
    mode = modeIdle;
    
}

void actionPleasePerform()
{
  keyValue = readKeyboard();
  if(!stbyToggle)
  {
    setLamp(green, lampCompActy);
    setLamp(white, lampSTBY);
    prog = 6;
    verb = 50;
    noun = 25;
    printProg(prog);
    printVerb(verb);
    printNoun(noun);
    valueForDisplay[4]= 62;
    valueForDisplay[5]= 0;
    valueForDisplay[6]= 0;
    setDigits();
      
    if (flashTimer > millis())  flashTimer = millis();
    if (millis() - flashTimer >= 500 && millis() - flashTimer < 1000) 
    {
      //ledControl.setIntensity(0, 3);
    }
    if (millis() - flashTimer >= 1000) 
    {
      ledControl.setIntensity(0, 15);
      flashTimer = millis(); // reset the timer
    }
  }

  while (keyValue == keyProceed && !stbyToggle)
  {
    if(!stbyToggle)
    {
      if (flashTimer > millis())  flashTimer = millis();
      if (millis() - flashTimer >= 500 && millis() - flashTimer < 1000) 
      {
        //ledControl.setIntensity(0, 3);
      }
      if (millis() - flashTimer >= 1000) 
      {
        ledControl.setIntensity(0, 15);
        flashTimer = millis(); // reset the timer
      }
      
      if (pressedTimer > millis())  pressedTimer = millis();
      if (millis() - pressedTimer >= 1000) 
      {
        pressedDuration++;
        pressedTimer = millis(); // reset the timer
      }
      
      if(pressedDuration > 1)
      {
        setLamp(off, lampProg);
        setLamp(off, lampVerb);
        setLamp(off, lampNoun);
        setLamp(off, lampCompActy);
        for (int index = 0; index < 4; index++) {ledControl.clearDisplay(index); }
        delay(300);
        stbyToggle = 1;
        keyValue = 0;
        delay(1500);
      }   
    }
  }
    
  if(stbyToggle == 1)
  {
    setLamp(white, lampSTBY);
    setLamp(off, lampCompActy);
  }
  
  while (keyValue == keyProceed && stbyToggle == 1)
  {
    if (pressedTimer2 > millis())  pressedTimer2 = millis();
    if (millis() - pressedTimer2 >= 1000) 
    {
     pressedDuration2++;
     pressedTimer2 = millis(); // reset the timer
    }
    if(pressedDuration2 > 1)
    {
      stbyToggle = 0;
      keyValue = 0;
      verb = 16;
      verb = 20;
      prog = 46;
      delay(1500);
      actionApollo13Startup();
      action = apollo13Startup;
      validateAction();
    }
  } 
}

void actionApollo13Startup()
{
  ledControl.setIntensity(0, 15);
  setLamp(off, lampSTBY);
  delay(1000);
  playTrack(5);
  setLamp(white, lampUplinkActy);
  delay(1000);
  playTrack(5);
  setLamp(yellow, lampTemp);
  delay(1000);
  playTrack(5);
  setLamp(white, lampNoAtt);
  delay(1000);
  playTrack(5);
  setLamp(yellow, lampProgCond);
  delay(1000);
  playTrack(5);
  setLamp(white, lampSTBY);
  delay(1000);
  playTrack(5);
  setLamp(yellow, lampTracker);
  delay(1000);
  
  setLamp(white, lampOprErr);
  setLamp(off, lampUplinkActy);
  setLamp(off, lampTemp);
  setLamp(off, lampNoAtt);
  setLamp(off, lampProgCond);
  setLamp(off, lampSTBY);
  setLamp(off, lampTracker);
  setLamp(off, lampOprErr);
  
  for(int i=0;i<4;) 
  {
    if (flashTimer > millis())  flashTimer = millis();
    if (millis() - flashTimer >= 500 && millis() - flashTimer < 1000) 
    {
      setLamp(yellow, lampRestart);
    }
    if (millis() - flashTimer >= 1000) 
    {      
      setLamp(off, lampRestart);
      flashTimer = millis(); // reset the timer
      i++;
    }
  } 
//  playTrack(5);
  setLamp(green, lampNoun);
  delay(500);
  //playTrack(5);
  setLamp(green, lampProg);
  delay(500);
//  playTrack(5);
  setLamp(green, lampVerb);
  delay(500);
//  playTrack(5);
  setLamp(green, lampCompActy);
  delay(500);
  stbyToggle = 0;
  setLamp(off, lampSTBY);
  printProg(prog);
  printVerb(verb);
  printNoun(noun);
  valueForDisplay[4]= 0;
  valueForDisplay[5]= 180;
  valueForDisplay[6]= 0;
  setDigits();
  ledControl.setRow(1, 0, B00100100);
  delay(15000);
  setLamp(off, lampCompActy);
  prog = 0;
  noun = 36;
  verb = 16;
  printProg(prog);
  printNoun(noun);
  printVerb(verb);
  pressedDuration = 0;
  pressedDuration2 = 0;
  action = actionReadTime;
  validateAction();
}

void actionIdleMode()
{
  ledControl.setRow(0,2,B01111110);
  ledControl.setRow(0,3,B01111110);  
  ledControl.setRow(0,4,B01111110);
  ledControl.setRow(0,5,B01111110);
  valueForDisplay[4]= 0;
  valueForDisplay[5]= 0;
  valueForDisplay[6]= 0;
  setDigits();
  delay(2000);
  
  for(int reg=1;reg<4;reg++) 
      { 
          for(int digit=0;digit<6;digit++) 
          {
                ledControl.setRow(reg,digit,B01111110);
                delay(3);
                ledControl.setRow(reg,digit,B01111100);
                delay(3);
                ledControl.setRow(reg,digit,B01111000);
                delay(3);
                ledControl.setRow(reg,digit,B01110000);
                delay(3);
                ledControl.setRow(reg,digit,B01100000);
                delay(3);
                ledControl.setRow(reg,digit,B01000000);
                delay(3);
                ledControl.setRow(reg,digit,B00000000);
                delay(3);
                ledControl.setRow(reg,digit,B00000000);
                delay(3);
          }
      }
  setLamp(off, lampNoAtt);
  action = 0;
  mode = modeIdle;
}

void actionCountUpTimer()
{
  if (oneSecTimer > millis())  oneSecTimer = millis();
  if (millis() - oneSecTimer >= 1000) 
  {
    timerSeconds++;
    if(timerSeconds > 59)
    {
      timerMinutes++;
      timerSeconds = 0;
    }
    if(timerMinutes > 59)
    {
      timerHours++;
      timerMinutes = 0;
    }
    oneSecTimer = millis();
    valueForDisplay[4]= timerHours;
    valueForDisplay[5]= timerMinutes;
    valueForDisplay[6]= timerSeconds;
    setDigits();
    Serial.println(timerSeconds);
  }  
  if (compActivityTimer > millis())  compActivityTimer = millis();
  if (millis() - compActivityTimer >= 200) {
      compActivityTimer = millis(); // reset the timer
      if(!toggle)
      {
        compAct();
      }
  }
}

void actionReadTime()
{
    // read time from real-time clock (RTC)
    DateTime now = realTimeClock.now();
    // 10th and hundreds of seconds
    if( oldSecond < now.second() )
    {
        oldSecond = now.second();
        previousMillis = millis();
    }
    int hundreds = ( ( millis()-previousMillis )/10 )%100;
    int tenth = hundreds - (hundreds % 10);
    printRegister(1,(now.hour()));
    printRegister(2,(now.minute()));
    printRegister(3,((now.second() * 100) + tenth));
}


void actionReadGPS()
{ // Read GPS
  setLamp(white, lampNoAtt);
  if (gpsfix == false)
    {
      if (flashTimer > millis())  flashTimer = millis();
      if (millis() - flashTimer >= 500 && millis() - flashTimer < 1000) 
      {
        setLamp(yellow, lampTracker);
      }
      if (millis() - flashTimer >= 1000) 
      {
        setLamp(off, lampTracker);
        flashTimer = millis(); // reset the timer
      }
      setLamp(off, lampAlt);
      setLamp(off, lampVel);
    }
    else if (gpsfix == true)
    {
        setLamp(off, lampTracker);
        setLamp(white, lampAlt);
        setLamp(yellow, lampVel);
    }
  if (toggle == true && gpsread == true)
  {
    //ALT_light_on()
    digitalWrite(GPS_SW,HIGH);
    delay(100);
    gpsread = false;
    // int index = 0;
    Serial.begin(9600);
    //delay(200);
    while((Serial.available()) && (GPS_READ_STARTED == true))
    {
      setLamp(white, lampAlt);
      if (gps.encode(Serial.read()))
      {
         //setLamp(orange, lampPosition);
         setLamp(orange, lampVel);
         GPS_READ_STARTED = false;
          setLamp(off, lampTracker);
      }
    }
    digitalWrite(GPS_SW,LOW);
   
    //if (gps.location.lat() != 0)
    if (gps.location.isValid() == 1)
    {
        gpsfix = true;
    } 
    //else if (gps.location.lat() == 0)
    else if (gps.location.isValid() != 1)
    {
        gpsfix = false;
    }
    printRegister(1,gps.location.lat()*100);
    printRegister(2,gps.location.lng()*100);
    printRegister(3,gps.altitude.meters());
  }
  if (toggle == false)
  {
     gpsread = true;
     GPS_READ_STARTED = true;
  }
}

void actionSetTime()
{   // read & display time from hardware real-time clock (RTC)
    DateTime now = realTimeClock.now();
    int nowYear = now.year();
    int nowMonth = now.month();
    int nowDay = now.day();
    int nowHour = now.hour();
    int nowMinute = now.minute();
    int nowSecond = now.second();

    while (keyValue == keyEnter) {
        keyValue = readKeyboard();
    }

    while (keyValue != keyEnter) {
        keyValue = readKeyboard();
        if (keyValue != oldKey) {
            oldKey = keyValue;
            if (keyValue == keyPlus) {
                nowHour++;
            }
            if (keyValue == keyMinus) {
                nowHour--;
            }
            if (nowHour > 23) {
                nowHour = 0;
            }
            if (nowHour < 0) {
                nowHour = 23;
            }
        }
        printRegister(1,nowHour);
        printRegister(2,nowMinute);
        printRegister(3,(nowSecond * 100)); // emulate milliseconds
    }

    while (keyValue == keyEnter) {
        keyValue = readKeyboard();
    }

    while (keyValue != keyEnter) {
        keyValue = readKeyboard();
        if (keyValue != oldKey) {
            oldKey = keyValue;
            if (keyValue == keyPlus) {
                nowMinute++;
            }
            if (keyValue == keyMinus) {
                nowMinute--;
            }
            if (nowMinute > 59) {
                nowMinute = 0;
            }
            if (nowMinute < 0) {
                nowMinute = 59;
            }
        }
        printRegister(1,nowHour);
        printRegister(2,nowMinute);
        printRegister(3,(nowSecond * 100));
    }

    while (keyValue == keyEnter) {
        keyValue = readKeyboard();
    }

    while (keyValue != keyEnter) {
        keyValue = readKeyboard();
        if (keyValue != oldKey) {
            oldKey = keyValue;
            if (keyValue == keyPlus) {
                nowSecond++;
            }
            if (keyValue == keyMinus) {
                nowSecond--;
            }
            if (nowSecond > 59) {
                nowSecond = 0;
            }
            if (nowSecond < 0) {
                nowSecond = 59;
            }
        }
        printRegister(1,nowHour);
        printRegister(2,nowMinute);
        printRegister(3,(nowSecond *100));
    }
    realTimeClock.adjust(DateTime(nowYear, nowMonth, nowDay, nowHour, nowMinute, nowSecond));
    action = displayRealTimeClock;
    setDigits(0, 0, 1);
    setDigits(0, 1, 6);
    verb = verbDisplayDecimal;
    verbOld[0] = 1;
    verbOld[1] = 6;
}

void actionSetDate()
{
    byte yearToSet[4];
    byte monthToSet[2];
    byte dayToSet[2];


    DateTime now = realTimeClock.now();
    int nowYear = now.year();
    int nowMonth = now.month();
    int nowDay = now.day();
    int nowHour = now.hour();
    int nowMinute = now.minute();
    int nowSecond = now.second();

    realTimeClock.adjust(DateTime(
                                  ((yearToSet[0] * 10^3) + (yearToSet[1] * 10^2) + (yearToSet[2] * 10) + yearToSet[3]),
                                  ((monthToSet[0] * 10) + monthToSet[1]),
                                  ((dayToSet[0] * 10) + dayToSet[1]),
                                  nowHour,
                                  nowMinute,
                                  nowSecond)
                         );
}

// V21 N98 read & enter & play the selected Audio Clip
void actionSelectAudioclip()
{   // V21 N98 read & enter & play the selected Audio Clip
    // first print initial clipnum = 1
    printRegister(1,clipnum);
    setLamp(off,lampProg);
    // enter can be pressed several times?
    while (keyValue == keyEnter) {
        keyValue = readKeyboard();
    }

    while (keyValue != keyEnter)
    { // now something else than enter has been pressed
        unsigned long blink_currentMillis = millis();
        if (blink_currentMillis - blink_previousMillis >= blink_interval)
        {
            // save the last time you blinked the LED
            blink_previousMillis = blink_currentMillis;
            // if the LED is off turn it on and vice-versa:

            if (blink == true)
            {
                blink = false;
            } else {
                blink = true;
            }
            printVerb(verb, blink);
            printNoun(noun, blink);
        }
        keyValue = readKeyboard();
        if (keyValue != oldKey)
        {
            oldKey = keyValue;
            if (keyValue == keyPlus) {
                clipnum++;
            }
            if (keyValue == keyMinus) {
                clipnum--;
            }
            if (clipnum > clipcount) {
                clipnum = 1;
            }
            if (clipnum < 1) {
                clipnum = clipcount;
            }
        }
        printRegister(1,clipnum);
    }
    action = PlaySelectedAudioclip;
    verb = verbDisplayDecimal;
    noun = nounSelectAudioclip;
    setLamp(green, lampProg);
    printProg(clipnum);
}


void flashUplinkAndComputerActivityRandomly()
{
    if ((toggle600 == true) && (uplink_compact_toggle == true))
    {
        uplink_compact_toggle = false;
        int randomNumber = random(1, 50);
        if ((randomNumber == 15) || (randomNumber == 25)) {
            setLamp(green,lampCompActy);
        }
        else {
            setLamp(off,lampCompActy);
        }
        if ((randomNumber == 17) || (randomNumber == 25)) {
            setLamp(white,lampUplinkActy);
        }
        else {
            setLamp(off,lampUplinkActy);
        }
    }
    else if ((toggle600 == false) && (uplink_compact_toggle == false))
    {
        uplink_compact_toggle = true;
    }
}

void readIMU(int imumode)
{ 
    setLamp(off, lampNoAtt);
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
    int accValueX = 0;
    int accValueY = 0;
    int accValueZ = 0;
    int accCorrX = 0;
    int accCorrY = 0;
    int accCorrZ = 0;
    float accAngleX = 0.0;
    float accAngleY = 0.0;
    float accAngleZ = 0.0;
    int temp = 0;
    int gyroValueX = 0;
    int gyroValueY = 0;
    int gyroValueZ = 0;
    float gyroAngleX = 0.0;
    float gyroAngleY = 0.0;
    float gyroAngleZ = 0.0; 
    float gyroCorrX = 0.0;
    float gyroCorrY = 0.0;
    float gyroCorrZ = 0.0;
  
    accValueX = (Wire.read() << 8) | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    accValueY = (Wire.read() << 8) | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    accValueZ = (Wire.read() << 8) | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    temp = (Wire.read() << 8) | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    gyroValueX = (Wire.read() << 8) | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    gyroValueY = (Wire.read() << 8) | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    gyroValueZ = (Wire.read() << 8) | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
        
    temp = (temp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
        
        
    accCorrX = accValueX - ACCEL_OFFSET;
    accCorrX = map(accCorrX, -ACCEL_SCALE, ACCEL_SCALE, -90, 90);
    accAngleX = constrain(accCorrX, -90, 90);
    // our IMU sits upside down in the DSKY, so we have to flip the angle
    accAngleX = -accAngleX;
    accAngleX = accAngleX + ACC_OFFSET_X;

    accCorrY = accValueY - ACCEL_OFFSET;
    accCorrY = map(accCorrY, -ACCEL_SCALE, ACCEL_SCALE, -90, 90);

    accAngleY = constrain(accCorrY, -90, 90);
    accAngleY = accAngleY + ACC_OFFSET_Y;

    accCorrZ = accValueZ - ACCEL_OFFSET;
    accCorrZ = map(accCorrZ, -ACCEL_SCALE, ACCEL_SCALE, -90, 90);
    accAngleZ = constrain(accCorrZ, -90, 90);
        // our IMU sits upside down in the DSKY, so we have to flip the angle
    accAngleZ = -accAngleZ;
    accAngleZ = accAngleZ + ACC_OFFSET_Z;

    gyroCorrX = (float)((gyroValueX/GYRO_SENSITITY)+GYRO_OFFSET_X);
    gyroAngleX = (gyroCorrX * GYRO_GRANGE) * -LOOP_TIME;
    gyroCorrY = (float)((gyroValueY/GYRO_SENSITITY)+GYRO_OFFSET_Y);
    gyroAngleY = (gyroCorrY * GYRO_GRANGE) * -LOOP_TIME;
    gyroCorrZ = (float)((gyroValueZ/GYRO_SENSITITY)+GYRO_OFFSET_Z);
    gyroAngleZ = (gyroCorrZ * GYRO_GRANGE) * -LOOP_TIME;
    setLamp(off, lampNoAtt);
    if (imumode == Gyro)
        {
            printRegister(1,int(gyroAngleX*100));
            printRegister(2,int(gyroAngleY*100));
            printRegister(3,int(gyroAngleZ*100));
        }
    else if (imumode == Accel)
        {
            printRegister(1,int(accAngleX*100));
            printRegister(2,int(accAngleY*100));
            printRegister(3,int(accAngleZ*100));
    }
    
}

void actionReadIMU(int imumode)
{
  setLamp(off, lampNoAtt);
    if ((toggle600 == true) && (imutoggle == true))
    {   // only every 600ms an imuupdate to avoid flickering
        imutoggle = false;
        flashUplinkAndComputerActivityRandomly();
        readIMU(imumode);
        
        
    }
    else if ((toggle600 == false) && (imutoggle == false))
    {
        flashUplinkAndComputerActivityRandomly();
        imutoggle = true;
        readIMU(imumode);
    }
}


//void jfk(byte jfk)
//{
//    if (audioTrack > 3) {
//        audioTrack = 1;
//    }
//
//    while (audioTrack != jfk) {
//        pinMode(9, OUTPUT);
//        delay(100);
//        pinMode(9, INPUT);
//        delay(100);
//        audioTrack++;
//        if (audioTrack > 3) {
//            audioTrack = 1;
//        }
//    }
//
//    pinMode(9, OUTPUT);
//    delay(100);
//    pinMode(9, INPUT);
//    audioTrack++;
//    currentProgram = programNone;
//    if (currentProgram == 0)
//    {
//      ledControl.setRow(0, 2, 0);
//      ledControl.setRow(0, 3, 0);
//    }
//}
    
void lunarDecentSim(){
  digitalWrite(5, LOW);
  lampit(0,0,0, 16);
  int totalSeconds = 242;
  uint32_t timer2 = millis();
  uint32_t alarmTimer = millis();
  int i=0;
  toggle = 0;
  toggle1201 = 0;
  toggle1202 = 0;
  alarmStatus = 0;
  while(i < totalSeconds){
    if(toggle)
    {
      lampit(100,100,0,6);
    }
    
    if(toggle1201)
    {
    alarmStatus = 1;
    toggle1201 = 0;
    valueForDisplay[4]= 1201;
    valueForDisplay[5]= 1201;
    setDigits();
      for(int i=1;i<4;i++) 
      {
        ledControl.setRow(i,0,B00000000);
        ledControl.setRow(i,1,B00000000); 
        if(i == 3)
        {
          for(int d=0;d<6;d++) 
          {
           ledControl.setRow(i,d,B00000000);
          }
        }
      }      
    }
    
    if(toggle1202)
    {
    alarmStatus = 1;
    toggle1202 = 0;
    valueForDisplay[4]= 1202;
    valueForDisplay[5]= 1202;
    setDigits();
      for(int i=1;i<4;i++) 
      {
        ledControl.setRow(i,0,B00000000);
        ledControl.setRow(i,1,B00000000); 
        if(i == 3)
        {
          for(int d=0;d<6;d++) 
          {
           ledControl.setRow(i,d,B00000000);
          }
        }
      }      
     } 

    if(i == 8)
    {
      radarAltitude = 3000;  
    }

    if(i == 24)
    {
      radarAltitude = 2000;  
    }

    if(i == 59)
    {
      radarAltitude = 700;
      verticalSpeed = 100;  
    }

    if(i == 69)
    {
      radarAltitude = 540; 
      verticalSpeed = 300;  
    }

    if(i == 78)
    {
      fwdVelocity = 40;
      radarAltitude = 400; 
      verticalSpeed = 90;  
    }
    
    if(i == 140)
    {
      radarAltitude = 250; 
      verticalSpeed = 40;  
    }    
    
    if(i == 160)
    {
      radarAltitude = 200; 
      verticalSpeed = 35;  
    }

    if(i == 170)
    {
      radarAltitude = 100; 
      verticalSpeed = 15;  
    }

     if(i == 180)
    {
      radarAltitude = 70; 
      verticalSpeed = 10;  
    }

     if(i == 190)
    {
      fwdVelocity = 19;
      radarAltitude = 50; 
      verticalSpeed = 25;  
    }

   if(i == 200)
    {
      fwdVelocity = 6;
      radarAltitude = 20; 
      verticalSpeed = 10;  
    }

    if(i == 220)
    {
      fwdVelocity = 14;
      radarAltitude = 10; 
      verticalSpeed = 4;  
    }

    
  /////////////////////////
  //     1201 Alarm     //
  /////////////////////// 
    if(i > 8 && i < 18)
      {       
       toggle = 1;
       if(i < 10)
       {
        for(int i=1;i<4;i++) { 
          for(int d=0;d<6;d++) 
          {
           ledControl.setRow(i,d,B00000000);
          }
        }
       }
       if(i > 10 && !alarmStatus)
       {
        toggle1201 = 1;
       }       
      if (alarmTimer > millis())  alarmTimer = millis();
      if (millis() - alarmTimer >= 500 && millis() - alarmTimer < 1000) 
      {
        setLamp(yellow, lampRestart);
      }
      if (millis() - alarmTimer >= 1000) {
        setLamp(off, lampRestart);
        alarmTimer = millis(); // reset the timer
      }
  }

  if (i == 18)
  {
    alarmStatus = 0;
    toggle = 0;
    toggle1201 = 0;
    setLamp(off, lampRestart);
    alarmTimer = millis(); // reset the timer
  }
        
  /////////////////////////
  //       END 1201     //
  ///////////////////////   

   ////////////////////////
  //     1202 Alarm     //
  /////////////////////// 
      if(i > 47 && i < 52){
       toggle = 1;
       if(i < 49)
       {
        for(int i=1;i<4;i++) { 
          for(int d=0;d<6;d++) 
          {
           ledControl.setRow(i,d,B00000000);
          }
        }
       }
       if(i > 49 && !alarmStatus)
       {
        toggle1202 = 1;
       }
      if (alarmTimer > millis())  alarmTimer = millis();
      if (millis() - alarmTimer >= 500 && millis() - alarmTimer < 1000) 
      {
        setLamp(yellow, lampRestart);
      }
      if (millis() - alarmTimer >= 1000) {
        setLamp(off, lampRestart);
        alarmTimer = millis(); // reset the timer
      }     
  }
  if (i == 52)
  {
    toggle = 0;
    toggle1202 = 0;
    setLamp(off, lampRestart);
    alarmTimer = millis(); // reset the timer
  }    
  /////////////////////////
  //       END 1202     //
  ///////////////////////   


  /////////////////////////
  //     PROGRAM 65     //
  /////////////////////// 
  if (i > 75 && i <= 82) 
  {
    printProg(65);
    if(i > 75 && i <= 80)
    {
    printVerb(06);
    printNoun(60);
    if (alarmTimer > millis())  alarmTimer = millis();
      if (millis() - alarmTimer >= 500 && millis() - alarmTimer < 1000) 
      {
        ledControl.setIntensity(0, 3);
      }
      if (millis() - alarmTimer >= 1000) {
        ledControl.setIntensity(0, 15);
        alarmTimer = millis(); // reset the timer
      }
    }     
  }

  if (i > 80 && i <= 92)
  {
      printNoun(68);
      printVerb(16);
      ledControl.setIntensity(0, 15);
  }
  /////////////////////////
  //     PROGRAM 66     //
  /////////////////////// 
  if (i > 92 && i <= 99) 
  {
    printProg(66);
    if(i > 75 && i <= 98)
    {
    if (alarmTimer > millis())  alarmTimer = millis();
      if (millis() - alarmTimer >= 500 && millis() - alarmTimer < 1000) 
      {
        ledControl.setIntensity(0, 3);
      }
      if (millis() - alarmTimer >= 1000) {
        ledControl.setIntensity(0, 15);
        alarmTimer = millis(); // reset the timer
      }
    }
    if (i > 98)
    {
      ledControl.setIntensity(0, 15);
    }     
  }
   ////////////////////////
  //  ALT & VEL LIGHTS  //
 //////////////////////// 
      if(i >= 134 && i < 199){
        lampit(100,100,0,9);
        lampit(100,100,0,10);
      }
  /////////////////////////
  //   END ALT & VEL    //
 ////////////////////////   
      if(!toggle && !toggle1201 && !toggle1202 && i < 256)
      {
        lampit(0,0,0,6);
        if (timer2 > millis())  timer2 = millis();
        if (millis() - timer2 >= 300 && toggle == 0) {
        int randNumb = random(0, 3);
        fwdVelocity = fwdVelocity - randNumb;
        verticalSpeed = verticalSpeed + randNumb;
        radarAltitude = radarAltitude - randNumb;
        if(i > 10 && i < 120)
        {
        //fwdVelocity = fwdVelocity - (randNumb + 3);
        //verticalSpeed = verticalSpeed + (randNumb - 4);
        //radarAltitude = radarAltitude - (randNumb + 5);
        }
        if(i > 120 && i < 192)
        {
        //fwdVelocity = fwdVelocity - (randNumb);
        //verticalSpeed = verticalSpeed + (randNumb);
        //radarAltitude = radarAltitude - (randNumb);
        }    
        if(i > 192 && i < 225)
        {
        //fwdVelocity = fwdVelocity - (randNumb + 10);
        //verticalSpeed = verticalSpeed - randNumb;
        //radarAltitude = radarAltitude - (randNumb);
        }
         
         if (fwdVelocity < 0)
         {
          fwdVelocity = 0; 
         }
         if (verticalSpeed > 0)
         {
          verticalSpeed = 0; 
         }
         if (radarAltitude < 0)
         {
          radarAltitude = 0; 
         }

         if(i > 225) 
         {
          fwdVelocity = 0; 
          verticalSpeed = 0; 
          radarAltitude = 0; 
         }
          valueForDisplay[4]= fwdVelocity;
          valueForDisplay[5]= verticalSpeed;
          valueForDisplay[6]= radarAltitude;
          setDigits();
          ledControl.setRow(2, 0, B00100100);            
          timer2 = millis(); // reset the timer
        }
      }
      if(toggle == 0 && i > 225)
      {
        setLamp(off, lampAlt);
        setLamp(off, lampVel);
      }
      if(toggle == 0 && i > 226)
      {
        setLamp(white, lampNoAtt);
        setLamp(off, lampVel);
      }
      
      if (activityTimer > millis())  activityTimer = millis();
      if (millis() - activityTimer >= 1000) {
          i++;
          activityTimer = millis(); // reset the timer
      }
      if (compActivityTimer > millis())  compActivityTimer = millis();
      if (millis() - compActivityTimer >= 200) {
          compActivityTimer = millis(); // reset the timer
          if(!toggle)
          {
            compAct();
          }
      }
      if(toggle == 0 && i >= 248)
      {
        digitalWrite(5, LOW);
        setLamp(off, lampUplinkActy);
        setLamp(off, lampCompActy);
        printProg(00);
        printVerb(16);
        printNoun(36);
        action = displayRealTimeClock;
        actionReadTime();
      }
    }
 }

 void compAct(){
  int randNumb = random(10, 30); 
  if ((randNumb == 15) || (randNumb == 20) || randNumb == 4 || randNumb == 17 || randNumb > 25) {lampit(0,150,0,3);}
  else {lampit(0,0,0,3);}
  if (randNumb == 9 || randNumb == 3 || randNumb == 27 || randNumb == 19 || randNumb == 6 || randNumb == 15) {lampit(90,90,90,17);}
  else {lampit(0,0,0,17);}
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
  for (int index = 0; index < 4; index++) {ledControl.clearDisplay(index); }
  verbnew[0] = verbold[0]; verbnew[1] = verbold[1];
  verb = ((verbold[0] * 10) + verbold[1]);
  if (verb == 0) {ledControl.setRow(0,0,0);ledControl.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {ledControl.setRow(0,2,0);ledControl.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {ledControl.setRow(0,4,0);ledControl.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
  
  keyValue = 20;
  mode = 0;
  validateAction(); 
  }
  // V16 N98 play the selected Audio Clip
void actionPlaySelectedAudioclip(int clipnum)
{   // V16 N98 play the selected Audio Clip
    printVerb(verb);
    printNoun(noun);
    playTrack(clipnum);
    action = none;
    verb = verbNone;
    noun = nounNone;
    prog = programNone;
    printVerb(verb);
    printNoun(noun);
    printProg(prog);
    setLamp(green, lampProg);
    clearRegister(1);
    clearRegister(2);
    clearRegister(3);
}

  
void setup()
{
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A7, INPUT);
    pinMode(GPS_SW, OUTPUT);        
    digitalWrite(GPS_SW, LOW);
    soundSetup();
    randomSeed(analogRead(A7));
    neoPixels.begin();

    for (int index = 0; index < 4; index++) {
        ledControl.shutdown(index,false);
        ledControl.setIntensity(index, 15);
        ledControl.clearDisplay(index);
    }

    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    realTimeClock.begin();

    // Toggle 
    timer.every(1000, toggle_timer);
    timer.every(600, toggle_timer_600);
    timer.every(100, toggle_timer_250);
    
  Serial.begin(9600);
  //Light up PRO, NOUN, VERB and NO ATT
    startUp();

  for (int index = 0; index < 3; index++){delay(300);lampit(0,150,0, index); lampit(100,100,100, 16);}
  
  while (!Serial)
    ;
    delay(100);
    clearRegister(1);
    delay(100);
    clearRegister(2);
    delay(100);
    clearRegister(3);
    delay(100);

    delay(100);
    //setLamp(white, lampPosition);
    delay(100);
    setLamp(green, lampNoun);
    delay(100);
    setLamp(green, lampVerb);
    delay(100);
    setLamp(green, lampProg);
    delay(100);
   // action = lunarDecent;
}

void loop()
{    
    timer.tick(); // toggle on / off
    if (toggle == true)
    {
        if ((toggle250 == true) && (toggled250 == false))
        {
            //setLamp(white, lampClk);
            toggled250 = true;
        }
        else if ((toggle250 == false) && (toggled250 == true))
        {
            setLamp(off, lampClk);
        }
    }
    else
    {
        //setLamp(off, lampClk);
        if ((toggle250 == true) && (toggled250 == true))
        {
            //setLamp(white, lampClk);
            toggled250 = false;
        }
        else if ((toggle250 == false) && (toggled250 == false))
        {
            setLamp(off, lampClk);
        }
    }
    
    if (currentProgram == programJFKAudio) {
        //jfk(4);
        playTrack(19);
        currentProgram = programNone;
        action = none;
        verb = verbNone;
        noun = nounNone;
        prog = programNone;
        printVerb(verb);
        printNoun(noun);
        printProg(prog);
        clearRegister(1);
        clearRegister(2);
        clearRegister(3);
    }
    else if (currentProgram == programApollo11Audio) {
        playTrack(1);
        currentProgram = programNone;
        action = none;
        verb = verbNone;
        noun = nounNone;
        prog = programNone;
        printVerb(verb);
        printNoun(noun);
        printProg(prog);
        clearRegister(1);
        clearRegister(2);
        clearRegister(3);
    }
    else if (currentProgram == programApollo13Audio) {
        //jfk(3);
        playTrack(2);
        currentProgram = programNone;
        action = none;
        verb = verbNone;
        noun = nounNone;
        prog = programNone;
        printVerb(verb);
        printNoun(noun);
        printProg(prog);
        clearRegister(1);
        clearRegister(2);
        clearRegister(3);
    }
    if (stbyToggle == 1)
    {
        setLamp(white, lampSTBY);
    }
    if(action == apollo13Startup)
    {
      setLamp(off, lampSTBY);
    }
        
    if (mode == modeIdle) 
    {
        executeIdleMode();
        if (action == none || stbyToggle == 1)
        {
            setLamp(white, lampSTBY);
        }
        else if (action != none && stbyToggle == 1)
        {
            setLamp(white, lampSTBY);
        }
        else if (action != none)
        {
            setLamp(off, lampSTBY);
        }
    }

    
    else if (mode == modeInputVerb) {
        executeVerbInputMode();
        setLamp(off, lampSTBY);

    }
    else if (mode == modeInputNoun) {
        executeNounInputMode();
        setLamp(off, lampSTBY);
    }
    else if (mode == modeInputProgram) {
        executeProgramInputMode();
        setLamp(off, lampSTBY);
    }
    else if (mode == modeLampTest) {
        setLamp(off, lampSTBY);
        executeLampTestModeWithDuration(2000);
    }
    if (action == displayIMUAttitude) {
        actionReadIMU(Accel);  // V16N17 ReadIMU Accel
    }
    if (action == idleMode) {
        actionIdleMode();  // V37N00 ExecuteMajorProgram (IdleMode)
    }
    if (action == pleasePerform) {
        actionPleasePerform();  // V37N06
    }
    if (action == displayIMUGyro) {
        actionReadIMU(Gyro);  // V16N18 ReadIMU Gyro
    }
     else if (action == countUpTimer) {
        actionCountUpTimer();   // V16N34 StopWatch
    }
    else if (action == displayRealTimeClock) {
        actionReadTime();   // V16N36 ReadTime
    }
    else if (action == displayGPS) {
        actionReadGPS();    // V16N43 Read GPS
    }
    else if (action == setTime) {
        actionSetTime();    // V21N36 Set The Time
    }
    else if (action == apollo13Startup) {
        actionApollo13Startup();
    }
    else if (action == lunarDecent) {
        printProg(64);
        playTrack(1);
        delay(1000);
        lunarDecentSim();    // V16N68
    }
    else if (action == PlayAudioclip) 
    {   // V21N98 Play Audio Clip
        actionSelectAudioclip();    
    }
    else if (action == PlaySelectedAudioclip) 
    {   // V16N98 Play Audio Clip
        actionPlaySelectedAudioclip(clipnum);    
    }
}
