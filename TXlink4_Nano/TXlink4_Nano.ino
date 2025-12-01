//  This program provides the transmitter functions that are controlled from a PC SDRuno plugin - TXlink
//  The Arduino drives the AD9851 DDS chip, both its frequency and output amplitude.
//    CW, FT4, FT8 and RTTY modes are accomodated. Arduino connects to CW key and gets encoded FT4/8 messages,
//    ie symbol lists and TX control from TXlink. In turn the PC app connects to both the SDRuno and wsjtx apps
//    for control and FT4/8 message sequencing. Note that the sound output of the wsjtx is not used rather the
//    encoded tone symbol list is used directly to drive the DDS frequency shift sequence including gaussian
//    transitions.
//
//  The Arduino also connects to an external PA providing PTT and cooling fan control.
//
//  Although originally I used the Arduino jtEncode library for digital mode coding this was limited to
//    short (13 char) messages with FT8 hence the change to use the wsjtx utilities, ft8code.exe & ft4code.exe.
//    These encoders provide the full WSJTX message capabilities. They are called by the PC based SDRuno plugin,
//    TXlink, which sends the encoded symbol strings here for transmission.
//
//  In RTTY mode the symbol string {Z...} command is simply the text to send which is buffered in a simple FIFO
//    queue and then encoded and output as a simple FSK signal. The centre frequency of the signal is the CurFreq 
//    (RX/TX freq) so the mark/space are symetrical around that. The decoding is done as part of the SDR plugin
//    in the PC.
//  
//  See the ProcessSerialIn() below for the list of commands used to control this code.
//
//  Without the TxLink PC software it is easy to manually drive this Arduino code from the Arduino IDE Serial Monitor.
//  For example the following command string is enough to setup an FT8 transmission on 10m:
//    {F28074000}{L1}{U}{M8}{W14500}{Y300}
//  and then to transmit the message "NOCALL G4AHN":
//    {Z3140652002457262353553516142250015633140652505007562410457700470324420543140652}{X1}
//
//  Simply use the ft8code.exe as a windows console app to generate your own message encoded symbol lists.
//
//  73, G4AHN

//*********************  NOTE  *******************
//Need to set system serial rx buffer to 256 in:
//C:\Users\'user name'\Documents\ArduinoData\packages\arduino\hardware\avr\1.8.6\cores\arduino\HardwareSerial.h
//(same core used in IDE 2.3.6 )
//************************************************

//#define debug

// DDS chip is using library from https://github.com/cjheath/AD9851
#include "AD9851.h"       //use copy in project folder
#include <SPI.h>

#include "RTTY.h"
#include "rtty_queue.h"

#include "FormatUtils.h"      //output serial message formatting

//SWR bridge and PA temp analog inputs (used for safety aborts and reporting to SDRplugin)
#define FWDpin  5
#define REFpin  6
#define TempPin 7
double  FWDavgRaw  = 0;         //average ADC readings (0 - 1023 = 0 to 5V)
double  REFavgRaw  = 0;
double  TempAvgRaw = 0;
double  TempDeg = 0;
bool    IsRxTemp = true;
double  TempTxOffset = 0;
double  LastRxTemp = -100;
int     AvgCounter = 0;
int     AvgCount   = 650;      //simple average of ADC readings

//DDS chip
#define AD9851_FQ_UD_PIN      2
#define AD9851_RESET_PIN      3
// And MOSI=11, SCK=13            //NOTE these drivers need to use SPI

class MyAD9851 : public AD9851<AD9851_RESET_PIN, AD9851_FQ_UD_PIN> {};
MyAD9851 dds;

/* DDS defaults */
// This is the base frequency value, ie the RX / TX frequency.
// Applicable offsets are applied for each mode
#define Fs        10120000UL  /* Hz */
uint32_t  CurFreq = Fs;
long lastkHz = 0;

//DDS calibration - legacy only
//This was necessary with original clock chip on the AD9851 "development board"
//Now uses an external TCXO which is good enough not to need this.
long CALIBRATION =  0;        /* Calibration for my test module; it runs fast */
                                  // NOTE: is now selected in SDRUno "SETT." panel
                                  // and loaded from SDTUno ini file.
//piecewise linear frequency calibration correction based on -21500 base value
// each offset value is 1 Hz incements starting at DDSfirstTemp
#define _DDSfirstTemp 17
#define _DDSoffsetHz {  2, 1.9, 1.8, 1.8, 1.7, 1.6, 1.5, 1.4, 1.3, 1.3, 1.2, 1.1, 1, 1, 1, 1, 1, 1,\
                        1, 1.13, 1.25, 1.38, 1.50, 1.63, 1.75, 1.88, 2, 2.1, 2.2, 2.3, 2.4, 2.5,\
                        2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5 }

//key and TX power control (see circuit diagram)
#define TXlevelPin  5   //PWM out for DDS output level
#define KeyIn       8   //active low
#define KeyOut      7   //active low
#define TXout       9   //active hi
#define ExtPAout    4     //PPT on external PA, pin active high
#define FanPin      10      // enable fan active hi

long    LastKeyUpMillis = 0;
long    KeyDownMillis = 0;
long    TXholdTime = 400;
long    FanHoldStartMillis = 0;
int    FanLowPWM = 150;
long    FanHoldTime = (long) 3 * 60 * 1000;      //3 minutes
int    FanPWMCur = 0;
float   FanHighTemp = 38;
bool    KeyDownActive  = false;
bool    TxTempLockout = false;
float   TxMaxTemp = 54;
float   SWRfwdMin = 1;
float   SWRmax = 3;

