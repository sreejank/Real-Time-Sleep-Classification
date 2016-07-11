function [result, deviceString] = PL_DOGetDeviceString(deviceNumber)
%
% [result, deviceString] = PL_DOGetDeviceString(deviceNumber)
%
% returns an identifying string for the given NI device number, e.g. "PCI-6071E"
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%	  deviceString - an identifying string
%
% Copyright (c) 2005, Plexon Inc
%

[result, deviceString] = mexPlexDO(2, deviceNumber);
