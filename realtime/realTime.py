import numpy as np

import load,processing

import sys
import os
def getEpochFeatures(fileName):
	
	channels=load.getTimeSeries(fileName)
	
	eeg=channels[0]
	emg=channels[2]

	
	delta=processing.power(eeg,0.5,4)
	
	theta=processing.power(eeg,5,10)
	
	if theta!=0:
		
		dtRatio=float(delta)/theta
	
	else:
		
		dtRatio=0.0
	
	emgPower=processing.power(emg,0,100)
	
	emgMed=np.median(emg)

	
	num=processing.power(eeg,0.5,20)
	
	dem=processing.power(eeg,0.5,55)
	
	if dem==0:
		
		largeRatio=0.0
	
	else:
		
		largeRatio=float(num)/dem

	
	signInv=processing.signInversions(eeg)

	
	features=[delta,dtRatio,emgPower,emgMed,largeRatio,signInv]
	
	
	print(features)
	
	return features



features=[]
for i in range(10):
	os.system("NIDAQRead.exe")
	features.append(getEpochFeatures("data2.csv"))
	print(i)

print(features)