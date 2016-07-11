function [result] = PL_DOReleaseDevices()
%
% PL_DOReleaseDevices()
%
% Must be called before exiting from any client program that uses PlexDO. 
% If this is not done, Matlab will not release the PlexDO hardware
% resources, and the client cannot be run again until Matlab is restarted.
%
% INPUT:
%   none
%
% OUTPUT:
%   none
%
% Copyright (c) 2015, Plexon Inc
%

mexPlexDO(16);
