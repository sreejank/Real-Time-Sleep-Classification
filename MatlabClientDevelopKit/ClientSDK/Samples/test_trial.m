% this script tests Trial API

% before using any of the PL_XXX functions
% you need to call PL_InitClient ONCE
% and use the value returned by PL_InitClient
% in all PL_XXX calls
s = PL_InitClient(0);
if s == 0
   return
end

% assume CM_Quickstart.plx

PL_TrialDefine(s, -21903, 258, 0, 0, 0, 0, [1 2 3 4 5 6 7 8], [0 1 2 4], 0);

[rn, trial, spike, analog, last] = PL_TrialStatus(s, 3, 0);
[ne, eventList]  = PL_TrialEvents(s, 0, 0);
[ns, spikeList]  = PL_TrialSpikes(s, 0, 0);
[na, ts, analogList] = PL_TrialAnalogSamples(s, 0, 0);

% you need to call PL_Close(s) to close the connection
% with the Plexon server
PL_Close(s);
s = 0;