#define PowerDown     false
#define PowerUp       true
bool    TXPowerOn = false;      //enable DDS
bool    LocalPAactive = false;  //control the built-in 3W amp
int     TXlevel = 0;            //0-100% or -ve values enables DDS but no 3W amp
                                // PWM is 0 for full power and 255 for min. By experiment
                                // effective range is 0 to 90. Above 90 is approx zero DDS o/p
                                // note this depends on having external 15V PSU.
int     ExtPAlevel = 0;         // drive level for external PA when enabled
bool    ExtPAon = false;

//digital mode params
#define FT8_TONE_SPACING                625           // 6.25 Hz
#define FT8_tone_spacing_FTWincr_x100   14913        //0.0419095Hz per bit
#define FT8_tone_duration               160           // Symbol tone duration (ms) value for FT8
#define FT8_SYMBOL_COUNT                79

//digital mode params
#define FT4_TONE_SPACING                2083          // 20.83(3333) Hz
#define FT4_tone_spacing_FTWincr_x100   49710         //0.0419095Hz per bit
#define FT4_tone_duration               48            // Symbol tone duration (ms) value for FT4
#define FT4_SYMBOL_COUNT                105

// Digital mode enumeration. "{Mx}" command sets mode = x
enum mode {MODE_RTTY = 1, MODE_FT4 = 4, MODE_FT8 = 8};
#define DEFAULT_MODE            MODE_FT8

// "ERROR G4AHN" symbol lists
// NOTE:  The Arduino Nano has limited dynamic variable memory so very careful use of String types
//          is needed because of fragmentation issues. It is not possible to handle the long symbol
//          lists as Strings. Instead char[] and literals are used where ever possible. 
#define  FT8TxMsgErr "3140652000026673266772757272250010533140652177703660737035675174622021543140652"
#define  FT4TxMsgErr  "001321033113123132233132211112310310233021123033313210212031312110023102012333201111223212010222333032010"

int     TxMode = MODE_FT8;
bool    DigiTxOn = false;       // if false then CW selected
int     FT8txDelay = 250;       //default tx delay after receiving {X1} tx start command
int     FTxTxDF = 0;            //delta freq (Hz) ie as set in WSJTX. FT4/8 output starts at CurFreq + FTxTxDF
int     FTxTxLastDF = 0;        // so can check for change mid tx
long    FTxMillisToEnd = 14999; // time left to TX in current 15 second period or 7.5 sec period for FT4
                                //    - used in AdjustStart function to decide where to start in symbol list
                                //    - like in WSJTX the start of the symbol list is skipped when TX starts late
long    TuneChk = 0;            //==0 means tune set by WSJTX, any other value is a std message
                                //    When Tune is selected the PC app sends all zero's for symbol list

//symbol lists
char  FTxTxMsg[106] = FT8TxMsgErr;    //default to "ERROR G4AHN"
bool  FTxTxMsgChanged = false;        //used to check if incoming symbol list message has changed
                                      //    required within TX loop to see if stop/restart is needed

// Global variables
uint8_t tx_buffer[106];               // this buffer holds a numerical version of encoded symbol list
                                      //    these are the required multiples of the FT4/8 step frequencies, eg 6.25 Hz for FT8
enum mode cur_mode = DEFAULT_MODE;
int8_t symbol_count;                  //number of symbols for selected mode. =-1 when RTTY
uint16_t tone_duration;               //tone_duration is time in ms for each tone
uint32_t tone_spacingHz_x100;                //spacing is freq step * 100 eg 625 for FT8 (6.25 Hz)
uint32_t tone_spacing_FTWincr_x100;   //equivalent AD5851 FTW increment for each tone, gives much more accurate tone spacing

//gaussian freq transition params. See Gassian Calcs.xlsx file for model used.
//these default values are the FT8 case. See setDigDefaults() function for FT4
uint32_t gTranSteps = 128;                       //+/- 10% tone_duration, eack step 0.5ms (tone_duration = 160ms)
uint32_t gTranMicrosPerStep = 250;              //per gaussian transition step
int gTranInterpFactor = gTranSteps / 32;    //number of steps per increment in LookUpTable (LUT)

/**
 * @brief Gaussian Pulse Shaping Lookup Table (LUT).
 * * Contains 32 steps representing the rising curve of the Gaussian function, 
 * Fractional increments scaled up by 2^16 (65536) for high-speed integer interpolation.
 * index[0] = 0 to simplify interpolation. Will scan table from 1 to 32 using
 *    an additional binary factor to interpolate between [i-1] to [i]
 * The 32 bit FTW (Frequency Tuning Word) of the AD9851 DDS is used to generate the
 *    gaussian transition steps. This provides 0.0419Hz frequency steps that can be
 *    calculated and switched up to 8 times/ms with the Arduino Nano CPU.
 * Used for FT4/FT8 FTW transition calculations.
 */
 //This table contains fractions from near zero to 1 multiplied by 2^16 so can
 //   be used to derive a fraction (multiply) of the freq transition step and be 
 //   correctly scaled using a simple right shift 16.
const uint16_t Gaussian_G_Scaled[33] PROGMEM = {
    0, 518, 705, 948, 1264, 1667, 2177, 2815, 3602, 
    4565, 5726, 7110, 8741, 10638, 12817, 15288, 18052, 
    21102, 24421, 27978, 31732, 35630, 39604, 43582, 47477, 
    51203, 54668, 57782, 60461, 62631, 64228, 65206, 65535 
};


