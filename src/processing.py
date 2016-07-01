import numpy as np
import math
from scipy.signal import butter,lfilter
from sklearn.decomposition import PCA
from scipy import stats

#Bandpass filtering of signal from lowcut to highcut. 
def butter_bandpass(data,lowcut,highcut,fs,order=2):
	nyqValue=0.5*fs
	lowest=lowcut/nyqValue
	highest=highcut/nyqValue
	b,a=butter(order,[lowest,highest],btype='band')
	return lfilter(b,a,data)

#Compute signal power in frequncies [bin1,bin2] for an epoch
def power(epochArray,bin1,bin2):
	bandpassed=butter_bandpass(epochArray,bin1,bin2,400)
	sumabs=sum(abs(bandpassed))
	if sumabs==0:
		return 0
	else:
		return math.log10(sumabs)

#Compute "Autocorrelation metric" found in the seizure paper for a given EEG epoch. 
def thetaAutoCorrelation(epochArray):
	theta=butter_bandpass(epochArray,5,10,400)
	i=0
	minmaxes=[]
	while(i+10<=len(theta)):
		samp=theta[i:i+10]
		extremes=(min(samp),max(samp))
		minmaxes.append(extremes)
		i+=10
	i=0
	sumdiff=0
	while(i+2<len(minmaxes)):
		si=minmaxes[i]
		si1=minmaxes[i+1]
		si2=minmaxes[i+2]
		Hvi=min(si[1],max(si1[1],si2[1]))
		Lvi=max(si[0],min(si1[0],si2[0]))
		sumdiff+=Hvi-Lvi
		i+=10

	return sumdiff

#Find Approximate Entropy of this epoch (bandpassing for theta). Using m=2 and r=0.2*SD. 
def approximateEntropy(epoch):
	r=0.2*np.std(epoch)
	epoch=butter_bandpass(epoch,5,10,400)

	def phi(m,epoch):
		x=[]
		for i in range(len(epoch)-m):
			xi=[]
			for k in range(m):
				xi.append(epoch[i+k])
			x.append(xi)
		C=[]
		for i in range(len(x)):
			num=0
			for j in range(len(x)):
				compDistances=[]
				for index in range(m):
					compDistances.append(abs(x[i][index]-x[j][index]))
				dist=max(compDistances)
				if dist<r:
					num+=1
			val=float(num)/len(x)
			C.append(val)
		p=sum(C)
		logged=np.log(p)
		logged=logged/len(C)
		return logged

	return phi(2,epoch)-phi(3,epoch)

#Calculates number of sign inversions in an eeg epoch. 
def signInversions(eegEpoch):
	inversions=0
	state=0
	if eegEpoch[0]>=0:
		state=1
	else:
		state=0

	for num in eegEpoch[1:]:
		prev=state
		if num>=0:
			state=1
		else:
			state=0
		if prev!=state:
			inversions+=1
	return inversions

#Find the following features: Delta Power, Theta Power, Delta/Theta Ratio, Total EMG Power, Autocorrelation for Theta
def findFeatures(eegEpochs,emgEpochs):
	print("Calculating features...")
	delta=[]
	theta=[]
	autocorr=[]
	signInv=[]
	largeRatio=[]
	for epoch in eegEpochs:
		delta.append(power(epoch,0.5,4))
		theta.append(power(epoch,5,10))
		autocorr.append(thetaAutoCorrelation(epoch))
		signInv.append(signInversions(epoch))

		num=power(epoch,0.5,20)
		dem=power(epoch,0.5,55)
		if dem==0:
			rati=0.0
		else:
			rati=float(num)/dem


		largeRatio.append(rati)

	emgMed=[]
	emgPower=[]
	for epoch in emgEpochs:
		emgPower.append(power(epoch,0,200))
		emgMed.append(np.median(epoch))

	ratios=[]

	for i in range(len(theta)):
		t=theta[i]
		d=delta[i]
		ratio=0.0
		if d==0:
			ratio=0.0
		else:
			ratio=float(t)/float(d)
		ratios.append(ratio)
	print("Done!")
	return (delta,theta,ratios,emgPower,autocorr,emgMed,largeRatio,signInv)

#Normalizes features through zscore. 
def normalize(*args):
	print("Normalizing features...")
	fin=[]
	for arg in args:
		fin.append(stats.zscore(arg))
	print("Done!")
	return fin

#Takes normalized features and creates the data array of all features. 
def genFeatureData(normalizedArray):
	data=[]
	for i in range(len(normalizedArray[0])):
		a=[normalizedArray[0][i],normalizedArray[1][i],normalizedArray[2][i],normalizedArray[3][i],normalizedArray[4][i],normalizedArray[5][i],normalizedArray[6][i], normalizedArray[7][i]]
		data.append(a)
	data=np.asarray(data)
	return data

#Get features. Return tuple of: Delta Power, Delta/Theta Ratio, EMG Power. 
def getFeatures(eegEpochs,emgEpochs):
	features=findFeatures(eegEpochs,emgEpochs) # (delta,theta,ratios,emgPower,autocorr,emgMed,largeRatio, sign inv)
	normalized=normalize(features[0],features[1],features[2],features[3],features[4],features[5],features[6], features[7])
	data=genFeatureData(normalized)
	return data

#Extract principal components for each eeg,emg epoch
def principalComponents(eegEpochs,emgEpochs,n_comps=4):
	print("Running Principal Components Analysis...")
	pca=PCA(n_components=n_comps)
	transformed=[]
	for i in range(len(eegEpochs)):
		eegEpoch=eegEpochs[i]
		emgEpoch=emgEpochs[i]
		pseeg=np.log10(np.abs(np.fft.rfft(eegEpoch)))
		psemg=np.log10(np.abs(np.fft.rfft(emgEpoch)))
		data=np.vstack((pseeg,psemg))
		pca.fit(data)
		comp=pca.transform(data)
		comp=np.concatenate((comp[0],comp[1]))  
		transformed.append(comp)
	print("Done!")
	return np.asarray(transformed)

#Does Analysis of Variance of each feature that returns corresponding F and p statistics. 
def evalFeatureEffectiveness(data,actual, featureLabels):
	for index in range(len(data[0])):
		featureArr=data[:,index]
		group1,group2,group3=[],[],[]
		for index2 in range(len(actual)):
			if actual[index2]==0:
				group1.append(featureArr[index2])
			elif actual[index2]==1:
				group2.append(featureArr[index2])
			else:
				group3.append(featureArr[index2])
		
		group1=np.asarray(group1)
		group2=np.asarray(group2)
		group3=np.asarray(group3)
		anovaResults=stats.f_oneway(group1,group2,group3)
		print("Feature {} has F={} and p={}".format(featureLabels[index],anovaResults[0],anovaResults[1]))

#Conducts Analysis of Variance on <Class> vs <non-class> (ex. REM vs NREM+Wake) to see how well features seperate REM.
#0 for wake, 1 for NREM, 2 for REM 
def evalFeatureEffectivenessClass(data,actual,featureLabels,state):
	for index in range(len(data[0])):
		featureArr=data[:,index]
		group1,group2=[],[]
		for index2 in range(len(actual)):
			if actual[index2]!=state:
				group1.append(featureArr[index2])
			else:
				group2.append(featureArr[index2])
		
		group1=np.asarray(group1)
		group2=np.asarray(group2)
		anovaResults=stats.f_oneway(group1,group2)
		print("Feature {} has F={} and p={}".format(featureLabels[index],anovaResults[0],anovaResults[1]))
