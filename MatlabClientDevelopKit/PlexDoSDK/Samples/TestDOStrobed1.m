%
% This script demonstrates basic PlexDO digital output functionality for 
% outputting strobed word values, using GPCTR1 as the strobe bit.  
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
% Strobed word output using a GPCTR output line as the strobe

pulseLine = 1; % use line 1 (GPCTR1) as strobe bit

% set line mode to pulse generation
PL_DOSetLineMode(deviceNumbers(1), pulseLine, 0); % 0 = pulse generation

% use DIO0..DIO7 as the bits of an 8 bit word
% output a series of 8 bit strobed words, values from 0..255,
% with approximately 1 sec delay after each word
for i = 0:255
  % set the value of the 8 bit word
  PL_DOSetWord(deviceNumbers(1), 0, 7, i); % bits 0 through 7 (low 8 bits of i are used)
  % generate the strobe pulse
  PL_DOOutputPulse(deviceNumbers(1), pulseLine);
  PL_DOSleep(1000);
end

% Should be called to release hardware devices before the client terminates.
PL_DOReleaseDevices();
