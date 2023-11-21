// base sketch from https://brainy-bits.com/tutorials/4-bits-7-segment-led-display-with-arduino/
// changed TM1637 library by niq_ro for degree, r & h letter
// DS3231 clock on TM1637 LED display by niq_ro from http://www.tehnic.go.ro
// & http://arduinotehniq.com/


#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html
Encoder knob(3, 2); //encoder connected to pins 2 and 3 (and ground)
#define buton 6

#include "TM1637.h"  // https://github.com/tehniq3/TM1637-display
 
//{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//0~9,A,b,C,d,E,F,"-"," ",degree,r,h

#define CLK 7//Pins for TM1637       
//#define CLK 7//Pins for CLK TM1637a,b,c      

// TM1637 hour minutes
#define DIO1 8 
TM1637 tm1637a(CLK,DIO1);

// TM1637 month day
#define DIO2 9
TM1637 tm1637b(CLK,DIO2);

#define DIO3 10
TM1637 tm1637c(CLK,DIO3);

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;

int hh, mm, mo, d, y ;

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 2500; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag
int sethourstemp;
int setminstemp;
int setmonthtemp;
int setdaytemp;
int setyeartemp;

int greenAM = 12;
int greenPM = 13;

#define goarna 11 // buzzer

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


//// Module MP3
static const uint8_t PIN_MP3_TX = 4; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 5; // Connects to module's TX 
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;
 

void setup()
{
  Serial.begin(9600);
  softwareSerial.begin(9600);  
  tm1637a.init();
  tm1637a.set(5); 
  tm1637b.init();
  tm1637b.set(5); 
  tm1637c.init();
  tm1637c.set(5); 

  pinMode(greenAM, OUTPUT);
  pinMode(greenPM, OUTPUT);

  player.begin(softwareSerial);
  player.volume(0);
  delay(1000);
  player.volume(14);
  player.play(1);  
  delay(5000); 
 
  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  rtc.begin();
  // initialisation du module RTC      
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");   
  } 
   
  // manual adjust
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));


  pinMode(buton,INPUT);//push button on encoder connected 
  digitalWrite(buton,HIGH); //Pull at 1 high

   pinMode(goarna, OUTPUT);   
   digitalWrite(goarna, LOW); //set in 0 logical level
   tone(goarna, 4000,50); 
   
}//end "setup()"
 
void loop(){
DateTime now = rtc.now();
hh = now.hour(), DEC;
mm = now.minute(), DEC;
mo = now.month(), DEC;
d = now.day(), DEC;
y = now.year(), DEC;

if(now.hour()>=13){
 
  digitalWrite(greenAM,0);
  digitalWrite(greenPM,1);}
  
else if(now.hour()==12)
  {
   digitalWrite(greenAM,0);
  digitalWrite(greenPM,1);}

else{
  digitalWrite(greenAM,1);
  digitalWrite(greenPM,0);
  }

    tm1637a.point(POINT_ON);
    tm1637a.display(0,hh/10);     // hour
    tm1637a.display(1,hh%10);
    tm1637a.display(2,mm/10);    // minutes
    tm1637a.display(3,mm%10);    // 
delay(500);

    pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    
    if (pushlength > pushlengthset)
    {
    player.play(2);  
    tone(goarna, 2000,50);
    //   ShortPush ();   
    }
      
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength < pushlengthset) {
        tone(goarna, 1000,50);
         player.play(2);  
       //  delay(500);
         DateTime now = rtc.now();
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;     
         setmonthtemp=now.month(),DEC; 
         setdaytemp=now.day(),DEC;
         setyeartemp=now.year(),DEC;      
         setclock();
         pushlength = pushlengthset;
       };

    tm1637a.point(POINT_OFF);
    tm1637a.display(0,hh/10);     // hour
    tm1637a.display(1,hh%10);
    tm1637a.display(2,mm/10);    // minutes
    tm1637a.display(3,mm%10);    //     
    delay(500);

   
    tm1637b.display(0,d/10);     // day
    tm1637b.display(1,d%10);
    tm1637b.display(2,mo/10);    // month
    tm1637b.display(3,mo%10);    //

    
    tm1637c.display(0,y/1000);     // 2
    tm1637c.display(1,(y%1000)/100); // 0
    tm1637c.display(2,((y%1000)%100)/10); // 2
    tm1637c.display(3,((y%10000)%100)%10);  // 3 
    
}// end loop() 

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(buton);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
              player.play(2); 
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}

int sethours () {
    tm1637a.display(0,sethourstemp/10);     // hour
    tm1637a.display(1,sethourstemp%10);
    tm1637a.display(2,16);    // minutes
    tm1637a.display(3,16);    // 
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 0) {
      sethourstemp = 23;
    }
    if (sethourstemp > 23) {
      sethourstemp = 0;
    }
    sethours();
}

int setmins () {
    tm1637a.display(0,16);     // hour
    tm1637a.display(1,16);
    tm1637a.display(2,setminstemp/10);    // minutes
    tm1637a.display(3,setminstemp%10);    // 
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 59;
    }
    if (setminstemp > 59) {
      setminstemp = 0;
    }
    setmins();
}

int setmonth () {
    tm1637a.display(0,sethourstemp/10);     // hour
    tm1637a.display(1,sethourstemp%10);
    tm1637b.display(0,16);     // day
    tm1637b.display(1,16);
    tm1637b.display(2,setmonthtemp/10);    // month
    tm1637b.display(3,setmonthtemp%10);    // 
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 0) {
      setmonthtemp = 12;
    }
    if (setmonthtemp > 12) {
      setmonthtemp = 1;
    }
    setmonth();
}

int setday() {
    tm1637b.display(0,setdaytemp/10);     // day
    tm1637b.display(1,setdaytemp%10);
    tm1637b.display(2,16);    // month
    tm1637b.display(3,16);    // 
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp + knobval;
    if (setdaytemp < 0) {
      setdaytemp = 31;
    }
    if (setdaytemp > 31) {
      setdaytemp = 1;
    }
    setday();
}
int setyear() {
  /*
    tm1637c.display(0,y/1000);     // 2
    tm1637c.display(1,(y%1000)/100); // 0
    tm1637c.display(2,((y%1000)%100)/10); // 2
    tm1637c.display(3,((y%10000)%100)%10);  // 3 
   */
    tm1637b.display(2,setmonthtemp/10);    // month
    tm1637b.display(3,setmonthtemp%10);    // 
    tm1637c.display(0,16);    // 2 = -
    tm1637c.display(1,16);    // 0 = -
    tm1637c.display(2,((setyeartemp%1000)%100)/10); // 2
    tm1637c.display(3,((setyeartemp%10000)%100)%10);  // 3  
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 21) {
      setyeartemp = 30;
    }
    if (setyeartemp > 30) {
      setyeartemp = 21;
    }
    setyear();
}


//sets the clock
void setclock (){
   sethours ();
   delay(500);
   setmins ();
   delay(500);
   setmonth ();
   delay(500);
   setday();
   delay(500);
   setyear();
   tm1637c.display(0,y/1000);     // 2
   tm1637c.display(1,(y%1000)/100); // 0
   player.volume(0);
   rtc.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,0));
   delay (500); 
}
