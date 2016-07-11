%
% This script demonstrates basic PlexDO digital output functionality for 
% outputting strobed word values, using DIO7 as the strobe bit.  
% See the .m file for each PlexDO function for more information.
%
% Uses mexPlexDO.mexw32 or mexPlexDO.mexw64.  
%
%
% Copyright (c) 2005-2015, Plexon Inc
%

% -----------------------------------------------------
% PlexDO initialization - must be called before any other PlexDO function.
[numDOCards, deviceNumbers, numBits, numLines] = PL_DOGetDigitalOutputInfo;

% Get the names of the available digital output devices (optional).
for i = 1:numDOCards
    [result, deviceString] = PL_DOGetDeviceString(deviceNumbers(i));
    % str = sprintf('NI Device %d = %s\n', deviceNumbers(i), deviceString);
end

% Initialize the first available device.  The second parameter should be 1 
% if the device is also being used by the MAP, otherwise 0.
result = PL_DOInitDevice(deviceNumbers(1), 1); 

% Set all output bits to 0.
result = PL_DOClearAllBits(deviceNumbers(1));

% -----------------------------------------------------
% Strobed word output using a DIO output bit as the strobe

strobeBit = 7; % use bit 7 (DIO7) as strobe bit
  
% Output a series of 7 bit strobed words with values from 0..127,
% with approximately 1 sec delay after each word.
for i = 0:127
  % Set the value of the 7 bit word on DIO0..DIO6
  PL_DOSetWord(deviceNumbers(1), 0, 6, i); % Bits 0 through 6 (lowest 7 bits of i are used)
  % output the strobe pulse on DIO7
  PL_DOPulseBit(deviceNumbers(1), strobeBit, 1); % 1 msec pulse on DIO7
  PL_DOSleep(1000);
end

% Should be called to release hardware devices before the client terminates.
PL_DOReleaseDevices();

