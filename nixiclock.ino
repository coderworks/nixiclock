// watch out for frying the tubes...

/*
 This program is written to run a clock program on an Arduino Micro
 It drives 3 DM/SN7441AN Nixi tube drivers and 3 relais + bc548b to show the
 numbers on the nixi tubes.
 There are 470ohm resistors added between every pin from the Arduino to the
 drivers.
 A seperate power supply is added for the 180v needed to power the nixi tubes.

  Pin-out arduino micro:
    [physical pin] = [pin name and or number and connection]
       ____________________
    1  = MOSI  n/c     34 = SCLK        n/c
    2  = SS    RX      33 = MISO        TX
    3  = TX    n/c     32 = Vin         Vin from external PSU
    4  = RX    n/c     31 = GND         GND
    5  = Reset n/c     30 = Reset       n/c
    6  = Gnd   n/c     29 = 5V          power to drivers
    7  = 2     n/c     28 = -           n/c
    8  = 3     n/c     27 = -           n/c
    9  = 4     d3-c    26 = A5          d2-d
    10 = 5     d3-b    25 = A4          d2-a
    11 = 6     d3-d    24 = A3          d1-c
    12 = 7     d3-a    23 = A2          d1-b
    13 = 8     d2-b    22 = A1          d1-d
    14 = 9     d2-c    21 = A0          d1-a
    15 = 10    4-(r0)  20 = aref        n/c
    16 = 11    4-(r1)  19 = 3v3         n/c
    17 = 12    4-(r2)  18 = onboard led
       ____________________
          [usb connector]

  (program with arduino micro, atmega328p, avrispmk2)

  Receives from software serial and sends to hardware serial.
  Note: the following pins on the Micro support change interrupts so for RX:
  8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI), 17 (SS)

  Pin-out DM/SN7441AN Nixi tube driver:
    [<- input from Arduino], [() output to tube]
    _________________
    1 = (0)   16 = (8)
    2 = (1)   15 = (9)
    3 = (5)   14 = A <-
    4 = (4)   13 = D <-
    5 = GND   12 = Vin 5v, 105mW typical, 105/5=21mA
    6 = (6)   11 = B <-
    7 = (7)   10 = C <-
    8 = (3)    9 = (2)
    _________________
  (Driver 4 got a spark to much so its replaced by 3 relais)

  Pin-out BN-220 GPS:
    1 = GND ( Black wire )
    2 = TX  ( White wire )
    3 = RX  ( Green wire )
    4 = VCC ( Red wire, (3 -) 5v DC)
    - TX led (blue) flashing = data output
    - PPS led (red) flashing when fixed

 The GPS unit is a u-blox m8030KT chip.
 GPS NMEA message overview:
    Positioning systems  Messages
    GPS                  $GGA = gps fix data
    Glonass              $GSA = gnss dop and active satellites
    galileo              $GLL(0xF0 0x00) = lat, lon, time of position and status
    beidou               $GSV = gnss satellites in view
    qzss                 $RMC = recommended minimum data
    sbas                 $VTG = course over ground speed

  More details can be found in the documentation
*/

#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 17

#define driver1PinA A0
#define driver1PinB A2
#define driver1PinC A3
#define driver1PinD A1

#define driver2PinA A4
#define driver2PinB 8
#define driver2PinC 9
#define driver2PinD A5

#define driver3PinA 7
#define driver3PinB 5
#define driver3PinC 4
#define driver3PinD 6

#define driver4Relais0 10
#define driver4Relais1 11
#define driver4Relais2 12

SoftwareSerial softSerial(rxPin, txPin);