// *******************************************************************************************
// ************   START OF CODE SECTION   ****************************************************
// *******************************************************************************************
// The following function is where the FT4 & FT8 transmission happens in response to the {X1} 
//    TX command received on the USB serial port.
// The setDigDefaults() function is called prior to this to set the various FT4/8 fixed params.
//
// Main variable inputs to this loop are:
//    * the encoded symbol list (Z....} command
//        but converted to the numeric tx_buffer[] using set_digital_tx_buffer()
//    * the CurFreq {Fx} command and the delta freq FTxTxDF {Yx} command
//    * the time to end of TX period {Wx} command. NOTE this is derived from WSJTX
//         and needs to be accurate to ensure best chance of decode by receiver.
// 
void DigitalTXmsg()
{  
  // quick exit if RTTY
  if (TxMode == MODE_RTTY)
  {
    transmitRttyString();     //checks rtty message queue and TX until empty
    return;
  }
  
  unsigned int i, n = 0;          //i is  next tone index into tx_buffer, n is number of tones sent
  uint32_t tStart = millis();

  FTxTxLastDF = FTxTxDF;        // WSJTX delta (TX) freq Hz
  FTxTxMsgChanged = false;      //its a new message

  uint32_t f0offsetHz_x100 = 0;
  if (TxMode == MODE_FT8) { f0offsetHz_x100 = tone_spacingHz_x100 / 2; }         //FT8 zero symbol tone offset

  //setup variables that track the gaussian transitions
  uint32_t baseFTW = 0;
  uint32_t lastFTW = dds.frequencyFTW(CurFreq * 10UL);      //was set to curfreq before call here
  uint32_t nextFTW = 0;
  #define ROUNDING_OFFSET  32768UL       // 2^(16-1) for rounding during division by 2^16

  // MAIN LOOP to TX each FT4/FT4 tone required from symbol list
  // Note AdjustStart calculates the best starting point in symbol list
  //    given time left in TX period.
  for(i = AdjustStart(); i < symbol_count; i++)
  {    
    //next tone calc from symbols array. NOTE:: need "long" ints here to get enough digit resolution for freq.
     baseFTW = dds.frequencyFTW((CurFreq + FTxTxDF) * 10UL);     //uses freqHz_x10 as input, this is tone zero freq
     uint32_t FTWincr = (((uint32_t) tx_buffer[i] * tone_spacing_FTWincr_x100) + 50) / 100;     //rounded FTW incr to next tone[i]
     nextFTW = baseFTW + FTWincr;
      //sp(CurFreq); sp(baseFTW); sp(lastFTW); sp(nextFTW); sp("\r\n");

    if (i == 0)
    {
      // set first output freq required
      dds.setFTW(nextFTW, PowerUp);       //first tone output
    }
    else
    {
      //gaussian transition to next tone
      uint32_t next_gStep_time = micros();
      // Gaussian LookUpTable version
      // LUT contains UINT16_t gaussian fractions values scaled up by x2^16
      // There are 32 increments in table (33 elements starting at 0, ending at 1 / 2^16 (-1))
      // Remember that freq goes up and down so take care of signs
      int32_t gTranChangeFTW = (int32_t) (nextFTW - lastFTW); //+/- Freq Transition expressed in AD9851 FTW units
  
      // top level loop scans Gaussian_G_Scaled[] LookupTable
      for (int g = 1; g <= gTranSteps / gTranInterpFactor ; g++)      //LUT[0] = 0 plus 32 increments
      {                                                               //for all steps in LUT
        //sp(gTranChangeFTW);

        //Prog mem for array to save on RAM
        uint16_t G_scaled_current = pgm_read_word(&Gaussian_G_Scaled[g]);
        uint16_t G_scaled_previous = pgm_read_word(&Gaussian_G_Scaled[g - 1]);

        // 1. Find the gaussian step size for interpolation
        uint16_t gLUTDelta = G_scaled_current - G_scaled_previous;

        // Find the step size for the interpolation (this assumes gTranInterpFactor is power of 2)
        uint16_t gLUTStepDeltaIncr = gLUTDelta / gTranInterpFactor;

        //inner loop interpolates between LUT points.
        //gTranInterpFactor is a binary number equal to the interpolation points required
        for (int h = 1; h <= gTranInterpFactor; h++)
        {                                                   //interpolate additional gTranInterpFactor steps
          // calc interpolated gaussian fraction
          uint16_t gLUTinterp = G_scaled_previous + gLUTStepDeltaIncr * h;

          // calc FTW change for this step (using 64-bit multiply and 2^16 shift to re-scale LUT fractions
          int64_t FTW_CHANGE_SCALED = (uint64_t) gLUTinterp * (int64_t) gTranChangeFTW;   //signed scaled change

          // Apply rounding and divide by 2^16 to derive FTW change for this step
          int32_t gTranFTW_delta = (int32_t)((FTW_CHANGE_SCALED + ROUNDING_OFFSET) >> 16);
          
          // new transition step frequency
          uint32_t gTranFTW = lastFTW + gTranFTW_delta;
          dds.setFTW(gTranFTW, PowerUp);   //set DDS output
        
        //calc end time for this step
         next_gStep_time += gTranMicrosPerStep;
/*
        
        Serial.print((micros()-next_gStep_time)/gTranMicrosPerStep); Serial.print(" ");
        sp(g); sp(h); sp(G_scaled_previous); sp(G_scaled_current);
        Serial.print(gLUTStepDeltaIncr); Serial.print(" ");
        Serial.print(gLUTinterp); Serial.print(" ");
        sp(gTranChangeFTW); sp((uint32_t)FTW_SCALED); sp(freeRam());
        Serial.println(gTranFTW);
*/
         //delay to next time increment
         while (micros() < next_gStep_time) {}
        }
      }      
    }
    //Serial.print(i); Serial.print(" "); Serial.print(lastFTW); Serial.print(" "); Serial.print(nextFTW); Serial.print(" "); Serial.println(freeRam());
    
    lastFTW = nextFTW;        //move on when gaussian transition complete and wait for end of tone for next change.
       
    //output tone duration and also check for {W0} abort command
    // allow for half of transition time at end of tone period. Take care of Micros to Millis convertion of int32 gTranMicrosPerStep.
    uint32_t waitFor = tone_duration - gTranSteps / (1000UL / gTranMicrosPerStep) / 2 ;     //allow for next gaussian transition time start
    if (i >= symbol_count - 1) { waitFor = tone_duration; }      //last tone so wait until its end
    while ((millis() - tStart - (n * tone_duration)) < waitFor  && FTxMillisToEnd > 0)   //avoid accumulating time errors
    {
      // do background stuff while waiting (take care this doesnt take too long!)
      SWRmeasure();
      ProcessSerialIn();

      //check if msg (symbols) or TX freq has been changed by user and adjust msg to suit time left in period
      if (FTxTxMsgChanged || FTxTxDF != FTxTxLastDF) {
        FTxMillisToEnd -= (millis() - tStart);
        i = AdjustStart();
        FTxTxMsgChanged = false;
        FTxTxLastDF = FTxTxDF;
        tStart = millis();
        n = -1;
        break;      //jump out to restart new message sequence
      }
    }
    n++; //count tones sent since tx tStart
    if (TuneChk == 0) {i = 1;}        //continue until waitfor time if Tune mode
    
    //exit when tx off message received and we reach end of period ("W0" used to force a HaltTx here)
    if (!DigiTxOn  && ((millis() - tStart) > FTxMillisToEnd) ) { break; }
  }

  // Reset freq, remove RF. TX is turned off properly in the main loop
  dds.setFrequencyHz_x10((CurFreq + FTxTxDF) * 10, PowerDown);
  DigiTxOn = false;      //key up
}


