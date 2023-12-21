//  This program provides the transmitter functions that are controlled from a SDRuno plugin - TXlink
//  The Arduino drives the AD9851 DDS chip, both frequency and output amplitude.
//    CW and FT8 modes are accomodated. Arduino connects to CW key and gets FT8 messages, symbols and 
//    TX control from TXlink. In turn TXlink connects to both the SDRuno and wsjtx apps for control
//    and FT8 message sequencing. Note that the sound output of the wsjtx is not used rather the encoded 
//    tone symbols list is used directly to drive the DDS frequency shifts.
//  The Arduino also connects to an external PA providing PTT and cooling fan control.
//
//  Although originally I used the Arduino jtEncode library for digital mode coding this was limited
//    to short (13 char) messages with FT8 hence the change to use the wsjtx utilities for this.
//    These are implimented in the SDRuno plugin TXlink and send encoded symbol strings here
//      for transmission.
//  73, G4AHN

//#define debug

// library from https://github.com/cjheath/AD9851
#include <AD9851.h>
#include <SPI.h>

//SWR bridge analog inputs
#define FWDpin  5
#define REFpin  6
double  FWDavgRaw  = 0;         //average ADC readings (0 - 1023 = 0 to 5V)
double  REFavgRaw  = 0;
int     AvgCounter = 0;
int     AvgCount   = 2000;      //simple average of ADC readings

//DDS chip
#define AD9851_FQ_UD_PIN      2
#define AD9851_RESET_PIN      3
// And MOSI=11, SCK=13            //NOTE these drivers need to use SPI

class MyAD9851 : public AD9851<AD9851_RESET_PIN, AD9851_FQ_UD_PIN> {};
MyAD9851 dds;

/* DDS defaults */
#define Fs        10120000UL  /* Hz */
String  CurFreq = "10120000";

//DDS calibration
int CALIBRATION =  -16900;        /* Calibration for my test module; it runs fast */
                                  // NOTE: is now selected in SDRUno "SETT." panel
                                  // and loaded from SDTUno ini file.

//key and TX power control
int TXlevelPin = 5;   //PWM out for DDS output level
int KeyIn     =  8;   //active low
int KeyOut    =  7;   //active low
int TXout    =   9;   //active hi
int ExtPAout = 4;     //PPT on external PA, pin active high
int FanPin = 10;      // enable fan active hi

long    LastKeyUpMillis = 0;
long    KeyDownMillis = 0;
long    TXholdTime = 400;
long    FanHoldStartMillis = 0;
long    FanHoldTime = (long) 3 * 60 * 1000;      //3 minutes
bool    KeyDownActive  = false;

#define PowerDown     false
#define PowerUp       true
bool    TXPowerOn = false;      //enable DDS
bool    LocalPAactive = false;       //first power buffer amp
String strFreq = "";
int     TXlevel = 0;            //0-100% or -ve values enables DDS but no power amp
                                // PWM is 0 for full power and 255 for min. By experiment
                                // effective range is 0 to 90. Above 90 is approx zero DDS o/p
                                // note this depends on having external 15V PSU.
int     ExtPAlevel = 0;            // drive level for extra PA when enabled
bool    ExtPAon = false;

//digital mode params
#define FT8_TONE_SPACING        625          // ~6.25 Hz
#define FT8_DELAY               159          // Delay value for FT8
#define FT8_SYMBOL_COUNT        79
#define FT8_ENCODE_COUNT        77
#define FT8_BIT_COUNT           174

#define DEFAULT_MODE            MODE_FT8

bool    FT8txOn = false;
int     FT8txDF = 0;
//String  FT8txMsg = "NOCALL G4AHN";
String  FT8txMsg = "3140652002457262353553516142250015633140652505007562410457700470324420543140652";
//String  FT8txMsgErr = "ERROR G4AHN";
String  FT8txMsgErr = "3140652000026673266772757272250010533140652177703660737035675174622021543140652";

