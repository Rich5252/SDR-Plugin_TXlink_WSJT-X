/*
 * Library for the AD9851 DDS chip
 *
 * NOTE: This version uses x10 scaling for high precision (0.1 Hz resolution) 
 * and stability up to the DDS's maximum frequency without overflow.
 */

#ifndef _AD9851_h_
#define _AD9851_h_

#include "Arduino.h"
#include <SPI.h>

#if ARDUINO < 10600
#error "Arduino 1.6.0 or later (SPI library) is required"
#endif

// First four bytes are the Frequency Tuning Word (FTW), LSB first.
// The fifth byte configures the device:
#define AD9851_ENABLE_MULTIPLIER 0x01
#define AD9851_POWER_DOWN 0x04
#define AD9851_PHASE_MASK 0xF8
#define AD9851_PHASE_SHIFT 3

template <
  uint8_t ResetPin,                      
  uint8_t FQ_UDPin,                      
  uint32_t core_clock = 180L*1000*1000, // 180MHz; core clock before calibration
  long SPIRate = 2000000,              
  uint8_t SPIClkPin = 13               
>
class AD9851 {
  uint32_t              reciprocal;             // 2^(64-shift)/calibrated_clock
  uint8_t               shift;                  // Dynamic right-shift factor
  uint32_t              current_ftw;            // Stores the last computed FTW (Frequency Tuning Word)

public:
  AD9851()
  {
    pinMode(ResetPin, OUTPUT);          
    pinMode(FQ_UDPin, OUTPUT);          
    SPI.begin();

    reset();
  }

  void reset()
  {
    pulse(ResetPin);                    
    pulse(SPIClkPin);                   
    pulse(FQ_UDPin);

    setClock(0);
    // Set initial frequency to 1 Hz (10 in x10 units)
    setFrequencyHz(1, false); 
  }

  void setClock(int32_t calibration = 0)
  {
    uint32_t calibrated_clock = core_clock * (1000000000ULL-calibration) / 1000000000ULL;
    
    // 32-bit reciprocal calculation path (Dynamic Scaling)
    uint64_t    scaled = calibrated_clock;
    for (shift = 32; shift > 0 && (scaled&0x100000000ULL) == 0; shift--)
        scaled <<= 1;                   
    reciprocal = (0x1ULL<<(32+shift)) / calibrated_clock;
  }

  
  // FTW calculation using x10 scaling for high stability (0.1 Hz resolution)
  // Input: freq_x10 (Frequency in units of 0.1 Hz)
  uint32_t frequencyFTW(uint32_t freq_x10) const
  {
    // 32-bit path: (freq_x10 * (reciprocal / 10) + rounding_term) >> shift
    return (uint32_t) (((uint64_t)freq_x10 * (reciprocal / 10) + (reciprocal/16/10)) >> shift);
  }

  void setFrequencyHz(uint32_t freq, bool PowerUp) 
  {
    current_ftw = frequencyFTW(freq * 10);    //store new base freq FTW
    setFTW(current_ftw, PowerUp);
  }

  void setFrequencyHz_x10(uint32_t freq_x10, bool PowerUp) 
  {
    current_ftw = frequencyFTW(freq_x10);     //store new base freq FTW
    setFTW(current_ftw, PowerUp);
  }

  // Set a FTW increment. Incr that is added to previously stored current_ftw base freq
  void setFrequencyIncrement(uint32_t FTWincrement, bool PowerUp)
  {
      setFTW(current_ftw + FTWincrement, PowerUp);
  }
  
  // Allows high-speed loading of a pre-calculated FTW (for FT4/FT8 tone sequencing)
  void setFTW(uint32_t ftw_word, bool PowerUp)
  {
    SPI.beginTransaction(SPISettings(SPIRate, LSBFIRST, SPI_MODE0));
    for (int b = 0; b < 4; b++, ftw_word >>= 8)
          SPI.transfer(ftw_word&0xFF); 
    // The fifth byte contains configuration settings including phase:
    if (PowerUp)
    {
        SPI.transfer(AD9851_ENABLE_MULTIPLIER);
    }
    else
    {
        SPI.transfer(AD9851_POWER_DOWN);
    }
    
    // Transfer the 40-bit control word into the DDS core
    pulse(FQ_UDPin); 
    SPI.endTransaction();
  }
  
  // New public function to retrieve the last calculated FTW
  uint32_t getCurrentFTW() const {
    return current_ftw;
  }


// Define the exact system clock frequency (180 MHz)
const uint32_t F_SYSCLK_HZ = 180000000UL; 

// Define the constant accumulator size (2^32)
const uint64_t ACCUMULATOR_SIZE = 4294967296ULL; 

// Scaling factor for the input frequency step (1,000,000 for high precision)
const uint32_t SCALE_FACTOR = 1000000UL; // 1,000,000

/**
 * @brief Calculates the FTW increment corresponding to a frequency step.
 * * WARNING: On Arduino Nano/Uno (8-bit AVR), 'double' is only 32-bit 'float'.
 * This function relies on the frequency step being very small (e.g., < 100 Hz)
 * so that the 'double' has enough precision to hold the required 6-7 digits.
 * * @param freq_step_hz The frequency difference in Hertz (e.g., 20.83333).
 * @return uint32_t The corresponding FTW increment (Delta_FTW).
 */
uint32_t calculateFTWIncrement(double freq_step_hz)
{
    // 1. Scale the input double to a large integer (e.g., 20.83333 -> 20833330)
    // This maintains the fractional precision before the 64-bit calculation.
    uint64_t freq_step_hz_scaled = (uint64_t)(freq_step_hz * (double)SCALE_FACTOR);

    // 2. Numerator: freq_step_hz_scaled * ACCUMULATOR_SIZE
    // This product (up to ~56 bits) fits safely in uint64_t.
    uint64_t numerator = freq_step_hz_scaled * ACCUMULATOR_SIZE;
    
    // 3. Denominator: f_SYSCLK * SCALE_FACTOR
    // This product (up to ~48 bits) fits safely in uint64_t.
    uint64_t denominator = (uint64_t)F_SYSCLK_HZ * SCALE_FACTOR;

    // 4. Perform the final division with explicit rounding:
    // Result = (Numerator + Denominator/2) / Denominator
    uint32_t delta_ftw = (uint32_t)((numerator + (denominator / 2)) / denominator);

    return delta_ftw;
}

protected:
  void pulse(uint8_t pin)
  {
    digitalWrite(pin, HIGH);
    digitalWrite(pin, LOW);
  }
};

#endif  /* _AD9851_h_ */