// The real time clock timing of the transmissions is critical to the success of FT4/FT8.
// If a message transmission is started late then there is an optimum strategy to still ensure
//    the best chance of the receiving station decoding the message. In brief this means that
//    the beginning of the message is ignored so tha the message ends before the end of the
//    transmission period. In the case of FT4 the "sync" also has to occur before 1 sec into period.
// See the FT4_FT8_QEX.pdf technical document for further detail regarding decoding requirements.
//
// This function takes care of these calculations.
int AdjustStart()
{
  //this checks FT4/8 message position in period and adjusts start point to give best chance of decode
  // if too late then kill it

  int iRet = 0;               //default start at first symbol
  int TimeLeft = FTxMillisToEnd;
  
  //FT8 time left defaults
                              //msg time 1264ms, period 15s
  int KillTime = 5500;        //will decode on partial msg up to this time remaining
  int TruncTime = 12650;      //minimum time left for whole msg to be sent
  if (TxMode == MODE_FT4)
  {
    TimeLeft = TimeLeft - 1460;   //msg nust end before this to comply with FT4 sync time
                                    //ie it cant run to end like FT8
    KillTime = 2020;          //msg time 5040ms, max FT4 period 7500 - 1460 = 6040ms
                              //sync has to be within 1s of start, must send at least 50% of msg
    TruncTime = 5040;         // truncate when time left less than whole msg time
  }
  
  if (TimeLeft < KillTime)
  {
    //not enough time left so don't bother and end
    iRet = symbol_count;
  }
  //there is enough time to try so what can be sent?
  //wsjyx truncates start so decode sync is in a reasonable time frame within period
  else if (TimeLeft <= TruncTime)
  {
    //not enough time for whole msg so truncate start (wsjtx does this on late msg starts)
    iRet = symbol_count - TimeLeft / tone_duration;
    
    //just in case error checks
    if (iRet < 0) {iRet = 0;}
    if (iRet > symbol_count) {iRet = symbol_count;}
  }
  return iRet;
}


// Convert string symbol list into numeric values used to multiply with tone spacing value.
// Encoded tone symbol list is derived using the WSJTX utilities ft4code.exe and ft8code.exe
//    and are sent here using the {Z....} command by the accompanying PC code.
//
void set_digital_tx_buffer()
{
  if (TxMode == MODE_RTTY) { return; }
  
  // use the symbols string received from TXlink and conver to int array of tone multipliers
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 106);

  // uses wsjtx encoder output to create tx_buffer
  // FTxTxMsg is loaded with symbols text
  //check validity
  //  NOTE that this syntax avoids dynamic memory issues with String data types
  //    when using the fixed error defaults
  const String& symbols = (strlen(FTxTxMsg) == symbol_count) ? FTxTxMsg :
                                 (TxMode == MODE_FT8 ? FT8TxMsgErr : FT8TxMsgErr);
  
 /*
  // Diagnostic output
  Serial.println(symbols);
  Serial.println("");
  Serial.println(TxMode);
  Serial.println(symbol_count);
  Serial.println(strlen(FTxTxMsg));
  Serial.println(FTxTxMsg);
  Serial.print("symbols = ");Serial.println(symbols);
*/

  // The Tune function from WSJTX is implimented by sending all zeros in symbol list
  // check list here to detect Tune required. The symbol list is still used to TX
  //    as normal but timing is changed to TX for entire period.
  TuneChk = 0;
  for (int i=0; i < symbol_count; i++)    //compute numeric values of symbols to multiple with tone freq.
  {
    if (symbols.charAt(i) >= '0' && symbols.charAt(i) <= '9')     //check valid number
    {
      tx_buffer[i] = symbols.charAt(i) - '0';
      TuneChk = TuneChk + tx_buffer[i];         //any non zero symbols mean a normal msg (ie not Tune mode)
    }
  }
}

