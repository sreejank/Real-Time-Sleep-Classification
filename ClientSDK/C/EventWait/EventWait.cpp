///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   EventWait.cpp 
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
//   Console-mode app that reads data from the server and counts the number of blocks of
//   each data type, printing the total number for each read to the console window.
//   Uses a Win32 synchronization object to wait on the server's polling event, rather 
//   than polling and then calling Sleep().  This minimizes the latency of data transfer
//   between server and client.  Make sure the appropriate options are set in OmniPlex Server
//   or the MAP Server to insure lowest data acquisition latency.
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

// maximum number of data blocks to be read at one time from the server
#define MAX_DATA_BLOCKS_PER_READ (500000)


int main(int argc, char* argv[])
{
  PL_Event*     pServerDataBuffer;  // buffer in which the server will return acquisition data
  int           NumDataBlocks;      // number of data blocks returned from the server
  int           NumSpikes;          // number of spikes read
  int           NumExtEvents;       // number of external events read
  int           NumContinuousBlocks;// number of blocks of continuous data read
  HANDLE        hServerPollEvent;   // handle to Win32 synchronization event
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

  // open the Win32 synchronization event used to synchronize with the server
  hServerPollEvent = OpenEvent(SYNCHRONIZE, FALSE, "PlexonServerEvent");
  if (!hServerPollEvent)
  {
    printf("Couldn't open server poll event, I can't continue!\r\n");
    Sleep(3000); //** pause before console window closes
    return 0;
  }

  // read any backlog first
  DWORD backlog = 0;
  for (;;)
  {
    NumDataBlocks = MAX_DATA_BLOCKS_PER_READ;
    PL_GetTimeStampStructures(&NumDataBlocks, pServerDataBuffer);
    backlog += NumDataBlocks;
    if (NumDataBlocks < MAX_DATA_BLOCKS_PER_READ)
      break;
  }
  printf("read %u blocks of pending data before entering main loop\r\n", backlog);

  // this loop reads from the Server, then waits until the Server event is signaled (indicating that
  // more data is available), until the user hits Control-C
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
    NumSpikes = NumExtEvents = NumContinuousBlocks = 0;

    // step through the array of data blocks
    for (i = 0; i < NumDataBlocks; i++)
    {
      // see the other sample clients for examples of how to interpret the data blocks
      switch (pServerDataBuffer[i].Type)
      {
        case PL_SingleWFType: // spike
          // process spike waveform here...
          NumSpikes++;
          break;
        case PL_ExtEventType: // digital (TTL) event from DI card - single-bit 
          // process single-bit event here...
          NumExtEvents++;
          break;
        case PL_StrobedExtChannel: // digital (TTL) event from DI card - strobed 16 bit word value
          // process strobed event word here...
          NumExtEvents++;
          break;
        case PL_ADDataType: // continuously digitized data data
          // process continuous data here...
          NumContinuousBlocks++;
          break;
        default:
          break;
      }
    }

    if (NumDataBlocks)
      printf("%d data blocks read: %d spikes, %d events, %d continuous blocks\r\n", 
        NumDataBlocks, NumSpikes, NumExtEvents, NumContinuousBlocks);

    // wait on the server event to indicate that another batch of data is ready;
    // execution continues immediately as soon as new data is available
    if (::WaitForSingleObject(hServerPollEvent, 10000) == WAIT_TIMEOUT)
    {
      printf("WaitForSingleObject timed out (10 secs) - is data acquisition running?\r\n");
      return 0;
    }
  }

  // in this sample client, we will never get to this point, 
  // but this is how we would free the allocated memory and disconnect from the server

  free(pServerDataBuffer);
  PL_CloseClient();

	return 0;
}

