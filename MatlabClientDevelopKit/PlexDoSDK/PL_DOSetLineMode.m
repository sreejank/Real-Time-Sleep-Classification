function [result] = PL_DOSetLineMode(deviceNumber, lineNumber, mode)
%
% [result] = PL_DOSetLineMode(deviceNumber, lineNumber, mode)
%
% Defines whether an output line is to be used for clock generation or 
% pulse generation.  mode is either PULSE_GEN or CLOCK_GEN. The default 
% mode for all lines is PULSE_GEN.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   lineNumber - output line number (1-based)
%   mode - 0 for PULSE_GEN, 1 for CLOCK_GEN
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(9, deviceNumber, lineNumber, mode);