void WriteDecimalToOutput(char decimal, uint8_t driver, bool debug = false){

  /*
    ASCII translation table:
    Char  ASCII    Binary
    0     chr(48)  0011 0000
    1     chr(49)  0011 0001
    2     chr(50)  0011 0010
    3     chr(51)  0011 0011
    4     chr(52)  0011 0100
    5     chr(53)  0011 0101
    6     chr(54)  0011 0110
    7     chr(55)  0011 0111
    8     chr(56)  0011 1000
    9     chr(57)  0011 1001
                   (Only the first 4 differ)
    The driver has 4 inputs (A,B,C,D).
    These inputs are based on the BDC encoding.
  */

  if (debug) { Serial.print("Driver : ");Serial.println(driver);  }
  if (debug) { Serial.print("Decimal: ");Serial.println(decimal); }

  /*
    DDRD = B11111110;  // sets Arduino pins 1 to 7 as outputs, pin 0 as input
    DDRD = DDRD | B11111100;  // this is safer as it sets pins 2 to 7 as outputs
  	                            // without changing the value of pins 0 & 1, which are RX & TX
                                // bitmasking...
     -DDR-makes pins input or output
     -PORT-makes pin LOW or HIGH
      B (digital pin 8 to 13)
      C (analog input pins)
      D (digital pins 0 to 7)

     char1 << 4; = 4 bits opschuiven
     dan char2 | met B00001111
     dan char1 & char 2 = uren in een keer...
     & of | testen...
    */

  switch(driver){
    case 1:
      digitalWrite(driver1PinA, (bitRead( decimal, 0) ));
      digitalWrite(driver1PinB, (bitRead( decimal, 1) ));
      digitalWrite(driver1PinC, (bitRead( decimal, 2) ));
      digitalWrite(driver1PinD, (bitRead( decimal, 3) ));
      break;

    case 2:
      digitalWrite(driver2PinA, (bitRead( decimal, 0) ));
      digitalWrite(driver2PinB, (bitRead( decimal, 1) ));
      digitalWrite(driver2PinC, (bitRead( decimal, 2) ));
      digitalWrite(driver2PinD, (bitRead( decimal, 3) ));
      break;

    case 3:
      digitalWrite(driver3PinA, (bitRead( decimal, 0) ));
      digitalWrite(driver3PinB, (bitRead( decimal, 1) ));
      digitalWrite(driver3PinC, (bitRead( decimal, 2) ));
      digitalWrite(driver3PinD, (bitRead( decimal, 3) ));
      break;

    case 4:
      digitalWrite(driver4Relais0, LOW);
      digitalWrite(driver4Relais1, LOW);
      digitalWrite(driver4Relais2, LOW);

      if ( (decimal = 0) ) digitalWrite(driver4Relais0, HIGH);
      if ( (decimal = 1) ) digitalWrite(driver4Relais0, HIGH);
      if ( (decimal = 2) ) digitalWrite(driver4Relais0, HIGH);
      break;

    default:
      break;
  }
}

void TestNixiTubes(bool debug = false){
  // first the drivers
  for (int i = 0; i < 10; i++) {
    for (int j = 1; j < 4; j++) {
      WriteDecimalToOutput(char(i), j, debug); delay(500);
    }
  }

  // then the relais
  digitalWrite(driver4Relais0, HIGH); delay(500); digitalWrite(driver4Relais0, LOW);
  digitalWrite(driver4Relais1, HIGH); delay(500); digitalWrite(driver4Relais1, LOW);
  digitalWrite(driver4Relais2, HIGH); delay(500); digitalWrite(driver4Relais2, LOW);
}

void ResetNixitubesToZero(){
  for (int j = 1; j < 5; j++) WriteDecimalToOutput(0, j);
}

