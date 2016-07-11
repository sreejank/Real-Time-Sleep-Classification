%
% This script demonstrates basic PlexDO digital output functionality, 
% including setting, clearing, and pulsing output bits.  See the .m 
% files for each PlexDO function for more information on individual
% functions.
%
% Uses mexPlexDO.mexw64 or mexPlexDO.mexw32.  NI drivers (NIDAQ 6.9 or above) must 
% be installed.
%
%
% Copyright (c) 2005-2015, Plexon Inc
%

% -----------------------------------------------------
% PlexDO initialization - must be called before any other PlexDO function.
[numDOCards, deviceNumbers, numBits, numLines] = PL_DOGetDigitalOutputInfo

% Get the names of the available digital output devices (optional).
for i = 1:numDOCards
    [result, deviceString] = PL_DOGetDeviceString(deviceNumbers(i));
    % str = sprintf('NI Device %d = %s\n', deviceNumbers(i), deviceString);
end

% Initialize the first available device.  The second parameter should be 1 
% if the device is also being used by the MAP, otherwise 0.
result = PL_DOInitDevice(deviceNumbers(1), 0); 

% Set all output bits to 0.
result = PL_DOClearAllBits(deviceNumbers(1));

% -----------------------------------------------------
% Single bit output functions

% Set bit 1 (DIO0) high.  The second parameter is the 1-based bit number.
PL_DOSetBit(deviceNumbers(1), 1); 

% Wait for approximately 1 second
PL_DOSleep(1000);

% Clear bit 1.
PL_DOClearBit(deviceNumbers(1), 1);

% Wait for approximately 1 second
PL_DOSleep(1000);

% Pulse bit 1.  Duration is at least the specified time in milliseconds.
PL_DOPulseBit(deviceNumbers(1), 1, 5); % 5 msec

% Should be called to release hardware devices before the client terminates.
PL_DOReleaseDevices();

