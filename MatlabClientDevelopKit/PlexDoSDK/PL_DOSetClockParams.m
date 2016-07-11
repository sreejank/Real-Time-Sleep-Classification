function [result] = PL_DOSetClockParams(deviceNumber, lineNumber, microsecsHigh, microsecsLow)
%
% [result] = PL_DOSetClockParams(deviceNumber, lineNumber, microsecsHigh, microsecsLow)
%
% Specifies a clock output signal in terms of the length of the high and 
% low times of a single clock cycle. Minimum value is 1 microsecond for both 
% high and low times (i.e. clock frequency of 500 kHz), maximum value is 0.5 
% second (1 Hz). Note that this clock is free-running with respect to the MAP clock.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   lineNumber - output line number (1-based)
%   microsecsHigh - high time in microseconds
%   microsecsLow - low time in microseconds
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(12, deviceNumber, lineNumber, microsecsHigh, microsecsLow);
