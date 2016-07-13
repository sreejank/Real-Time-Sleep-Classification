///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PlexDOSampleWin32.cpp 
//  Author: LFS 10/05, 4/06, 8/07, 12/07
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   (c) 1999-2015 Plexon Inc. Dallas Texas 75206 
//   www.plexon.com
//
//   This code is provided for users of Plexon products. If you copy, disseminate, reproduce,
//   post or archive this code, please do not remove or alter this notice or the copyright above.
//   All sample code and libraries are provided as-is and Plexon cannot be responsible for errors 
//   or consequential damage caused by its use, including user code derived from or based on this code.  
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Sample Win32 console app showing use of PlexDO API for controlling digital
//  outputs on supported National Instruments cards.  Currently supported
//  cards include:
//
//     PCI-MIO-16E-1
//     PCI-MIO-16E-4
//     PCI-6071E
//     PXI-6071E
//     PCI-6052E
//     PXI-6052E
//     PCI-6503
//     PCI-6602E
//     PXI-6602E
//     USB-6501
//
//  Contact Plexon regarding support for other NI cards.
//
//  A typical E series NIDAQ card will have at least 8 digital output bits 
//  (DIO0..DIO7) and 2 digital output lines (GPCTR0 and GPCTR1):
//
//    - DIO output bits can be set high or low, or used for medium-precision 
//      pulse output
//    - GPCTR output lines can be used for either high-precision pulse generation
//      or clock generation
//
//  The 6503 has 24 digital output bits but no digital output lines.
//
//  The 6602 has 32 digital output bits and 8 digital output lines.
//
//  The 6501 is a low-cost USB digital I/O module with 24 digital output bits 
//  but no digital output lines.
//  
//  Note that the PL_DO* functions expect bit numbers and line number to start 
//  at 1, whereas the NIDAQ hardware is labeled starting with DIO0 and GPCTR0.
//  Bit 1 in the sample code corresponds to DIO0, etc.
//
//  You must have the National Instruments NIDAQ drivers (both "Traditional" 
//  and DAQmx) installed, and at least one supported device, to run this 
//  sample code.  Note that some older Plexon MAP systems have only the Traditional
//  drivers installed; you can use the NI-MAX utility to determine what NIDAQ 
//  software is installed on your system.  See PlexDOReadme.rtf for more information
//  on how to use the NI-MAX utility.
//
//  IMPORTANT: By default, PlexDO uses the NIDAQ DAQmx drivers to control NI digital 
//  output devices; however, any device being used by the MAP is controlled by PlexDO 
//  using the Traditional NIDAQ API.  To insure correct operation, you must insure 
//  that each device has the same device number in both Traditional NIDAQ and in DAQmx.
//  For example, the card defined as device 1 in Traditional NIDAQ must have the name 
//  "Dev1" (not "1") in DAQmx.  See PlexDOReadme.rtf for more information on how to use
//  the NI-MAX utility to define NIDAQ device numbers.
//
//  See PlexDOReadme.rtf for more information, including examples.
//
//  Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//  See SampleClients.rtf for more information.  Plexon technical support is available at 
//  support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


//  To build: include PlexDO.h, link with PlexDO.lib

// include windows.h and other standard headers
#include "stdafx.h"

// the PlexDO API header
#include "../../include/PlexDO.h"


// forward declaration of test routines
void IndividualBitOutputTest(unsigned int deviceNum, unsigned int numDigitalOutputBits);
void StrobedWordOutputUsingDIOStrobeTest(unsigned int deviceNum);
void StrobedWordOutputUsingGPCTRStrobeTest(unsigned int deviceNum);
void ClockGenerationTest(unsigned int deviceNum);


