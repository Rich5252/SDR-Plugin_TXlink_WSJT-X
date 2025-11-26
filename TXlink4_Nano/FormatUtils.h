#ifndef FORMAT_UTILS_H
#define FORMAT_UTILS_H

#include <Arduino.h>

const int OUTPUT_BUFFER_SIZE = 25; 
char outputBuffer[OUTPUT_BUFFER_SIZE];      //for output print messages to TxLink etc

// --------------------------------------------------------------------------
// STEP 1: DEFINE THE GENERIC TEMPLATE FIRST (Crucial for template specialization syntax)
// --------------------------------------------------------------------------
template <typename T>
void formatForSerial(
    const char* prefix, 
    T value, 
    int decimalDigits, 
    const char* suffix, 
    char* buffer, 
    size_t bufferSize) 
{
    // ... (Full function body for integers/generic types) ...
    char numberStr[15]; 
    snprintf(numberStr, sizeof(numberStr), "%ld", (long)value);
    const char* startOfNumber = numberStr;
    if (*startOfNumber == ' ') {
        startOfNumber++;
    }
    snprintf(buffer, bufferSize, "%s%s%s", prefix, startOfNumber, suffix); 
}

// --------------------------------------------------------------------------
// STEP 2: DEFINE EXPLICIT SPECIALIZATIONS
// --------------------------------------------------------------------------

// --- EXPLICIT SPECIALIZATION FOR FLOAT ---
template <> // Now the compiler knows what 'formatForSerial' is
void formatForSerial<float>(
    const char* prefix, 
    float value, 
    int decimalDigits, 
    const char* suffix, 
    char* buffer, 
    size_t bufferSize) 
{
    // ... (Full function body for float) ...
    char numberStr[15]; 
    dtostrf((double)value, 0, decimalDigits, numberStr);
    
    const char* startOfNumber = numberStr;
    if (*startOfNumber == ' ') {
        startOfNumber++;
    }
    snprintf(buffer, bufferSize, "%s%s%s", prefix, startOfNumber, suffix); 
}

// --- EXPLICIT SPECIALIZATION FOR DOUBLE ---
template <> 
void formatForSerial<double>(
    const char* prefix, 
    double value, 
    int decimalDigits, 
    const char* suffix, 
    char* buffer, 
    size_t bufferSize) 
{
    // ... (Full function body for double) ...
    char numberStr[15]; 
    dtostrf(value, 0, decimalDigits, numberStr);
    
    const char* startOfNumber = numberStr;
    if (*startOfNumber == ' ') {
        startOfNumber++;
    }
    snprintf(buffer, bufferSize, "%s%s%s", prefix, startOfNumber, suffix); 
}

// --------------------------------------------------------------------------
// STEP 3: DEFINE C-STRING OVERLOAD (Using inline to prevent linker errors)
// --------------------------------------------------------------------------
inline void formatForSerial(
    const char* prefix, 
    const char* value, 
    const char* suffix, 
    char* buffer, 
    size_t bufferSize) 
{
    // ... (Full function body for const char*) ...
    snprintf(
        buffer, 
        bufferSize, 
        "%s%s%s", 
        prefix, 
        value, 
        suffix
    ); 
}

#endif // FORMAT_UTILS_H
