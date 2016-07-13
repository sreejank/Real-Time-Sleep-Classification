#include "vt_interpret.h"
#include <stdio.h>


// useful constants for VT data
enum VT_Data_Constants
{
    DATA_MASK = 0x03FF,
    DIBT_MASK = 0x8000,
    TYPE_MASK = 0x3C00,

    CENTROID_X = 0x0000,
    CENTROID_Y = 0x0400,
    CENTROID_MOTION = 0x1000,
    LED_X1 = 0x1400,
    LED_Y1 = 0x1800,
    LED_X2 = 0x1C00,
    LED_Y2 = 0x2000,
    LED_X3 = 0x2400,
    LED_Y3 = 0x2800
};

// initialize VT_Data
void VT_Data_Init( VT_Data* that, const PL_Event* data )
{
    that->timestamp = ( ((unsigned __int64)data->UpperTS) << 32 ) | data->TimeStamp;
    that->value = data->Unit & DATA_MASK;
    switch( data->Unit & TYPE_MASK ) {
        case CENTROID_X:      that->type = CTX; break;
        case CENTROID_Y:      that->type = CTY; break;
        case CENTROID_MOTION: that->type = CM;  break;
        case LED_X1:          that->type = X1;  break;
        case LED_Y1:          that->type = Y1;  break;
        case LED_X2:          that->type = X2;  break;
        case LED_Y2:          that->type = Y2;  break;
        case LED_X3:          that->type = X3;  break;
        case LED_Y3:          that->type = Y3;  break;
        default:              that->type = BAD; break;
    }
}


// useful constants for VT accumulator
enum VT_Acc_Constants
{
    LED_123_MASK = X1 + Y1 + X2 + Y2 + X3 + Y3,
    LED_12_MASK = X1 + Y1 + X2 + Y2,
    LED_13_MASK = X1 + Y1 + X3 + Y3,
    LED_23_MASK = X2 + Y2 + X3 + Y3,
    LED_1_MASK = X1 + Y1,
    LED_2_MASK = X2 + Y2,
    LED_3_MASK = X3 + Y3,
    CENTROID_MASK = CTX + CTY,
    CENTROID_WITH_MOTION_MASK = CTX + CTY + CM
};

// initialize VT accumulator
void VT_Acc_Init( VT_Acc* that )
{
    VT_Acc_Clear(that);
}

// clear VT accumulator
void VT_Acc_Clear( VT_Acc* that )
{
    that->present = 0;
}

// accept new VT data and return true, or reject it and return false
bool VT_Acc_Accept( VT_Acc* that, const VT_Data* data, unsigned __int64 acceptable_delay )
{
    if( that->present && ( data->timestamp < that->timestamp || 
        data->timestamp > that->timestamp + acceptable_delay ) )    return  false;
    if( data->type == BAD || ( that->present & data->type ) )       return  false;
    that->timestamp = data->timestamp;
    switch( data->type ) {
        case X1:  that->x1 = data->value; break;
        case Y1:  that->y1 = data->value; break;
        case X2:  that->x2 = data->value; break;
        case Y2:  that->y2 = data->value; break;
        case X3:  that->x3 = data->value; break;
        case Y3:  that->y3 = data->value; break;
        case CTX: that->cx = data->value; break;
        case CTY: that->cy = data->value; break;
        case CM:  that->cm = data->value; break;
    }
    that->present |= data->type;
    return  true;
}

// internal utility function
inline bool test( unsigned short value, unsigned short mask ) { return ( value & mask ) == mask; }

// return current VT mode
VT_Mode VT_Acc_Mode( const VT_Acc* that )
{
    if( test( that->present, LED_123_MASK ) ) return  LED_123;
    if( test( that->present, LED_12_MASK ) )  return  LED_12;
    if( test( that->present, LED_13_MASK ) )  return  LED_13;
    if( test( that->present, LED_23_MASK ) )  return  LED_23;
    if( test( that->present, CENTROID_WITH_MOTION_MASK ) )    return  CENTROID_WITH_MOTION;
    if( test( that->present, CENTROID_MASK ) )                return  CENTROID;
    if( test( that->present, LED_1_MASK ) )   return  LED_1;
    if( test( that->present, LED_2_MASK ) )   return  LED_2;
    if( test( that->present, LED_3_MASK ) )   return  LED_3;
    return  UNKNOWN;
}

// print currently accumulated values
void VT_Acc_Print( const VT_Acc* that )
{
    printf( "ts=%I64u, ", that->timestamp );

    switch( VT_Acc_Mode(that) ) {
        case CENTROID:
            printf( "CENTROID, x=%u, y=%u\n", that->cx, that->cy );
            break;
        case CENTROID_WITH_MOTION:
            printf( "CENTROID WITH MOTION, x=%u, y=%u, m=%u\n", that->cx, that->cy, that->cm );
            break;
        case LED_1:
            printf( "LED #1, x=%u, y=%u\n", that->x1, that->y1 );
            break;
        case LED_2:
            printf( "LED #2, x=%u, y=%u\n", that->x2, that->y2 );
            break;
        case LED_3:
            printf( "LED #3, x=%u, y=%u\n", that->x3, that->y3 );
            break;
        case LED_12:
            printf( "LEDs #1 & #2, x1=%u, y1=%u, x2=%u, y2=%u\n", 
                that->x1, that->y1, that->x2, that->y2 );
            break;
        case LED_13:
            printf( "LEDs #1 & #3, x1=%u, y1=%u, x3=%u, y3=%u\n", 
                that->x1, that->y1, that->x3, that->y3 );
            break;
        case LED_23:
            printf( "LEDs #2 & #3, x2=%u, y2=%u, x3=%u, y3=%u\n", 
                that->x2, that->y2, that->x3, that->y3 );
            break;
        case LED_123:
            printf( "LEDs #1 & #2 & #3, x1=%u, y1=%u, x2=%u, y2=%u, x3=%u, y3=%u\n", 
                that->x1, that->y1, that->x2, that->y2, that->x3, that->y3 );
            break;
        default:
            printf( "UNKNOWN" );
    }
}
