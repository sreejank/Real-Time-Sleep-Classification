function [result] = PL_DOInitDevice(deviceNumber, isUsedByMAP)
%
% [result] = PL_DOInitDevice(deviceNumber, isUsedByMAP)
%
% Must be called before any digital output is attempted on the specified 
% device. All digital output bits and lines will be set to 0.  
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   isUsedByMAP - 1 indicates that the MAP is sharing this device, in which case output line 1 (GPCTR0) is not be available for digital output.
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%

[result] = mexPlexDO(3, deviceNumber, isUsedByMAP);
