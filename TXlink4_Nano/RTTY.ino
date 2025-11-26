#include "RTTY.h"
#include "rtty_queue.h"

// --- RTTY Constants ---
const double RTTY_BAUD = 45.45; // Standard 60 WPM
const long BIT_DURATION_US = static_cast<long>(1000000.0 / RTTY_BAUD + 0.5); // Duration of one bit in milliseconds (~22 ms)
const long STOP_BIT_DURATION_US = static_cast<long>(BIT_DURATION_US * 2 + 0.5); // 1.5 Stop bits (~33 ms)

// FSK Tones (170 Hz Shift) based on using CW with 1k offset and MMTTY set to mark 915
const long MARK_FREQ = 170/2;       //2125; // Logic '1'
const long SPACE_FREQ = -170/2;                //2295; // Logic '0'

// Baudot/ITA2 Control Codes (Constants remain the same)
const int LETTERS_SHIFT_CODE = 31; // 11111 (Shift to Letters)
const int FIGURES_SHIFT_CODE = 27; // 11011 (Shift to Figures)
const int CR_CODE            = 8;  // 01000 (Carriage Return)
const int LF_CODE            = 2;  // 00010 (Line Feed)
const int SPACE_CODE         = 4;  // 00100 (Space)
const int BELL_CODE          = 5;  // 00101 (Bell/Strobe)
const int NULL_CODE          = 0;  // 00000 (Blank/Null)

bool current_shift_LTRS = true;

// ITA2 standard as implemented in MMTTY
BaudotEntry getBaudotCode(char c) {
    c = toupper(c); // RTTY is upper-case only

    // 1. Handle Unshifted Control Characters (Universal)
    if (c == '\r' || c == 8) return {CR_CODE, false}; // CR is 8
    if (c == '\n' || c == 2) return {LF_CODE, false}; // LF is 2
    if (c == ' ') return {SPACE_CODE, false}; // SPACE is 4

    // 2. Main Character Switch
    switch (c) {
        // ------------------------------------------------------------------
        // LETTERS SHIFT (requires_figures_shift = false)
        // ------------------------------------------------------------------
        case 'E': return {1, false};  // 00001
        case 'A': return {3, false};  // 00011
        case 'S': return {5, false};  // 00101
        case 'I': return {6, false};  // 00110
        case 'U': return {7, false};  // 00111
        case 'D': return {9, false};  // 01001
        case 'R': return {10, false}; // 01010
        case 'J': return {11, false}; // 01011
        case 'N': return {12, false}; // 01100
        case 'F': return {13, false}; // 01101
        case 'C': return {14, false}; // 01110
        case 'K': return {15, false}; // 01111
        case 'T': return {16, false}; // 10000
        case 'Z': return {17, false}; // 10001
        case 'L': return {18, false}; // 10010
        case 'W': return {19, false}; // 10011
        case 'H': return {20, false}; // 10100
        case 'Y': return {21, false}; // 10101
        case 'P': return {22, false}; // 10110
        case 'Q': return {23, false}; // 10111
        case 'O': return {24, false}; // 11000
        case 'B': return {25, false}; // 11001
        case 'G': return {26, false}; // 11010
        case 'M': return {28, false}; // 11100
        case 'X': return {29, false}; // 11101
        case 'V': return {30, false}; // 11110
        
        // ------------------------------------------------------------------
        // FIGURES SHIFT (requires_figures_shift = true)
        // ------------------------------------------------------------------
        case '3': return {1, true};  // Corresponds to E
        case '-': return {3, true};  // Corresponds to A (Dash)
        //case '"': return {5, true};  // Corresponds to S
        case '8': return {6, true};  // Corresponds to I
        case '7': return {7, true};  // Corresponds to U
        case '$': return {9, true};  // Corresponds to D (Using $ for "who are you?/$")
        case '4': return {10, true}; // Corresponds to R
        case '\'': return {11, true}; // Corresponds to J (Apostrophe/Single Quote)
        case ',': return {12, true}; // Corresponds to N
        case '!': return {13, true}; // Corresponds to F
        case ':': return {14, true}; // Corresponds to C
        case '(': return {15, true}; // Corresponds to K
        case '5': return {16, true}; // Corresponds to T
        case '"': return {17, true}; // Corresponds to Z
        case ')': return {18, true}; // Corresponds to L
        case '2': return {19, true}; // Corresponds to W
        //case '#': return {20, true}; // Corresponds to H (Using # for £/#)
        case '6': return {21, true}; // Corresponds to Y
        case '0': return {22, true}; // Corresponds to P
        case '1': return {23, true}; // Corresponds to Q
        case '9': return {24, true}; // Corresponds to O
        case '?': return {25, true}; // Corresponds to B
        case '&': return {26, true}; // Corresponds to G
        case '.': return {28, true}; // Corresponds to M
        case '/': return {29, true}; // Corresponds to X
        case ';': return {30, true}; // Corresponds to V
        // Code 27 (11011) and 31 (11111) are explicitly used for shifting and are not printable.
        
        case 0:
            return {NULL_CODE, false}; // Handles input of ASCII 0 / NUL
            
        default: 
            // Fallback for unmapped characters.
            return {SPACE_CODE, false}; 
    }
}

