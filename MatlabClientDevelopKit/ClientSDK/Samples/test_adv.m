% this script tests PL_GetADV (get A/D data in volts, accounting for preamp and NIDAQ gain) 

% NOTE1: Reading data from up to 256 A/D ("slow") channels is supported; however, 
% please make sure that you are using the latest version of Rasputin (which includes
% support for acquisition from multiple NIDAQ cards in parallel).

% NOTE 2: See PL_GetPars.m for information on how to determine the sampling
% rates for A/D channels.  Note that in a multiple NIDAQ card
% configuration, each card may acquire data at a different sampling rate.


% before using any of the PL_XXX functions
% you need to call PL_InitClient ONCE
% and use the value returned by PL_InitClient
% in all PL_XXX calls

s = PL_InitClient(0);
if s == 0
   return
end

% get A/D data and plot it
for i=1:10
   [n, t, d] = PL_GetADV(s);
   plot(d);
   pause(1);
end

% you need to call PL_Close(s) to close the connection
% with the Plexon server
PL_Close(s);
s = 0;