// default Arduino call, runs only once
void setup() {

  // set a bool to debug the program
  bool debug = false;

  if (debug) {
    Serial.begin(9600);
    while (!Serial) { ; }
    Serial.print("Init: ");
  }

  // setup connection to GPS module
  if (debug) Serial.print("1, ");

  softSerial.begin(9600);

  /* needed?
  pinMode(A0, INPUT_PULLUP); // pullup to match digital pins
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  */

  if (debug) Serial.print("2, ");
  pinMode(driver1PinA, OUTPUT); digitalWrite(driver1PinA, LOW);
  pinMode(driver1PinB, OUTPUT); digitalWrite(driver1PinB, LOW);
  pinMode(driver1PinC, OUTPUT); digitalWrite(driver1PinC, LOW);
  pinMode(driver1PinD, OUTPUT); digitalWrite(driver1PinD, LOW);

  pinMode(driver2PinA, OUTPUT); digitalWrite(driver2PinA, LOW);
  pinMode(driver2PinD, OUTPUT); digitalWrite(driver2PinD, LOW);
  pinMode(driver2PinB, OUTPUT); digitalWrite(driver2PinB, LOW);
  pinMode(driver2PinC, OUTPUT); digitalWrite(driver2PinC, LOW);

  pinMode(driver3PinA, OUTPUT); digitalWrite(driver3PinA, LOW);
  pinMode(driver3PinD, OUTPUT); digitalWrite(driver3PinD, LOW);
  pinMode(driver3PinB, OUTPUT); digitalWrite(driver3PinB, LOW);
  pinMode(driver3PinC, OUTPUT); digitalWrite(driver3PinC, LOW);

  pinMode(driver4Relais0, OUTPUT); digitalWrite(driver4Relais0, LOW);
  pinMode(driver4Relais1, OUTPUT); digitalWrite(driver4Relais1, LOW);
  pinMode(driver4Relais2, OUTPUT); digitalWrite(driver4Relais2, LOW);


  if (debug) Serial.print("3, ");
  TestNixiTubes();
  delay(5000);

  if (debug) Serial.print("4, ");
  ResetNixitubesToZero();
  delay(25);

  if (debug) { Serial.println("done."); Serial.println("Starting main program:"); }

  delay(500);
}

// default Arduino Call, never ends
void loop() {

  bool debug = false;

  if (softSerial.available() ){
    //Serial.print(softSerial.read());
    String data = softSerial.readStringUntil('\n'); // message sepparation with '\n'
    /*
    Serial output from NMEA unit looks like this:
    $GNGSA,A,3,12,02,06,24,,,,,,,,,6.68,6.01,2.91*18(\n)
    $GNGSA,A,3,,,,,,,,,,,,,6.68,6.01,2.91*19(\n)
    $GPGSV,3,1,10,02,36,083,27,03,03,339,,06,23,046,16,12,52,082,22*76(\n)
    $GPGSV,3,2,10,14,36,275,,24,18,147,26,25,83,300,,29,48,201,*7B(\n)
    $GPGSV,3,3,10,31,32,305,,32,30,248,*71(\n)
    $GLGSV,1,1,00*65(\n)
    $GNGLL,5319.31241,N,00559.42548,E,204453.00,A,A*7E(\n)
    $GNRMC,204454.00,A,5319.31067,N,00559.43172,E,1.581,,060118,,,A*69(\n)
           (char 7 to 10 has the hours an minutes)
    $GNVTG,,T,,M,1.581,N,2.928,K,A*31(\n)
    $GNGGA,204454.00,5319.31067,N,00559.43172,E,1,04,6.01,-16.0,M,45.6,M,,*58(\n)
    */

    if(data.substring(0, 6) == "$GNRMC"){

      // implementation of the TimeZone difference
      char digitOne = data.charAt(7);
      char digitTwo = data.charAt(8);
      int utcOffset = 1; // time +1 for Holland
      int hours     = ( (digitOne * 10) + digitTwo ) + utcOffset;

      // split the decimals of hour
      uint8_t count = 0;
      while (hours > 0) {
          int digit = hours % 10;

          // store the sepparate digits during runtime
          if (count == 0) digitOne = digit;
          if (count == 1) digitTwo = digit;

          hours /= 10;
          count++;
      }

      // write the  4 decimals(chars) to the drivers
      WriteDecimalToOutput( digitOne         , 1, debug);
      WriteDecimalToOutput( digitTwo         , 2, debug);
      WriteDecimalToOutput( data.charAt(  9 ), 3, debug);
      WriteDecimalToOutput( data.charAt( 10 ), 4, debug);

      if (debug) Serial.println(data);
    }
    // stop running code for about 2.5 seconds, sort of low power
    delay(2500);
  }
}
