function [result] = PL_DOClearBit(deviceNumber, bitNumber)
%
% [result] = PL_DOClearBit(deviceNumber, bitNumber)
%
% Clears a single digital output bit to 0. bitNumber is one-based. Bit 1
% corresponds to DIO0 on most NI devices.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo
%   bitNumber - output bit number (1-based)
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(6, deviceNumber, bitNumber);