void setDigDefaults()
{
  // Set the proper frequency, tone spacing, symbol count, and
  // tone duration depending on mode

  switch(TxMode)
  {
  case MODE_FT8:
    symbol_count = FT8_SYMBOL_COUNT;                              // From the library defines
    tone_spacingHz_x100 = FT8_TONE_SPACING;                              //Hx * 100
    tone_spacing_FTWincr_x100 = FT8_tone_spacing_FTWincr_x100;    //149.13
    tone_duration = FT8_tone_duration;                            //160ms
    
    //gaussian freq transition params
    gTranSteps = 256;                         //MUST BE 32*n where n is gTranInterpFactor.
                                              // Applied around (+/-) each tone transition point
    gTranMicrosPerStep = 64;                  //micros per gaussian transition step min ~60
                                              // tran time ~16ms 10%
    gTranInterpFactor = gTranSteps / 32;      //number of steps per increment in LookUpTable (LUT)
                                              // ** MUST BE AN INTEGER **
    break;
  
  case MODE_FT4:
    symbol_count = FT4_SYMBOL_COUNT;                              // From the library defines
    tone_spacingHz_x100 = FT4_TONE_SPACING;                              //Hx * 100
    tone_spacing_FTWincr_x100 = FT4_tone_spacing_FTWincr_x100;    //497.10
    tone_duration = FT4_tone_duration;                            //48ms
    
    //gaussian freq transition params
    gTranSteps = 128;                         //MUST BE 32*n where n is gTranInterpFactor.
                                              // Applied around (+/-) each tone transition point
    gTranMicrosPerStep = 64;                  //micros per gaussian transition step min ~60
                                              //tran time ~8ms 16%
    gTranInterpFactor = gTranSteps / 32;      //number of steps per increment in LookUpTable (LUT)
                                              // ** MUST BE AN INTEGER **
    break;
  
  case MODE_RTTY:
    symbol_count = -1;
    break;
  }
}

// run setup code
void setup() {
  Serial.begin(115200);         // initializes the Serial connection @ 9600 baud
  while (!Serial) ;
  Serial.println("{~TX}");

   // setup control pins
   pinMode(TXout, OUTPUT);
   digitalWrite(TXout, LOW);
   pinMode(ExtPAout, OUTPUT);
   digitalWrite(ExtPAout, LOW); 
   pinMode(KeyIn, INPUT_PULLUP);
   pinMode(KeyOut, OUTPUT);
   digitalWrite(KeyOut, HIGH);
   pinMode(FanPin, OUTPUT);
   analogWrite(FanPin,255);
   delay(500);                        //needed for proper start at zero - don't know why!
   analogWrite(FanPin,FanPWMCur);

  //CalcFreqCorrection(20);       //set calibration at nominal 20deg
  
  analogWrite(TXlevelPin,  255);    //0 = max out, 255 =min
  
  setDigDefaults(); //fixed to FT8 currently

   initialiseQueue();       //for RTTY text messages
}


