% this script tests PL_GetWFEvs (get waveforms and events) function

% before using any of the PL_XXX functions
% you need to call PL_InitClient ONCE
% and use the value returned by PL_InitClient
% in all PL_XXX calls
s = PL_InitClient(0);
if s == 0
   return
end

% call PL_GetWFEvs 10 times and plot the waveforms
for i=1:10
   [n,t,w] = PL_GetWFEvs(s);
   plot(w');  
   evsSizes = size(find(t(:,1) == 4));
   nEvs = evsSizes(1,1)
   pause(0.5);
end

% you need to call PL_Close(s) to close the connection
% with the Plexon server
PL_Close(s);
s = 0;

