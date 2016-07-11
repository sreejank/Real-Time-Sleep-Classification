function [result] = PL_DOSetBit(deviceNumber, bitNumber)
%
% [result] = PL_DOSetBit(deviceNumber, bitNumber)
%
% Sets a single digital output bit to 1. bitNumber is one-based. Bit 1 
% corresponds to DIO0 on most NI devices.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   bitNumber - output bit number (1-based)
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(5, deviceNumber, bitNumber);
