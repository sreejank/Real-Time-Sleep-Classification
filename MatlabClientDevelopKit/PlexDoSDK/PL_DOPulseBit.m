function [result] = PL_DOPulseBit(deviceNumber, bitNumber, duration)
%
% [result] = PL_DOPulseBit(deviceNumber, bitNumber, duration)
%
% Pulses a single bit from low to high (0 to 1) for approximately the 
% specified duration (in milliseconds). The pulse will be at least as long 
% as specified, but the exact length and the variance of the length will 
% depend on Windows system activity. If duration is 0, the bit is pulsed 
% for as short a time as possible; this time will depend on system activity
% and the speed of the system processor.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   bitNumber - output bit number (1-based)
%   duration - approximate duration in milliseconds
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%

[result] = mexPlexDO(7, deviceNumber, bitNumber, duration);