// run loop (forever)
void loop() {
  int fanPWM = FanPWMCur;
  if ((digitalRead(KeyIn) == LOW || DigiTxOn) && TXPowerOn)
  {
    // Transmiting or need to go to transmit
    if (!KeyDownActive)              // need to go through TXon process?
    {
      dds.setFrequencyHz(CurFreq, PowerUp);      //turn DDS on
      Serial_print("{f" + String(CurFreq) + "}");                 //msg for ATU etc to make sure correct freq selected
      formatForSerial("{f", CurFreq, 0, "}" , outputBuffer, OUTPUT_BUFFER_SIZE);
      Serial_println(outputBuffer);

      KeyDownActive = true;
      KeyDownMillis = millis();
      if (LocalPAactive) 
      {
        digitalWrite(TXout, HIGH);    // turn on ant relay and PA power
        if (ExtPAon)
        {
          digitalWrite(ExtPAout, HIGH);     // also turn on external PA power if requested (PA PTT)
          fanPWM = FanLowPWM;
          if (fanPWM < 0 || fanPWM > 255) { fanPWM = 175; }
          if (TempDeg > FanHighTemp) fanPWM = 255;
          FanPWMCur = fanPWM;
          analogWrite(FanPin,  fanPWM);        // Fan on, reduce volts (PWM) to stop whirring -:) (100% = 255)
          FanHoldStartMillis = millis();    // hold fan on
        }
      }
      Serial_println("{M}");        //Mute RX
      delay(5);                   //a bit of time for relay and PA to power up before applying RF
      if (DigiTxOn)       //tx delay
      { 
        //FT8 tx delay to help stations that have DT error
        long TxDelay = FT8txDelay;
        if (TxMode == MODE_FT4) {TxDelay = TxDelay / 2;}
        long startFT8delay = millis();
        while (millis() - startFT8delay < TxDelay)
        {
          SWRmeasure();
          ProcessSerialIn();
        }            
      }
      StartNewAverage();
    }
    digitalWrite(KeyOut, LOW);    // now pull key down to get RF using DDS level shaper
    if (DigiTxOn) { DigitalTXmsg(); }
  
    LastKeyUpMillis = 0;          //reset key up timer
  }
  else
  {
    //receive mode or need to go to receive
    digitalWrite(KeyOut, HIGH);     //RF OFF
    if (LastKeyUpMillis > 0)        //already waiting for key up delay?
    {
      // if waiting then check for timeout to take out of TX
      // this avoids switching off between morse characters and words etc.
      if ((millis() - LastKeyUpMillis) >= TXholdTime)  //check to turn TX off
      {
        KeyDownActive = false;
        Serial_println("{N}");            //RX off Mute
        digitalWrite(ExtPAout, LOW);    //turn off external PA (may not of been on of course)
        digitalWrite(TXout, LOW);       //turn off RF
        dds.setFrequencyHz(CurFreq, PowerDown);    //and dds off
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

  //Serial_println("");
  //Serial_println(FanPWMCur);
      
  // Background tasks
  SWRmeasure();             //SWR readings
  ProcessSerialIn();        //command inputs from SDR UNO user interface
}

void SWRmeasure()
{
    //SWR readings
  //average n readings and send to SDRUno TXlink plugin
  FWDavgRaw = FWDavgRaw + analogRead(FWDpin);
  REFavgRaw = REFavgRaw + analogRead(REFpin);
  TempAvgRaw = TempAvgRaw + analogRead(TempPin);
  AvgCounter++;
  if (AvgCounter >= AvgCount)
  {
    SendSWR();
  }
}

void SendSWR()
{
    bool HiReading = true;
    double minRaw = 2;
    double avgFWD = 0;
    double avgREF = 0;
    double SWR = 0;
    
    avgFWD = FWDavgRaw / AvgCount;
    if (avgFWD < minRaw)
    {
      avgFWD = 0;
    }
    else
    {
      avgFWD = avgFWD / 1023 * 5 * 0.7071;    //Vrms measured
      avgFWD = 34.232 * avgFWD + 2.2884;      // measured scaling to Vrms at 50 ohm load
    }
    avgFWD = pow(avgFWD, 2) / 50;           // convert to Watts

    HiReading = true;
    avgREF = REFavgRaw / AvgCount;
    if (avgREF < minRaw) { avgREF = 0; }
    if (avgREF > 0 && avgREF < 100)                           // linearise low readings due to diode knee
    {
      avgREF = avgREF * 12.549 * pow(avgREF, -0.555);
      HiReading = false;
    }
    avgREF = avgREF / 1023 * 5;
    avgREF = pow(avgREF / 0.0932 , 2) / 50;
    if (HiReading && avgREF > 0) {avgREF = avgREF * 0.79 + 0.59;}              //further lin scale
    if (CurFreq > 30000000) {avgREF = avgREF / 2;}               //compensate for high reading on 6m
    
    //String msg = "{F" + String(avgFWD,1) + "}";
    //Serial_print(msg);
    formatForSerial("{F", avgFWD ,1, "}", outputBuffer, OUTPUT_BUFFER_SIZE);
    Serial_print(outputBuffer);

    //msg = "{R" + String(avgREF,2) + "}";
    //Serial_print(msg);
    formatForSerial("{R", avgREF ,2, "}", outputBuffer, OUTPUT_BUFFER_SIZE);
    Serial_print(outputBuffer);

    avgFWD = sqrt(abs(avgFWD));
    avgREF = sqrt(abs(avgREF));
    double tmp = avgFWD - avgREF;
    SWR = 0;
    if (tmp > 0)
    {
      SWR = (avgFWD + avgREF) / (avgFWD - avgREF);
    }
    //msg = "{S" + String(SWR,1) + "}";
    //Serial_print(msg);
    formatForSerial("{S", SWR , 1, "}", outputBuffer, OUTPUT_BUFFER_SIZE);
    Serial_print(outputBuffer);

    //internal temp
    TempAvgRaw = calcNTC(TempAvgRaw / AvgCount);
    //TX offset adjust
    if (digitalRead(KeyOut) == LOW)    //TX is on and RF being generated
    {
      if (IsRxTemp)     //last temp was RX status
      {
        //so remember last RX temp
        LastRxTemp = TempDeg;
        IsRxTemp = false;
      }
      else              //now in tx
      {
        if (LastRxTemp > -100)        //then this is first TX temp so calc offset value
        {
          TempTxOffset = LastRxTemp - TempAvgRaw;
          LastRxTemp = -100;
        }
      }
    }
    else      //in rx
    {
      TempTxOffset = 0;
      IsRxTemp = true;
    }
    
    TempDeg = TempAvgRaw + TempTxOffset;      
    //msg = "{T" + String(TempAvgRaw) + "}";
    formatForSerial("{T", TempAvgRaw, 2, "}" , outputBuffer, OUTPUT_BUFFER_SIZE);
    Serial_println(outputBuffer);
    //if (!DigiTxOn) { CalcFreqCorrection(TempAvgRaw); }
    CheckFanState(avgFWD, SWR);

  StartNewAverage();
}

void StartNewAverage()
{
    //start new average
    AvgCounter = 0;
    FWDavgRaw = 0;
    REFavgRaw = 0;
    TempAvgRaw = 0;
}


void CheckFanState(double avgFWD, double SWR)
{
  int fanPWM = FanPWMCur;
  if (KeyDownActive)
  {
    FanPWMCur = 255;
    fanPWM = 255;
    analogWrite(FanPin, fanPWM);                     // Fan on full    
  }
  else if (FanPWMCur > 0 && (millis() - FanHoldStartMillis) >= FanHoldTime)     //check to turn fan off
  {
    FanPWMCur = 0;
    fanPWM = 0;
    analogWrite(FanPin, fanPWM);                     // Fan off
  }
  else if (FanPWMCur >= 255 && TempDeg <= FanHighTemp && !KeyDownActive)    //turn fan low if it was on high
  {
    fanPWM = FanLowPWM;
    if (fanPWM < 0 || fanPWM > 255) { FanLowPWM = 150; }    //check limits and reset if required
    analogWrite(FanPin,  FanLowPWM);        // Fan on, reduce volts (PWM) to stop whirring -:) (100% = 255)
    FanPWMCur = FanLowPWM;
  }
  else if (FanPWMCur <= FanLowPWM && TempDeg > FanHighTemp)    //turn fan high if temp high
  {
    analogWrite(FanPin,  255);      //max fan
    FanPWMCur = 255;
  }

  //tx temp and SWR safety lockout
  if (TempDeg >= TxMaxTemp || (avgFWD >= SWRfwdMin && SWR > SWRmax)) 
  {
      TxTempLockout = true;
      FTxMillisToEnd = 0;         //force digital stop
      // action like {D} power down - requires user to power up (TX On) again from UI
      dds.setFrequencyHz(CurFreq, PowerDown);
      Serial_println("{D}{N}");      //TX off in UI
      TXPowerOn = false;
      FTxMillisToEnd = 0;
      DigiTxOn = false;          //force immediate exit from Digital tx loop
  }
  if (TempDeg < FanHighTemp) { TxTempLockout = false; }
  
}


void ProcessSerialIn() {
  // Serial commands from PC TXlink
  // {Bn}     Ext PA fan hold time minutes
  // {Cn}     DDS Frequency calibration  
  // {D}      Power Down (TX off)
  // {Fn}     TX frequency
  // {Hn}     TX key up hold time ms
  // {L%}     Local PA level (QRP)
  // (Mn)     digital mode num (FT4 = 4, FT8 = 8)
  // {P%)     External PA power level
  // {Q0/1}   External PA Off/On
  // {R%}     Fan low PWM (0 to 255 max)
  // {T%}     Temp Limit
  // {U}      Power Up (TX On)
  // {V%}     FT8 TX start delay ms (default 250ms)
  // {W%}     TX FT8 watchdog - millisecs to end of this tx period
  // {X0/1)   digital mode TX on/off
  // {Yn}     FT4/8 TXDF (delta freq)
  // {Zmsg}   FT4/8 message symbol list

  //  Out to TXLink UI
  //  {M} Mute
  //  {N} Un-mute
  //  {fx} freq to atu & filter modules, x in integer Hz to 1kHz resolution 

  #define MAX_COMMAND_LEN 128 // Define max length for your serial command

  // Replace String variables with char buffers
  static char chrCommand[MAX_COMMAND_LEN];
  char nextChar = 0;

  while (Serial.available())
  {
    nextChar = Serial.read();

    if (nextChar == '{')
    {
        // Start of command: clear the buffer by setting the first char to null terminator
        chrCommand[0] = '\0';
    }
    else if (nextChar == '}')
    {
      // End of command: process the stored command
      
      // --- Single-Character Commands ---
      if (strcmp(chrCommand, "D") == 0)
      {
          dds.setFrequencyHz(CurFreq, PowerDown);
          TXPowerOn = false;
      }
      else if (strcmp(chrCommand, "U") == 0)
      {
          dds.setFrequencyHz(CurFreq, PowerDown);
          TXPowerOn = true;
          SendTunekHz(true);               //msg for ATU etc
          setDDSlevelPWM();                //reset current power level
      }
      
      // --- Multi-Character Commands (Prefix Check) ---
      else if (strncmp(chrCommand, "L", 1) == 0) // startsWith("L")
      {
          // Parse integer part starting after 'L' (index 1)
          TXlevel = atoi(&chrCommand[1]); 
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
      else if (strncmp(chrCommand, "P", 1) == 0) // startsWith("P")
      {
          ExtPAlevel = atoi(&chrCommand[1]);
          setDDSlevelPWM();        
      }
      else if (strncmp(chrCommand, "Q", 1) == 0)    ///QRO mode with external PA
      {
          // Q starts at index 0, value starts at index 1
          ExtPAon = (strcmp(&chrCommand[1], "1") == 0); 
          setDDSlevelPWM();        
      }
      else if (strncmp(chrCommand, "C", 1) == 0)    //freq calibartion update
      {
          CALIBRATION = atoi(&chrCommand[1]);
          dds.setFrequencyHz(CurFreq, PowerDown);
          dds.setClock(CALIBRATION);
          dds.setFrequencyHz(CurFreq, TXPowerOn);        
      }
      else if (strncmp(chrCommand, "H", 1) == 0)    //TX hold on time (ms)
      {
          TXholdTime = atoi(&chrCommand[1]);        
      }
      else if (strncmp(chrCommand, "B", 1) == 0)    //Fan hold in minutes (Blower)
      {
          FanHoldTime = (long) atoi(&chrCommand[1]) * 60 * 1000;        
      }
      else if (strncmp(chrCommand, "R", 1) == 0)    //Fan Low PWM
      {
          FanLowPWM = atoi(&chrCommand[1]);        
      }
      else if (strncmp(chrCommand, "T", 1) == 0)    //PA Temp Limit
      {
          TxMaxTemp = atoi(&chrCommand[1]);        
      }
      else if (strncmp(chrCommand, "F", 1) == 0)    //TX frequency
      {
        // new frequency complete. Copy the new frequency string starting at index 1.
        if (true) // Original 'if' retained for logical consistency
        {
          CurFreq = atol(&chrCommand[1]); 
          
          SendTunekHz(true);
          
          // Use the String object's built-in conversion method
          dds.setFrequencyHz(CurFreq, PowerDown);
        }

        chrCommand[0] = '\0'; // Clear buffer
      }
      else if (strncmp(chrCommand, "M", 1) == 0)    //digital TX mode
      {
          int newMode = atoi(&chrCommand[1]);
          if (newMode != TxMode) {
              TxMode = newMode;
              setDigDefaults();
          }
      }
      else if (strncmp(chrCommand, "V", 1) == 0)    //FT8 TX delay millis
      {
          FT8txDelay = atoi(&chrCommand[1]);
      }
      else if (strncmp(chrCommand, "W", 1) == 0)    //FT8 TX millis left in period
      {
          FTxMillisToEnd = atoi(&chrCommand[1]);
          if (FTxMillisToEnd == 0) { DigiTxOn = false; }
      }
      else if (strncmp(chrCommand, "X", 1) == 0)    //Digi Mode TX on/off
      {
          DigiTxOn = atoi(&chrCommand[1]);
          if (!DigiTxOn) { clearQueue(); }    //clear down any outstanding RTTY messages on {X0}
          //Serial.print(chrCommand);Serial.print("  DigiTxOn="); Serial.println(DigiTxOn);
      }
      else if (strncmp(chrCommand, "Y", 1) == 0)    //FT8 TX freq offset
      {
          FTxTxDF = atoi(&chrCommand[1]);
          //Serial.print(chrCommand);Serial.print("  FTxTxDF="); Serial.println(FTxTxDF);      
      }
      else if (strncmp(chrCommand, "Z", 1) == 0)    //FT8 TX message
      {
        // &chrCommand[1] is the C-string containing the message
        const char* msg = &chrCommand[1]; 

        if (TxMode == MODE_RTTY) {
            // Assuming enqueueMessage is also updated to take const char*
            enqueueMessage(msg);
        }
        else if (strlen(msg) == FT4_SYMBOL_COUNT || strlen(msg) == FT8_SYMBOL_COUNT) {
          if (strcmp(msg, FTxTxMsg) != 0){
            strcpy(FTxTxMsg, msg);
            FTxTxMsgChanged = true;           //flag change during transmission
          }
            
            //Serial.print("Z input = "); Serial.println(FTxTxMsg);
        }

        set_digital_tx_buffer();    //convert encoded message symbols to numeric tone multiplier array
      }
      
      // --- Special ATU Commands (Multi-string comparison) ---
      else if (strcmp(chrCommand, "+l") == 0 || strcmp(chrCommand, "-l") == 0 || 
              strcmp(chrCommand, "+c") == 0 || strcmp(chrCommand, "-c") == 0 || 
              strcmp(chrCommand, "i") == 0  || strcmp(chrCommand, "o") == 0  || 
              strcmp(chrCommand, "s") == 0  || strcmp(chrCommand, "f") == 0  || 
              strcmp(chrCommand, "?") == 0  || strcmp(chrCommand, "b") == 0)
      {
        //ATU tune messages so echo
        Serial.print('{');
        Serial.print(chrCommand);
        Serial.print('}');
      }
      
      // Reset command buffer after processing
      chrCommand[0] = '\0';
    }
    else // Accumulate characters if not '{' or '}'
    {
      // Check for buffer overflow before appending
      size_t len = strlen(chrCommand);
      if (len < MAX_COMMAND_LEN - 1)
      {
          // Append char to C-string buffer
          chrCommand[len] = nextChar;
          chrCommand[len + 1] = '\0'; // Maintain null termination
      }
    }
  }
}

int SerialAvailable()
{
  int nAvailable = Serial.available();
  if (nAvailable > 250) {Serial.println("**********  Warning - Serial Input Overflow  ***********");}
  return nAvailable;
}

bool startsWith(const char* str, const char* prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

double CheckPowerDownFreq()
{
  //on 50Mhs there is too much breakthro when in "U" mode ready for keydown
  //  so set freq 1 Mhz higher. Gets reset at keydown
  double Freq = (double)CurFreq;
  Freq > 30000000 ? Freq = Freq + 1000000 : Freq = Freq;
  return Freq;
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

double calcNTC(double AI) {
  // take the analog reading and convert to degC

  // 50k NTC pulled to 5V with 47k
  // T = 1 / ((1/T0) + (1/B)*ln(R/R0))
  // Note T in kelvin
  // T0 = 25degC, B = 3950, R0 = 50k

  float R = 51000.f * AI /(1023.f - AI);   // convert to Ohms assuming 51k pull up to 5V = 1023

  float T = 1 / ((1/298.f) + (1/3950.f) * log(R/50000.f));

  return T - 273;   //return degC
}

/*
void CalcFreqCorrection(double temp)
{

  return;
  
  double DDSfirstTemp = _DDSfirstTemp;                //array index 0
  double DDSoffsetHz[] = {0} ;     //_DDSoffsetHz;
  int DDSarrSize = sizeof(DDSoffsetHz) / sizeof(DDSoffsetHz[0]);
  
  
  //build index into offset array
  int i = (int) temp - DDSfirstTemp;
  if (i < 0) { i = 0; }
  if (i >= DDSarrSize - 2) { i = DDSarrSize - 2; }
  
  double offPerHz = (DDSoffsetHz[i+1] - DDSoffsetHz[i]);          //all elements 1 Hz apart
  long newCorrection = CALIBRATION - (long) ((DDSoffsetHz[i] + ((temp - ((double) i + DDSfirstTemp)) * offPerHz)) * 100);

  //don't change while transmitting
  if (!(KeyDownActive || DigiTxOn))
  {
    dds.setFrequencyHz(CurFreq, PowerDown);
    dds.setClock(newCorrection);
    dds.setFrequencyHz(CurFreq, PowerDown);
  } 

  //Serial_println(i);
  //Serial_println(DDSoffsetHz[i]);
  //Serial_println(newCorrection);
}
*/

void SendTunekHz(bool bSendAlways)
{
  //reduce msg rate by only sending kHz changes to ATU and filters etc
  
  long thiskHz = (CurFreq + 500) / 1000;  //round kHz
  thiskHz = thiskHz * 1000;               //back to Hz resolution
  if (thiskHz != lastkHz || bSendAlways)
  {
    //Serial.println("{f" + String(thiskHz) + "}");                 //msg for ATU etc - always send it
    formatForSerial("{f", thiskHz, 0, "}", outputBuffer, OUTPUT_BUFFER_SIZE);
    Serial.println(outputBuffer);
    lastkHz = thiskHz;
  } 
}

template <typename T>
void Serial_print(T msg)
{
  if (TXPowerOn || TXlevel != 0)
  {
    Serial.print(msg);
  }
}

template <typename T>
void sp(T msg)
{
    Serial.print(msg); Serial.print(" ");
}

template <typename T>
void Serial_println(T msg)
{
  if (TXPowerOn || TXlevel != 0)
  {
    Serial.println(msg);
  }
}


extern unsigned int __heap_start; 
extern void *__brkval;
int freeRam() {
  char top; // Local variable on the stack
  return (int)&top - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