// Enumerations, only ft8 supported currently via wsjtx encoder utiliy (ft8code.exe)
enum mode {MODE_JT9, MODE_JT65, MODE_JT4, MODE_WSPR, MODE_FSQ_2, MODE_FSQ_3,
  MODE_FSQ_4_5, MODE_FSQ_6, MODE_FT8};

// Global variables
char message[] = "N0CALL AA00";
char call[] = "N0CALL";
char loc[] = "AA00";
uint8_t dbm = 27;
uint8_t tx_buffer[80];
enum mode cur_mode = DEFAULT_MODE;
uint8_t symbol_count;
uint16_t tone_delay, tone_spacing;

// Loop through the string, transmitting one character at a time.
void DigitalTXmsg()
{  
  // Reset the tone to the base frequency and turn on the output
  dds.setFrequency(CurFreq.toDouble() + FT8txDF, PowerUp);
  
  long i;
  long tStart = millis();
  for(i = 0; i < symbol_count; i++)
  {
    //next tone calc from symbols array
    dds.setFrequency((CurFreq.toInt() + FT8txDF + ((tx_buffer[i] * tone_spacing) + 50)/100), PowerUp);  //tone calcs to 2 dec places

    //output tone duration
    while ((millis() - tStart - (i * tone_delay)) < tone_delay)   //avoid accumulating time errors
    {
      SWRmeasure();
      ProcessSerialIn();
    }
    //exit when tx off message received
    if (!FT8txOn) { break; }
  }

  // Reset freq, remove RF. TX is turned off properly in the main loop
  dds.setFrequency(CurFreq.toDouble(), PowerDown);
  FT8txOn = false;      //key up
}

void set_digital_tx_buffer()
{
  // use the symbols string received from TXlink and conver to int array of tone multipliers
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 80);

  // uses wsjtx encoder output to create tx_buffer
  // FT8txMsg is loaded with symbols text (79 chars)
  String symbols = FT8txMsgErr;
  if (FT8txMsg.length() == 79)
  {
    symbols = FT8txMsg;
  }
  //Serial.println(symbols);
  for (int i=0; i < 79; i++)    //compute numeric values of symbols to multiple with tone freq.
  {
    if (symbols.charAt(i) >= '0' && symbols.charAt(i) <= '9')     //check valid number
    {
      tx_buffer[i] = symbols.charAt(i) - '0';
    }
  }
}

void setDigDefaults()
{
  // Set the mode to use
  cur_mode = MODE_FT8;

  // Set the proper frequency, tone spacing, symbol count, and
  // tone delay depending on mode
  switch(cur_mode)
  {
  case MODE_FT8:
    symbol_count = FT8_SYMBOL_COUNT; // From the library defines
    tone_spacing = FT8_TONE_SPACING;
    tone_delay = FT8_DELAY;
    break;
  }
}

// run setup code
void setup() {
  Serial.begin(38400);         // initializes the Serial connection @ 9600 baud

 // setup control pins
 pinMode(TXout, OUTPUT);
 digitalWrite(TXout, LOW);
 pinMode(ExtPAout, OUTPUT);
 digitalWrite(ExtPAout, LOW); 
 pinMode(KeyIn, INPUT_PULLUP);
 pinMode(KeyOut, OUTPUT);
 digitalWrite(KeyOut, HIGH);
 pinMode(FanPin, OUTPUT);
 digitalWrite(FanPin,LOW);


  dds.setClock(CALIBRATION);
  dds.setFrequency(CurFreq.toDouble(), PowerDown);    //power down
  analogWrite(TXlevelPin,  255);    //0 = max out, 255 =min

 setDigDefaults(); //fixed to FT8 currently 
}