// -------------------------------------------------------------
// Core RTTY Modulation Function
// -------------------------------------------------------------

// Function to transmit a single bit (Mark or Space)
void transmitBit(int bit, long uSecNextBitEnd) {
    long nextToneFreqHz = 0;
    static bool bServiceSerial = true;

    if (bit == 1) {
        nextToneFreqHz = CurFreq + MARK_FREQ;
    }
    else {
        nextToneFreqHz = CurFreq + SPACE_FREQ;
    }
    
    dds.setFrequency(nextToneFreqHz, PowerUp); 
    
    // Use delay() for the precise bit duration
    long d = uSecNextBitEnd - micros();
    while (d > 0) {
        d = uSecNextBitEnd - micros();
        if (d >5000) {          //make sure there is enough time
            if (bServiceSerial) {
                ProcessSerialIn();
            }
            else {
                SWRmeasure();
            }
            bServiceSerial = !bServiceSerial;
        }
    }
}

// Function to transmit a full 5-bit Baudot code (with Start/Stop bits)
void transmitBaudotCode(int code) {
    long uSecNextBitEnd = micros();

    // 1. Start Bit: Space (0) - 1 bit duration
    transmitBit(0, uSecNextBitEnd += BIT_DURATION_US); 

    // 2. 5 Data Bits (Least Significant Bit first - LSB)
    for (int i = 0; i < 5; ++i) {
        int bit = (code >> i) & 0x01;
        transmitBit(bit, uSecNextBitEnd += BIT_DURATION_US);
    }

    // 3. Stop Bits: Mark (1) - 1.5 bit duration
    // We send a Mark tone for the total required stop bit time
    transmitBit(1, uSecNextBitEnd += STOP_BIT_DURATION_US); 
}

// -------------------------------------------------------------
// Main RTTY Transmission Logic
// -------------------------------------------------------------

void transmitRttyString() {
// get text from text queue and send until queue is empty
    char* text = dequeueMessage();
    //Serial.print("dequeueMessage for tx - "); Serial.print(*text);Serial.print(" ");Serial.println(text);
    if (text != NULL) {   
        //send some preamble to sync receiver
        transmitBaudotCode(FIGURES_SHIFT_CODE);
        transmitBaudotCode(LETTERS_SHIFT_CODE);
        transmitBaudotCode(FIGURES_SHIFT_CODE);
        transmitBaudotCode(LETTERS_SHIFT_CODE);
        current_shift_LTRS = true;
        //Serial.println("LTRS");
        //and a new line to make message reception clearer
        transmitBaudotCode(CR_CODE);
        transmitBaudotCode(LF_CODE);

        while (text != NULL) {
            while (*text && DigiTxOn) {     
                char c = *text;
                BaudotEntry entry = getBaudotCode(c);

                if (entry.requires_figures_shift && current_shift_LTRS) {
                    // Need to send FIGURES SHIFT
                    transmitBaudotCode(FIGURES_SHIFT_CODE);
                    current_shift_LTRS = false;
                    //Serial.println("FIGS");
                } else if (!entry.requires_figures_shift  && !current_shift_LTRS) {
                    // Need to send LETTERS SHIFT
                    transmitBaudotCode(LETTERS_SHIFT_CODE);
                    current_shift_LTRS = true;
                    //Serial.println("LTRS");
                }
                // --- Transmit Character ---
                transmitBaudotCode(entry.code);
                text++;
            }
            // here when message finishes so look for another in queue
            text = dequeueMessage();
            if (text != NULL) {
                //transmit a space between strings
                transmitBaudotCode(getBaudotCode(" ").code);
            }
        }
        transmitBaudotCode(CR_CODE);
        transmitBaudotCode(LF_CODE);

        // Ensure we end in LETTERS shift for common practice
        if (!current_shift_LTRS) {
            transmitBaudotCode(LETTERS_SHIFT_CODE);
            current_shift_LTRS = true;
        }
    }
    // Reset freq, remove RF. TX is turned off properly in the main loop
    dds.setFrequency(CurFreq - SPACE_FREQ, PowerDown);
    DigiTxOn = false;      //key up
}
