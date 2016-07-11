function [result] = PL_DOSetWord(deviceNumber, lowBitNumber, highBitNumber, value)
%
% [result] = PL_DOSetWord(deviceNumber, lowBitNumber, highBitNumber, value)
%
% Sets a contiguous range of digital output bits to the specified value.
% Only the lowest (highBitNumber - lowBitNumber + 1) bits of value are used. 
% The specified bits are set sequentially as quickly as possible.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo
%   lowBitNumber - low bit number (1-based)
%   highBitNumber - high bit number (1-based)
%   value - value of bits
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(8, deviceNumber, lowBitNumber, highBitNumber, value);
