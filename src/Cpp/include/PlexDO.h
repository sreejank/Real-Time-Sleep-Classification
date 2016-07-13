//////////////////////////////////////////////////////////////////////////
//
// PlexDO.h - Plexon digital output API for controlling the digital 
//            output functionality of "E" series NIDAQ cards
//
// (c) 2005-2006 Plexon, Inc., Dallas, Texas
//  www.plexoninc.com
//  Author: LFS 9/2005, 4/2006
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

// General notes:

// Functions return 0 if successful, -1 if error.  Typical errors are passing 
// an out of range device number (must be in the range 1..16), bit number 
// (1..n), or line number (1..n), or calling a function before 
// PL_DOGetDigitalOutputInfo and PL_DOInitDevice have been called to 
// initialize PlexDO and the desired NIDAQ device.

// In parameters to PL_DO functions, bit and line numbers start at 
// one.  Bit and line numbers on Plexon breakout boxes also start at one.
// However, NI hardware and NI-supplied breakout boxes use zero-based
// numbering, so that PL_DO output bit 1 corresponds to NIDAQ digital output 
// DIO0 and PL_DO output line 1 corresponds to NIDAQ counter GPCTR_0. 

// See the sample code PlexDOSampleWin32.cpp for examples of how to use 
// the PL_DO functions.

//////////////////////////////////////////////////////////////////////////


// for PL_DOSetLineMode
#define PULSE_GEN (0)
#define CLOCK_GEN (1)


//////////////////////////////////////////////////////////////////////////
// Initialization and query functions

// Query function which returns info on available (and supported) digital NI 
// output devices. Return value is the number of supported NI devices which 
// were found. Caller must provide three arrays in which this function returns
// device info:
//   deviceNumbers[] returns the NI device numbers of these devices
//   numBits[] returns the number of digital output bits on each device 
//   numLines[] returns the number of digital output lines on each device
// Returns the number of cards with DO capability. Returns -1 if NIDAQ is not installed.
// Arrays are shown as [16] to emphasize the NI limit of 16 devices.
extern "C" int WINAPI PL_DOGetDigitalOutputInfo(unsigned int deviceNumbers[16], 
                                                unsigned int numBits[16], 
                                                unsigned int numLines[16]);

// returns an identifying string for the given NI device number, e.g. "PCI-6071E"
extern "C" int WINAPI PL_DOGetDeviceString(unsigned int deviceNumber, 
                                           char* deviceString);

// Must be called before any digital output is attempted on the specified 
// device. All digital output bits and lines will be set to 0.  
// isUsedByMAP == 1 indicates that the MAP is sharing this device, in which case 
// output line 0 is not be available for digital output.
extern "C" int WINAPI PL_DOInitDevice(unsigned int deviceNumber, 
                                      unsigned int isUsedByMAP);

//////////////////////////////////////////////////////////////////////////
// Digital output bit functions

// Resets all digital output bits to 0 on the given device. The bits are 
// sequentially set to 0 as quickly as possible.
extern "C" int WINAPI PL_DOClearAllBits(unsigned int deviceNumber);

// Sets a single digital output bit to 1. bitNumber is one-based. Bit 1 
// corresponds to DIO0 on most NI devices.
extern "C" int WINAPI PL_DOSetBit(unsigned int deviceNumber, 
                                  unsigned int bitNumber);

// Clears a single digital output bit to 0. bitNumber is 1-based: bit 1
// corresponds to DIO0 on E series NI devices.
extern "C" int WINAPI PL_DOClearBit(unsigned int deviceNumber, 
                                    unsigned int bitNumber);

// Pulses a single bit from low to high (0 to 1) for approximately the 
// specified duration (in milliseconds). The pulse will be at least as long 
// as specified, but the exact length and the variance of the length will 
// depend on Windows system activity. If duration is 0, the bit is pulsed 
// for as short a time as possible; this time will depend on system activity
// and the speed of the system processor.
extern "C" int WINAPI PL_DOPulseBit(unsigned int deviceNumber, 
                                    unsigned int bitNumber, 
                                    unsigned int duration);

// Sets a contiguous range of digital output bits to the specified value.
// Only the lowest (highBitNumber - lowBitNumber + 1) bits of value are used. 
// The specified bits are set sequentially as quickly as possible.
extern "C" int WINAPI PL_DOSetWord(unsigned int deviceNumber, 
                                   unsigned int lowBitNumber, 
                                   unsigned int highBitNumber, 
                                   unsigned int value);

//////////////////////////////////////////////////////////////////////////
// Digital output line functions

// Defines whether an output line is to be used for clock generation or 
// pulse generation.  mode is either PULSE_GEN or CLOCK_GEN. The default 
// mode for all lines is PULSE_GEN.  lineNumber is 1-based: line 1 
// corresponds to GPCTR_0 on E series NI devices.
extern "C" int WINAPI PL_DOSetLineMode(unsigned int deviceNumber, 
                                       unsigned int lineNumber, 
                                       unsigned int mode);

// When PL_SetLineMode is used to set a line to pulse generation mode, this 
// function should be called before calling PL_DOOutputPulse on that line; 
// otherwise, a default 1 msec pulse will be output. pulseDuration is the 
// length of the pulse in microseconds. The line must have been set to pulse 
// generation mode by a previous call to PL_DOSetLineMode.
extern "C" int WINAPI PL_DOSetPulseDuration(unsigned int deviceNumber, 
                                            unsigned int lineNumber,
                                            unsigned int pulseDuration);

// Outputs a single pulse on the specified line.  Duration is as specified 
// by a previous call to PL_DOSetPulseDuration. If PL_DOSetPulseDuration has 
// not been called previously, a default pulse width of 1 msec is used.
extern "C" int WINAPI PL_DOOutputPulse(unsigned int deviceNumber, 
                                       unsigned int lineNumber);

// Specifies a clock output signal in terms of the length of the high and 
// low times of a single clock cycle. Minimum value is 1 microsecond for both 
// high and low times (i.e. clock frequency of 500 kHz), maximum value is 0.5 
// second (1 Hz). Note that this clock is free-running with respect to the MAP clock.
extern "C" int WINAPI PL_DOSetClockParams(unsigned int deviceNumber, 
                                          unsigned int lineNumber, 
                                          unsigned int microsecsHigh, 
                                          unsigned int microsecsLow);

// Starts and stops the clock output on the specified line. Frequency and duty 
// cycle are as specified by a previous call to PL_DOSetClockParams.  
// If PL_DOSetClockParams has not been called previously, a default clock with 
// frequency = 1 kHz and duty cycle = 50% is output.  
extern "C" int WINAPI PL_DOStartClock(unsigned int deviceNumber, 
                                      unsigned int lineNumber);
extern "C" int WINAPI PL_DOStopClock(unsigned int deviceNumber, 
                                     unsigned int lineNumber);

//////////////////////////////////////////////////////////////////////////
// Utility functions

// Sleeps for the specified time (in milliseconds) before returning.  
// Better timing accuracy than the Win32 Sleep() function, but note that 
// accuracy can vary, depending on system activity.  For precision pulse 
// output, a digital output line and the functions PL_DOSetPulseDuration and 
// PL_DOOutputPulse should be used, rather than timing individual pulses 
// using PL_Sleep.
extern "C" int WINAPI PL_Sleep(unsigned int millisecs);