// main test program
int _tmain(int argc, _TCHAR* argv[])
{
  int          numDOCards;
  unsigned int deviceNumbers[16]; 
  unsigned int numDigitalOutputBits[16];
  unsigned int numDigitalOutputLines[16];

  ////////////////////////////////////////////////////////////
  // Digital output intialization - always required 
  
  // Get info on available digital output devices
  printf("\n\nPlexDO Digital Output Sample Application\n");
  printf("========================================\n");
  printf("\ngetting info on digital output devices...\n");
  numDOCards = PL_DOGetDigitalOutputInfo(deviceNumbers, numDigitalOutputBits, 
                                         numDigitalOutputLines);
                                         
  // Dump out info on what NI devices (with digital output) are available                                         
  for (int card = 0; card < numDOCards; card++)
  {
    char deviceString[64];
    PL_DOGetDeviceString(deviceNumbers[card], deviceString);
    printf("NI devicenumber=%d, model=%s, %d DO bits, %d DO lines\n", 
            deviceNumbers[card], deviceString, numDigitalOutputBits[card], 
            numDigitalOutputLines[card]);
  }
  printf("\n");
  
  // The following examples use only one NI device.  However, PlexDO 
  // allows you to initialize and independently perform digital output from
  // multiple devices.  You must call PL_DOInitDevice on each device before 
  // it can be used in subsequent PlexDO operations.
  // IMPORTANT: Each device must be have the same device number in both
  // Traditional NIDAQ and in DAQmx, using the NI-MAX utility.
 
  // Initialization of selected digital output device
  int deviceNumIndex = 0; // use the first device returned
                          // by PL_DOGetDigitalOutputInfo 
  int deviceNum = deviceNumbers[deviceNumIndex]; // the NI device number
  printf("initializing device at NI device number %d\n", deviceNum);
  // NOTE: the second parameter (false) indicates that the selected device
  // is NOT being shared with the MAP, and therefore its GPCTR0 line
  // is available to PlexDO.  If the NI device being initialized is one 
  // that is shared with the MAP, then the second parameter passed to 
  // PL_DOInitDevice must be true.
  if (PL_DOInitDevice(deviceNum, false)) // false == not shared by MAP
  {
    printf("PL_DOInitDevice failed!\n");
    exit(-1);
  }

  ////////////////////////////////////////////////////////////
  // Each of the test routines illustrates how to use
  // a particular area of digital output functionality

  if (numDigitalOutputBits[deviceNumIndex])
  {
    // Clear all output bits (DIO) to 0
    printf("clearing all output bits\n\n");
    if (PL_DOClearAllBits(deviceNum))
    {
      printf("PL_DOClearAllBits failed!\n");
      exit(-1);
    }
  
    // Set individual output bits (DIO) using
    // PL_DOSetBit, PL_DOClearBit, PL_DOPulseBit
    IndividualBitOutputTest(deviceNum, numDigitalOutputBits[deviceNumIndex]);
  
    // Output 7-bit strobed word values, using DIO7 as the strobe bit
    PL_DOClearAllBits(deviceNum);
    StrobedWordOutputUsingDIOStrobeTest(deviceNum);
  }
  
  if (numDigitalOutputLines[deviceNumIndex])
  {
    // Output 8-bit strobed word values, using GPCTR1 as the strobe bit
    PL_DOClearAllBits(deviceNum);
    StrobedWordOutputUsingGPCTRStrobeTest(deviceNum);

    // Output a clock signal on GPCTR1
    ClockGenerationTest(deviceNum);
  }
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void IndividualBitOutputTest(unsigned int deviceNum, unsigned int numDigitalOutputBits)
{
  // Examples of setting individual output bits 
  // using PL_DOSetBit, PL_DOClearBit, PL_DOPulseBit

  // turn each bit on for 1 sec, then off for 1 sec
  printf("toggling bits 0 through %d (high for 1 sec each)\n\n", numDigitalOutputBits-1);
  for (unsigned int bit = 1; bit <= numDigitalOutputBits; bit++)
  {
    printf("turning bit %d on\n", bit);
    if (PL_DOSetBit(deviceNum, bit))
    {
      printf("PL_DOSetBit failed!\n");
      exit(-1);
    }
    PL_Sleep(1000); // set bit high for 1 sec
    printf("turning bit %d off\n", bit);
    if (PL_DOClearBit(deviceNum, bit))
    {
      printf("PL_DOClearBit failed!\n");
      exit(-1);
    }
    PL_Sleep(1000); // set bit low for 1 sec
    printf("\n");
  }
  printf("\n");
  
  // pulse each bit for 1 msec, with a 1 sec delay between pulses
  printf("pulsing bits 0 through %d (high for 1 msec each, 1 sec delay)\n", numDigitalOutputBits-1);
  for (unsigned int bit = 1; bit <= numDigitalOutputBits; bit++)
  {
    printf("pulsing bit %d\n", bit);
    if (PL_DOPulseBit(deviceNum, bit, 1)) // 1 ms pulse width
    {
      printf("PL_DOPulseBit failed!\n");
      exit(-1);
    }
    PL_Sleep(1000); // wait 1 sec between 1 ms pulses
  }

  printf("\n\n");
}
  
/////////////////////////////////////////////////////
  
void StrobedWordOutputUsingDIOStrobeTest(unsigned int deviceNum)
{
  // This example uses DIO7 as the strobe line.  It is the simplest way
  // to output a strobed word, but since a DIO line is used for the strobe
  // pulse, one less bit is available for the word value itself.  On most
  // E series devices, there are eight DIO lines, DIO0..DIO7, so using DIO7
  // for the strobe leaves DIO0..DIO6 available for a 7 bit word value.
  // In addition, if a very short (< 1 msec) strobe pulse is required, then
  // a GPCTR output line should be used for the strobe pulse, not a DIO.
    
  int strobeBit = 8; // bit 8 = DIO7
  
  // Output a series of 7 bit strobed words with values from 0..127,
  // with approximately 1 sec delay after each word.
  printf("\noutputting 7 bit strobed words from 0 to 127\n");
  for (unsigned int i = 0; i < 127; i++)
  {
    printf("strobed word = 0x%2X\n", i);
    // set the value of the 7 bit word on DIO0..DIO6
    PL_DOSetWord(deviceNum, 1, 7, i); // bits 1 through 7 (lowest 7 bits of i are used)
    // output the strobe pulse on DIO7
    PL_DOPulseBit(deviceNum, strobeBit, 1); // 1 msec pulse on DIO7
    PL_Sleep(1000);
  }

  printf("\n\n");
}  

/////////////////////////////////////////////////////

void StrobedWordOutputUsingGPCTRStrobeTest(unsigned int deviceNum)
{
  // This example uses GPCTR1 as the strobe line, resulting in high-precision
  // strobe pulse generation.  This also makes all DIO output bits (typically
  // at least DIO0..DIO7) available for the strobed word value itself.  However,
  // note that the first line (GPCTR0) is not available when the NIDAQ device
  // is being used by the MAP for continuous signal acquisition.
  
  // use output line 2 (GPCTR1) to generate the strobe pulse
  int pulseLine = 2;

  // set line mode to pulse generation
  printf("setting mode for line %d to pulse generation\n", pulseLine);
  if (PL_DOSetLineMode(deviceNum, pulseLine, PULSE_GEN))
  {
    printf("PL_DOSetLineMode failed!\n");
    exit(-1);
  }

  // optional: set a pulse duration of 500 microseconds
  // if this call is not made, a default pulse duration of 1 msec is used
  if (PL_DOSetPulseDuration(deviceNum, pulseLine, 500))
  {
    printf("PL_DOSetPulseDuration failed!\n");
    exit(-1);
  }
  
  // use DIO0..DIO7 as the bits of an 8 bit word
  
  // output a series of 8 bit strobed words, values from 0..255,
  // with approximately 1 sec delay after each word
  printf("\noutputting 8 bit strobed words from 0 to 255\n");
  for (unsigned int i = 0; i < 256; i++)
  {
    printf("strobed word = 0x%2X\n", i);
    // set the value of the 8 bit word
    PL_DOSetWord(deviceNum, 1, 8, i); // bits 1 through 8 (low 8 bits of i are used)
    // generate the strobe pulse
    PL_DOOutputPulse(deviceNum, pulseLine);
    PL_Sleep(1000);
  }

  printf("\n\n");
}

/////////////////////////////////////////////////////

void ClockGenerationTest(unsigned int deviceNum)
{
  printf("\n-------------------------\n\n");
  printf("precision pulse and clock output\n");
  
  int clockLine = 2; // GPCTR1
  
  // set line mode for GPCTR1 to clock generation
  printf("setting line mode for line %d to clock generation\n", clockLine);
  if (PL_DOSetLineMode(deviceNum, clockLine, CLOCK_GEN))
  {
    printf("PL_DOSetLineMode failed!\n");
    exit(-1);
  }
  
  // clock waveform: 100 usec high, 100 usec low = 5 kHz frequency
  printf("setting clock generation params for line %d to 100 usec high, 100 usec low\n", clockLine);
  if (PL_DOSetClockParams(deviceNum, clockLine, 100, 100)) 
  {
    printf("PL_DOSetClockParams failed!\n");
    exit(-1);
  }
  
  // start the clock 
  printf("starting clock on line %d\n", clockLine);
  if (PL_DOStartClock(deviceNum, clockLine))
  {
    printf("PL_DOStartClock failed!\n");
    exit(-1);
  }
  printf("5 kHz clock is now running on line %d\n", clockLine);
  
  // let clock run for 10 sec
  Sleep(10000); 
  
  // stop the clock
  printf("stopping clock on line %d\n", clockLine);
  if (PL_DOStopClock(deviceNum, clockLine))
  {
    printf("PL_DOStopClock failed!\n");
    exit(-1);
  }
  
  printf("\n\n");
}
