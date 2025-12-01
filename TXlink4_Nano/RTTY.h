// rtty_data.h

#ifndef RTTY_DATA_H
#define RTTY_DATA_H

#include <Arduino.h>

// --- ENUM Definition (Fixes LETTERS and FIGURES not declared) ---
// The compiler needs to know these names exist before any function uses them.
//enum ShiftState { LETTERS, FIGURES };
//extern ShiftState current_shift; // Initializes RTTY state to LETTERS

// --- Struct Definition ---
struct BaudotEntry {
    int code;
    bool requires_figures_shift;
};



// --- Function Prototypes ---
BaudotEntry getBaudotCode(char c);
void transmitRttyString(const char* text); // Include this too if it's in a separate file

#endif

extern uint32_t CurFreq;
extern bool DigiTxOn;