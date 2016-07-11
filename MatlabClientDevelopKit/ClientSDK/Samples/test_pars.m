% This script tests the PL_GetPars function

% Before calling any of the PL_XXX functions you must call PL_InitClient
% ONCE, but NOT within each of your Matlab functions; then pass the value 
% returned by PL_InitClient as a parameter in all subsequent PL_XXX calls 
% made from the same Matlab client program; likewise, you must call 
% PL_CloseClient ONCE before your Matlab client program exits, but NOT 
% within each of your Matlab functions.

s = PL_InitClient(0);
if s == 0
   return
end

% call PL_GetPars and display the Rasputin acquisition parameters
% see also: PL_GetPars.m

pars = PL_GetPars(s);

% ------------------------------------------------------------------------
% basic parameters
fprintf('Server Parameters:\n\n');
fprintf('DSP channels: %.0f\n', pars(1));
fprintf('Timestamp tick (in usec): %.0f\n', pars(2));
fprintf('Number of points in waveform: %.0f\n', pars(3));
fprintf('Number of points before threshold: %.0f\n', pars(4));
fprintf('Maximum number of points in waveform: %.0f\n', pars(5));
fprintf('Total number of A/D channels: %.0f\n', pars(6));
fprintf('Number of enabled A/D channels: %.0f\n', pars(7));
fprintf('A/D frequency (for continuous "slow" channels, Hz): %.0f\n', pars(8));
fprintf('A/D frequency (for continuous "fast" channels, Hz): %.0f\n', pars(13));
fprintf('Server polling interval (msec): %.0f\n', pars(9));

% ------------------------------------------------------------------------
% the following parameters are related to continuous A/D channels only

% the max number of continuous A/D channels supported in Rasputin
maxcontchans = 256; 

% Note: when using the functions PL_GetAD or PL_GetADEx, blocks of samples 
% are returned for only those A/D channels which are enabled in the Analog
% Channels settings pages in Sort Client.

% The elements of the pars( ) matrix following the "basic" parameters 
% listed above contain up to 256 per-channel sampling rates and a list of 
% the Sort Client channel numbers (e.g. 7 = AD07) for the enabled channels.

% pars(14) is the sampling rate for the first enabled A/D channel, 
% pars(15) is the rate for the second enabled A/D channel, etc.  
% note that although 256 entries are reserved for sampling rates in pars(),
% if there are less than 256 enabled channels, the trailing unused entries 
% will be 0.
adratestart = 14;

% pars(14+256) is the channel number shown in Sort Client for the first
% enabled A/D channel, pars(15+256) is the second enabled channel, etc.
% note that although 256 entries are reserved for enabled channel numbers 
% in pars(), if there are less than 256 enabled channels, the trailing 
% unused entries will be 0.
adenabledstart = adratestart+maxcontchans;

% this fills a matrix enabledchans( ) with the channel numbers of 
% only the enabled channels - we do this first so that we can display
% the per-channel sampling rates with the correct channel numbers
for i = adenabledstart : adenabledstart+maxcontchans-1
  if (pars(i))
    enabledchans(i-adenabledstart+1) = pars(i);
  end
end

% show the sampling rates for all enabled A/D channels
for i = adratestart : adratestart+maxcontchans-1
  if pars(i) > 0
    fprintf('A/D channel %d rate = %d\n', enabledchans(i-adratestart+1), pars(i));
  end
end

% show a list of all enabled A/D channel numbers
for i = adenabledstart : adenabledstart+maxcontchans-1
  if pars(i) > 0
    fprintf('A/D channel %d is enabled\n', pars(i));
  end
end

pause(30);

% you must call PL_Close(s) to close the connection
% with the Plexon server; this only needs to be done ONCE, when your 
% client program exits, NOT within every Matlab function in your client
PL_Close(s);
s = 0;

