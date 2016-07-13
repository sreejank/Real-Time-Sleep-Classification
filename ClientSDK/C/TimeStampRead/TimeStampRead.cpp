//
//   TimeStampRead.cpp 
//
//   (c) 1999-2015 Plexon Inc. Dallas Texas 75206 
//   www.plexon.com
//
//   Simple console-mode app that reads data from the server 
//   and prints the individual spike timestamps to the console window.  
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.
//

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
  PL_Event*     pServerDataBuffer;   // buffer in which the server will return acquisition data
  int           NumDataBlocks;       // number of data blocks returned from the server
  double        SampleRate;          // sampling rate in Hz for spike data
  int           SpikeChannel;        // 1-based spike channel number 
  char          SpikeUnit;           // single character 'a','b','c'... indicating sorted unit 1,2,3..., or 'u' for unsorted
  DWORD         SpikeTime;           // 32 bit spike timestamp, in ticks (default tick interval = 25 usec (40 kHz))
  int           i;                   // loop counter

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
 
    // step through the array of data blocks
    for (i = 0; i < NumDataBlocks; i++)
    {
      // is this data block a spike?
      if (pServerDataBuffer[i].Type == PL_SingleWFType)
      {
        SpikeChannel = pServerDataBuffer[i].Channel; // 1-based channel number
        if (pServerDataBuffer[i].Unit == 0)
          SpikeUnit = 'u'; // Unit 0 = unsorted
        else // Unit > 0
          SpikeUnit = 'a' + (pServerDataBuffer[i].Unit-1); // Unit 1,2,3,4... = a,b,c,d... sorted units

        // for all data types, 
        //     pServerDataBuffer[i].TimeStamp 
        // is the low 32 bits of the timestamp, in units of 25 microseconds, and
        //     pServerDataBuffer[i].UpperTS
        // is the upper 8 bits of the timestamp (note: only nonzero after lower 32 bits roll over at 29.8 hrs)        
        SpikeTime = pServerDataBuffer[i].TimeStamp; // low 32 bits of 40 bit timestamp
        printf("SPK%03d%c t=%u (%fs)\r\n", SpikeChannel, SpikeUnit, SpikeTime, (double)SpikeTime/SampleRate);
      }
    }

    if (NumDataBlocks) // note that this count includes all data blocks, not just spikes
      printf("%d data blocks read\r\n", NumDataBlocks);

    // yield to other programs for approximately CLIENT_POLL_TIME msec before reading data again
    Sleep(CLIENT_POLL_TIME);
  }

  // in this sample client, we will never get to this point, 
  // but this is how we would free the allocated memory and disconnect from the server

  free(pServerDataBuffer);
  PL_CloseClient();

	return 0;
}

