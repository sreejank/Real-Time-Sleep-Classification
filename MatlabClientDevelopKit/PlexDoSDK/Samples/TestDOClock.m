%
% This script demonstrates basic PlexDO clock generation functionality.
% See the .m files for each PlexDO function for more information on individual
% functions.
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

% -----------------------------------------------------
% Clock generation functions

% Use GPCTR1 for clock generation.
clockLine = 1;

% Default mode for GPCTR is pulse generation, so set clock generation mode
PL_DOSetLineMode(deviceNumbers(1), clockLine, 1); % 1 = clock gen

% Define clock waveform: 100 usec high, 100 usec low = 5 kHz frequency.
PL_DOSetClockParams(deviceNumbers(1), clockLine, 100, 100);

% Start the clock.
PL_DOStartClock(deviceNumbers(1), clockLine);

% Leave clock running for 10 seconds.
PL_DOSleep(10000);

% Stop the clock.
PL_DOStopClock(deviceNumbers(1), clockLine);

% Should be called to release hardware devices before the client terminates.
PL_DOReleaseDevices();