// run loop (forever)
void loop() {
  if ((digitalRead(KeyIn) == LOW || FT8txOn) && TXPowerOn)
  {
    if (!KeyDownActive)              // need to go through TXon process?
    {
      KeyDownActive = true;
      KeyDownMillis = millis();
      if (LocalPAactive) 
      {
        digitalWrite(TXout, HIGH);    // turn on ant relay and PA power
        if (ExtPAon)
        {
          digitalWrite(ExtPAout, HIGH);     // also turn on external PA power if requested (PA PTT)
          analogWrite(FanPin,  200);        // Fan on, reduce volts (PWM) to stop whirring -:)
          FanHoldStartMillis = millis();    // hold fan on
        }
      }
      Serial.print("{M}");        //Mute RX
      delay(5);                   //a bit of time for relay and PA to power up before applying RF
    }
    digitalWrite(KeyOut, LOW);    // now pull key down to get RF
    if (FT8txOn) { DigitalTXmsg(); }    //TODO will hold up serial so maybe a problem
    
    LastKeyUpMillis = 0;          //reset key up timer
  }
  else
  {
    digitalWrite(KeyOut, HIGH);
    if (LastKeyUpMillis > 0)        //already waiting for key up delay?
    {
      // if waiting then check for timeout to take out of TX
      // this avoids switching off between morse characters and words etc.
      if ((millis() - LastKeyUpMillis) >= TXholdTime)  //check to turn TX off
      {
        KeyDownActive = false;
        Serial.print("{N}");            //RX off Mute
        digitalWrite(ExtPAout, LOW);    //turn off external PA (may not of been on of course)
        digitalWrite(TXout, LOW);       //turn off RF
        LastKeyUpMillis = 0;

        FanHoldStartMillis = millis();  // hold fan on (if on) until fan hold time expires
      }
    }
    else      //new key high event
    {
      if (KeyDownActive)
      {
        LastKeyUpMillis = millis();     // start count before TX turned off
      }
    }
  }

  if ((millis() - FanHoldStartMillis) >= FanHoldTime)     //check to turn fan off
  {
    digitalWrite(FanPin, LOW);                     // Fan off
  }

  //SWR readings
  SWRmeasure();

  ProcessSerialIn();        //inputs from SDR UNO user interface
}

void SWRmeasure()
{
    //SWR readings
  //average n readings and send to SDRUno TXlink plugin
  FWDavgRaw = FWDavgRaw + analogRead(FWDpin);
  REFavgRaw = REFavgRaw + analogRead(REFpin);
  AvgCounter++;
  if (AvgCounter >= AvgCount)
  {
    SendSWR();
  }
}


void SendSWR()
{
     FWDavgRaw = FWDavgRaw / AvgCount;
    if (FWDavgRaw > 0 && FWDavgRaw < 100)                           // linearise low readings due to diode knee
    {
      FWDavgRaw = FWDavgRaw * 12.549 * pow(FWDavgRaw, -0.555);      //derived from measured data
    }
    FWDavgRaw = FWDavgRaw / 1023 * 5;                   //convert to voltage measured
                                                        // which is 9.32% of output volts rms
    FWDavgRaw = pow(FWDavgRaw / 0.0932, 2) / 50;        // convert this to actual W, 9.32% is bridge sensitivity

    REFavgRaw = REFavgRaw / AvgCount;
    if (REFavgRaw > 0 && REFavgRaw < 100)                           // linearise low readings due to diode knee
    {
      REFavgRaw = REFavgRaw * 12.549 * pow(REFavgRaw, -0.555);
    }
    REFavgRaw = REFavgRaw / 1023 * 5;
    REFavgRaw = pow(REFavgRaw / 0.0932 , 2) / 50;
    
    String msg = "{F" + String(FWDavgRaw,1) + "}";
    Serial.print(msg);

    msg = "{R" + String(REFavgRaw,2) + "}";
    Serial.print(msg);

    double SWR = 0;
    if ((FWDavgRaw - REFavgRaw) > 0)
    {
      FWDavgRaw = sqrt(FWDavgRaw);
      REFavgRaw = sqrt(REFavgRaw);
      SWR = (FWDavgRaw + REFavgRaw) / (FWDavgRaw - REFavgRaw);
    }
    msg = "{S" + String(SWR,1) + "}";
    Serial.print(msg);
    
    //start new average
    AvgCounter = 0;
    FWDavgRaw = 0;
    REFavgRaw = 0;
}


