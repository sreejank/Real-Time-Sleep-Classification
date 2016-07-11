function [result] = PL_DOSetPulseDuration(deviceNumber, lineNumber, pulseDuration)
%
% [result] = PL_DOSetPulseDuration(deviceNumber, lineNumber, pulseDuration)
%
% When PL_DOSetLineMode is used to set a line to pulse generation mode, this 
% function should be called before calling PL_DOOutputPulse on that line; 
% otherwise, a default 1 msec pulse will be output. The line must have been set 
% to pulse generation mode by a previous call to PL_DOSetLineMode.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   lineNumber - output line number (1-based)
%   pulseDuration - the length of the pulse in microseconds
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%


[result] = mexPlexDO(10, deviceNumber, lineNumber, pulseDuration);
