///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ExternalEventRead.cpp 
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
//   Simple console-mode app that reads data from the server and 
//   prints the timestamps of digital (external) events, 
//   and the values of strobed digital events, to the console window.  
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.  Plexon technical support is available at 
//   support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>

// header file containing the Plexon APIs (link with PlexClient.lib, run with PlexClient.dll)
#include "../../include/plexon.h"

// approximate time, in milliseconds, between reads from the server
#define CLIENT_POLL_TIME (250)

// maximum number of data blocks to be read at one time from the server
#define MAX_DATA_BLOCKS_PER_READ (500000)


int main(int argc, char* argv[])
{
  PL_Event*     pServerDataBuffer;  // buffer in which the server will return acquisition data
  int           NumDataBlocks;      // number of data blocks returned from the server
  int           NumExternalEvents;  // number of external events read
  double        SampleRate;         // sampling rate in Hz for spike and external event data
  int           EventChannel;       // channel on which an external event occurred
  WORD          EventExtra;         // additional external event info
  DWORD         EventTime;          // 32 bit event timestamp, in ticks (default tick interval = 25 usec (40 kHz))
  int           i;                  // loop counter

  // connect to the server
  int h = PL_InitClientEx3(0, NULL, NULL);

  // allocate memory in which the server will return data
  pServerDataBuffer = (PL_Event*)malloc(sizeof(PL_Event)*MAX_DATA_BLOCKS_PER_READ);
  if (pServerDataBuffer == NULL)
  {
    printf("Couldn't allocate memory, I can't continue!\r\n");
    Sleep(3000); // pause before console window closes
    return 0;
  }

  // get the timestamp resolution in microseconds
  switch (PL_GetTimeStampTick()) 
  {
    case 25: // 25 usec = 40 kHz, default
      SampleRate = 40000.0;
      break;
    case 40: // 40 usec = 25 kHz
      SampleRate = 25000.0;
      break;
    case 50: // 50 usec = 20 kHz
      SampleRate = 20000.0;
      break;
    default:
      printf("Unsupported sampling rate, I can't continue!\r\n");
      Sleep(3000); // pause before console window closes
      return 0;
  }

  // this loop reads from the server every CLIENT_POLL_TIME msec, until the user hits Control-C or closes the console window
  while (TRUE)
  { 
    // this tells the server the max number of data blocks we can accept in each read (every CLIENT_POLL_TIME msec)
    NumDataBlocks = MAX_DATA_BLOCKS_PER_READ;

    // call the server to get all the data blocks since the last time we called PL_GetTimeStampStructures
	  PL_GetTimeStampStructures(&NumDataBlocks, pServerDataBuffer);
 
    // check to see if the incoming data completely filled the buffer (indicating that more was available);
    // this may indicate that PL_GetTimeStampStructures should be called more frequently or that the 
    // buffer should be larger
    if (NumDataBlocks == MAX_DATA_BLOCKS_PER_READ)
      printf("(more than %d data blocks were available)\r\n", MAX_DATA_BLOCKS_PER_READ);
 
    // reset event count
    NumExternalEvents = 0;

    // step through the array of data blocks, counting only the external event timestamps
    for (i = 0; i < NumDataBlocks; i++)
    {
      // is this data block an external event?
      if (pServerDataBuffer[i].Type == PL_ExtEventType) 
      {
        EventTime = pServerDataBuffer[i].TimeStamp; 
        EventChannel = pServerDataBuffer[i].Channel; 
        EventExtra = pServerDataBuffer[i].Unit; // interpretation of Unit depends on external event type, see below
        NumExternalEvents++;

        switch (EventChannel)
        {
          case PL_StrobedExtChannel:
            // For strobed events, EventExtra contains the actual strobed word value, either 15 or 16 bits as 
            // described below.
            //
            // For OmniPlex systems, if both DI ports (A and B) are configured in strobed word mode (Mode 3 in the 
            // Digital Input device options), the high bit in the strobed word is overwritten with the value 0
            // (for events from Port A) or 1 (for events from Port B), in order to identify the source of the strobed
            // data.  If only one port is configured in Mode 3, then all 16 bits of the strobed word are available 
            // for the external event data.
            //
            // For MAP systems, the high (16th) bit is reserved to indicate whether the DI board from which this event
            // was input was on the first DSP section (high bit = 0) or on some other DSP section (high bit = 1).
            // This allows up to two strobed DI boards to be installed in a MAP while still being able to 
            // distinguish their strobed data words, but one of the two must be installed on the first DSP.
            // To extract the low 15 bits which constitute the original strobed word value, the high bit must
            // be set to zero.  Also, the "set high bit" option in the MAP Server must be checked or this bit
            // will always be 0, regardless of which DSP section the DI was on -- this is acceptable if only 
            // a single strobed DI board is installed.

            // If you are not using both DI ports/boards in strobed mode, you can skip the following test
            // of the high bit, since all 16 bits can contain external event data:
            if (EventExtra & 0x8000) // test high bit
            {
              // print with an asterisk to indicate that the high bit was 1
              printf("* Strobed external event t=%f strobed_word=%u\r\n", (double)EventTime/SampleRate, 
                     EventExtra & 0x7FFF); // mask: low 15 bits
            }
            else
            {
              printf("Strobed external event t=%f strobed_word=%u\r\n", (double)EventTime/SampleRate, EventExtra);
            }
            break;
            
          case PL_StartExtChannel:
            printf("Start-recording event t=%f\r\n", (double)EventTime/SampleRate);
            break;
            
          case PL_StopExtChannel:
            printf("Stop-recording event t=%f\r\n", (double)EventTime/SampleRate);
            break;
            
          case PL_Pause:
            printf("Pause-recording event t=%f\r\n", (double)EventTime/SampleRate);
            break;
            
          case PL_Resume:
            printf("Resume-recording event t=%f\r\n", (double)EventTime/SampleRate);
            break;
            
          default:
            // Single-bit (unstrobed) external events
            // 
            // For MAP systems, EventExtra (the Unit field of the data block) contains 
            // the number of the DSP section on which the DI board which received 
            // the event was located, since MAP DI channel numbers are port-relative.  
            //
            // For OmniPlex systems, this value will be zero, since each port has its 
            // own range of channel numbers (1-16 for port A, 17-32 for port B).
            //
            printf("Single-bit (unstrobed) external event: Extra=%d EventChan=%d t=%f\r\n", 
              EventExtra, EventChannel, (double)EventTime/SampleRate);
            break;
        }
      }
    }

    if (NumDataBlocks) // note that this count includes all data blocks, not just external events
    {
      if (NumExternalEvents)
        printf("%d external events read\r\n", NumExternalEvents);
      else
        printf("%d data blocks read\r\n", NumDataBlocks);
    }

    // yield to other programs for approximately CLIENT_POLL_TIME msec before reading data again
    Sleep(CLIENT_POLL_TIME);
  }

  // in this sample client, we will never get to this point, 
  // but this is how we would free the allocated memory and disconnect from the server

  free(pServerDataBuffer);
  PL_CloseClient();

	return 0;
}

