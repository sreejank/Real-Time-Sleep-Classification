//
//   NIDAQRead.cpp 
//
//   (c) 1999-2015 Plexon Inc. Dallas Texas 75206 
//   www.plexon.com
//
//   Simple console-mode app that reads data from the server 
//   and prints information on continuous data to the console window.  
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   NIDAQRead.cpp 
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
//   For historical reasons (MAP systems were only able to continuously acquire 
//   data from NIDAQ cards), this sample client is named NIDAQRead, and there are
//   MAP-centric references to "slow channels" and NIDAQ, but the same client API 
//   functions can be used to read continuous data from OmniPlex systems, including 
//   the WB, SPKC, FP, and AI continuous sources. Channels within OmniPlex 
//   continuous sources are assigned global channel numbers for the purposes of PLX 
//   files and the client-API as shown in PlexControl in the "PLX Chan" column of the 
//   Properties Spreadsheet for each source.
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
  PL_WaveLong*  pServerDataBuffer;   // buffer in which the server will return acquisition data
  int           NumDataBlocks;       // number of data blocks returned from the server
  double        FastSampleRate;      // sampling rate in Hz for wideband and spike data, typically 40 kHz
  int           NumContinuousSamples;// number of continuous samples in a data block
  int           ContinuousChannel;   // channel number for a continuous data block
  DWORD         SampleTime;          // timestamp of a continuous sample 
  int           Dummy;               // dummy parameter for function calls
  int           i;                   // loop counter

  // connect to the server
  int h = PL_InitClientEx3(0, NULL, NULL);

  // allocate memory in which the server will return data;
  // note that for continuous data, we need to use PL_WaveLong, as opposed
  // to PL_Event, which does not include continuous sample data
  pServerDataBuffer = (PL_WaveLong*)malloc(sizeof(PL_WaveLong)*MAX_DATA_BLOCKS_PER_READ);
  if (pServerDataBuffer == NULL)
  {
    printf("Couldn't allocate memory, I can't continue!\r\n");
    Sleep(3000); // pause before console window closes
    return 0;
  }

  // get the timestamp resolution in microseconds ("fast" sampling rate)
  switch (PL_GetTimeStampTick()) 
  {
    case 25: // 25 usec = 40 kHz, default
      FastSampleRate = 40000.0;
      break;
    case 40: // 40 usec = 25 kHz
      FastSampleRate = 25000.0;
      break;
    case 50: // 50 usec = 20 kHz
      FastSampleRate = 20000.0;
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

    // call the server to get all the data blocks since the last time we called PL_GetTimeStampStructures;
    // note that we have to use PL_GetLongWaveFormStructures, since PL_GetTimeStampStructures does not
    // return continuous sample data
    PL_GetLongWaveFormStructures(&NumDataBlocks, pServerDataBuffer, &Dummy, &Dummy); // last two parameters unused
 
    // check to see if the incoming data completely filled the buffer (indicating that more was available);
    // this may indicate that PL_GetTimeStampStructures should be called more frequently or that the 
    // buffer should be larger
    if (NumDataBlocks == MAX_DATA_BLOCKS_PER_READ)
      printf("(more than %d data blocks were available)\r\n", MAX_DATA_BLOCKS_PER_READ);
 
    // step through the array of data blocks
    for (i = 0; i < NumDataBlocks; i++)
    {
      if (pServerDataBuffer[i].Type == PL_ADDataType &&
          pServerDataBuffer[i].Channel == 1) // to reduce output, we only dump one channel
      {
        ContinuousChannel = pServerDataBuffer[i].Channel;
        NumContinuousSamples = pServerDataBuffer[i].NumberOfDataWords;
        SampleTime = pServerDataBuffer[i].TimeStamp; // timestamp of the first sample in the block
        printf("chan %d: t = %.6f\r\n", ContinuousChannel, SampleTime/FastSampleRate);
        
        break; // to reduce output, we only dump one block per read
      }    
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

