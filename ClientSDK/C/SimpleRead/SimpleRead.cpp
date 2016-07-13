///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   SimpleRead.cpp 
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
//   Simple console-mode app that reads data from the server 
//   and prints a count of timestamps to the console window.  
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
  PL_Event*     pServerDataBuffer;              // buffer in which the server will return acquisition data
  int           NumDataBlocks;                  // number of data blocks returned from the server
  int           NumUnsortedSpikeTimestamps;     // number of data blocks which are unsorted spike timestamps
  int           NumSortedSpikeTimestamps;       // number of data blocks which are sorted spike timestamps
  int           i;                              // loop counter

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
 
    // reset counts
    NumUnsortedSpikeTimestamps = 0;
    NumSortedSpikeTimestamps = 0; 

    // step through the array of data blocks, counting only the spike timestamps
    for (i = 0; i < NumDataBlocks; i++)
    {
      // is this data block a spike?
      if (pServerDataBuffer[i].Type == PL_SingleWFType) 
      {
        // update timestamp counts
        if (pServerDataBuffer[i].Unit >= 1) // 1,2,3,4... = a,b,c,d... units
          NumSortedSpikeTimestamps++;
        else
          NumUnsortedSpikeTimestamps++; // unsorted spikes have Unit == 0
      }

      /* optional: handle other data block types

      if (pServerDataBuffer[i].Type == PL_ExtEventType) // digital (TTL) event from DI card - single-bit 
      {
        pServerDataBuffer[i].Channel; // event channel number
        pServerDataBuffer[i].Unit;    // for strobed events, Unit holds the strobed word value
      }
      else if (pServerDataBuffer[i].Type == PL_StrobedExtChannel) // digital (TTL) event from DI card - strobed 16 bit word value
      {
      }
      else if (pServerDataBuffer[i].Type == PL_ADDataType) // continuously digitized data data
      {
      }

      */
    }

    // display the stats for this read
    if (NumSortedSpikeTimestamps + NumUnsortedSpikeTimestamps) // were there any spikes in this read?
    {
      printf("%d spikes (%d sorted, %d unsorted)\r\n", 
        NumSortedSpikeTimestamps+NumUnsortedSpikeTimestamps, NumSortedSpikeTimestamps, NumUnsortedSpikeTimestamps);
    }
    else
    {
      printf("%d data blocks, no spikes\r\n", NumDataBlocks);
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