void ProcessSerialIn() {
  // Serial commands from PC TXlink
  // {Bn}     Ext PA fan hold time minutes
  // {Cn}     DDS Frequency calibration  
  // {D}      Power Down (TX off)
  // {Fn}     TX frequency
  // {Hn}     TX key up hold time ms
  // {L%}     Local PA level (QRP)
  // {P%)     External PA power level
  // {Q0/1}   External PA Off/On
  // {U}      Power Up (TX On)
  // {X0/1)   FT8 TX on/off
  // {Yn}     FT8 TXDF (delta freq)
  // {Zmsg}   FT8 message
   
  char nextChar = 0;
  String nextStrChar = "";

  while (Serial.available())
  {
    nextStrChar = "";
    nextChar = Serial.read();

    nextStrChar = String(nextChar);
    if (nextStrChar == "{")
    {
      strFreq = "";
    }
    else if (nextStrChar ==  "}")
    {
      if (strFreq == "D")
      {
        dds.setFrequency(CurFreq.toDouble(), PowerDown);
        TXPowerOn = false;
      }
      else if (strFreq == "U")
      {
        dds.setFrequency(CurFreq.toDouble(), PowerUp);
        TXPowerOn = true;
        setDDSlevelPWM();         //reset current power level
      }
      else if (strFreq.startsWith("L"))
      {
        TXlevel = strFreq.substring(1).toInt();
        if (TXlevel < 0)    //diagnostic mode if negative - DDS on but no local PA
        {
          LocalPAactive = false;
        }
        else
        {
          LocalPAactive = true;
        }
        setDDSlevelPWM();        
      }
      else if (strFreq.startsWith("P"))
      {
        ExtPAlevel = strFreq.substring(1).toInt();
        setDDSlevelPWM();        
      }
      else if (strFreq.startsWith("Q"))     ///QRO mode with external PA
      {
        String strPAstate = strFreq.substring(1);
        ExtPAon = false;
        if (strPAstate == "1") { ExtPAon = true; }
        setDDSlevelPWM();        
      }
      else if (strFreq.startsWith("C"))     //freq calibartion update
      {
        CALIBRATION = strFreq.substring(1).toInt();
        dds.setClock(CALIBRATION);        
      }
      else if (strFreq.startsWith("H"))     //TX hold on time (ms)
      {
        TXholdTime = strFreq.substring(1).toInt();        
      }
      else if (strFreq.startsWith("B"))     //Fan hold in minutes (Blower)
      {
        FanHoldTime = (long) strFreq.substring(1).toInt() * 60 * 1000;        
      }
      else if (strFreq.startsWith("F"))     //TX frequency
      {
        //new frequency complete
        CurFreq = strFreq.substring(1);
        dds.setFrequency(CurFreq.toDouble(), TXPowerOn);
        strFreq = "";        
      }
      else if (strFreq.startsWith("X"))     //FT8 TX on/off
      {
        //FT8 tx status change
        FT8txOn = strFreq.substring(1).toInt();       
      }
      else if (strFreq.startsWith("Y"))     //FT8 TX freq offset
      {
        //FT8 tx status change
        FT8txDF = strFreq.substring(1).toInt();       
      }
      else if (strFreq.startsWith("Z"))     //FT8 TX message
      {
        //FT8 tx status change
        FT8txMsg = strFreq.substring(1);
        set_digital_tx_buffer();
      }
    }
    else
    {
      //add next char to string
      strFreq = strFreq + nextStrChar;
    }
  }
}


void setDDSlevelPWM()
{
    long level = TXlevel;
    if (ExtPAon)
    {
      level = ExtPAlevel;
    }
     //set level by scaling PWM to % TXlevel input
     // a -ve value of TXlevel is for diagnostics, it enables DDS but disables power amps
    if (abs(level) == 0)
    {
       analogWrite(TXlevelPin,  255);   //min o/p       
    }
    else if (abs(level) > 0 && abs(level) < 100)
    {
       analogWrite(TXlevelPin,  80 - (abs(level) * 80) / 100);    //~~80 is min out by experiment
                                                                  //so PWM range 80 -> 0 for 0->100% power
    }
    else if (abs(level) >= 100)
    {
       analogWrite(TXlevelPin,  0);     //max out
    }
}
