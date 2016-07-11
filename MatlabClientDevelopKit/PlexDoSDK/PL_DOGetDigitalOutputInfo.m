function [result, deviceNumbers, numBits, numLines] = PL_DOGetDigitalOutputInfo()
%
% [result, deviceNumbers, numBits, numLines] = PL_DOGetDigitalOutputInfo()
%
% Query function which returns info on available (and supported) digital NI 
% output devices. Return value is the number of supported NI devices which 
% were found. Caller must provide three matrices in which this function returns
% device info.
%
% INPUT:
%   none
%
% OUTPUT:
%   result - the number of available digital output devices 
%	  deviceNumbers - 16 by 1 matrix containing the NI device numbers
%	  numBits - 16 by 1 matrix containing the number of digital output bits on each device
%	  numLines - 16 by 1 matrix containing the number of digital output lines on each device
%
% Copyright (c) 2005, Plexon Inc
%

[result, deviceNumbers, numBits, numLines] = mexPlexDO(1);
