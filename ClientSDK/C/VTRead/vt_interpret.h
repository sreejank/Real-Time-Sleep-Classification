#pragma once

#include <windows.h>
#include "../../include/plexon.h"


// possible VT data type
enum VT_Type { 
    BAD = 0, 
    X1 = 1,     // x coordinate of LED #1
    Y1 = 2,     // y coordinate of LED #1 
    X2 = 4,     // x coordinate of LED #2
    Y2 = 8,     // y coordinate of LED #2
    X3 = 16,    // x coordinate of LED #3
    Y3 = 32,    // y coordinate of LED #3
    CTX = 64,   // x coordinate of centroid
    CTY = 128,  // y coordinate of centroid
    CM = 256    // centroid motion
};


// possible VT data mode
enum VT_Mode
{
    UNKNOWN,
    CENTROID,               // 1 set of coordinates, no motion
    CENTROID_WITH_MOTION,   // 1 set of coordinates, with motion
    LED_1,                  // 1 set of coordinates
    LED_2,                  
    LED_3,
    LED_12,                 // 2 sets of coordinates
    LED_13,
    LED_23,
    LED_123,                // 3 sets of coordinates
};


// decoded VT data item
struct  VT_Data
{
    unsigned __int64 timestamp;
    VT_Type          type;
    unsigned         value;
};

// initialize VT_Data
void VT_Data_Init( VT_Data* that, const PL_Event* data );


// VT accumulator
struct VT_Acc
{
    unsigned __int64 timestamp; // timestamp of last accepted value
    unsigned         present;   // what data is present in the accumulator
                                // this is a bitset of VT_Type
    unsigned short   x1, y1, x2, y2, x3, y3, cx, cy, cm;  // accumulated values
};

// initialize VT accumulator
void    VT_Acc_Init( VT_Acc* that );

// clear VT accumulator
void    VT_Acc_Clear( VT_Acc* that );

// accept new VT data and return true, or reject it and return false
bool    VT_Acc_Accept( 
    VT_Acc*             that,
    const VT_Data*      data,               // VT data for examination
    unsigned __int64    acceptable_delay    // acceptable delay in ticks
    );

// return current VT mode
VT_Mode VT_Acc_Mode( const VT_Acc* that );

// print currently accumulated values
void    VT_Acc_Print( const VT_Acc* that );
