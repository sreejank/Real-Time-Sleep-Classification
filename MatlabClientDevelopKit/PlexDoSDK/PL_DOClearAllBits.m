function [result] = PL_DOClearAllBits(deviceNumber)
%
% [result] = PL_DOClearAllBits(deviceNumber)
%
% Resets all digital output bits to 0 on the given device. The bits are 
% sequentially set to 0 as quickly as possible.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%

[result] = mexPlexDO(4, deviceNumber);
