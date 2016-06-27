import numpy as np
from pyeeg import bin_power
from scipy import stats
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
from scipy.signal import butter,lfilter
from sklearn.cluster import KMeans
"""
Read test states from dat file. 
0=EEG Channel 1
1=EEG Channel 2
2=EMG Channel 1
"""
def readChannels(fileName):
    print("Reading Channels from file...")
    a=[]
    for line in open(fileName):
        numbers=str(line).split()
        numbers=[float(x) for x in numbers]
        a.append(numbers)
    arr=np.array(a)
    print("Done!")
    return arr

"""
Sequential search. 
"""
def find(arr,elem):
    for i in range(len(arr)):
        if arr[i]==elem:
            return i

"""
Read test states from dat file. 
0=Wake
1=NREM
2=REM
"""
def readTestStates(fileName):
    print("Reading test states from file...")
    a=[]
    for line in open(fileName):
        numbers=str(line).split()
        numbers=[int(x) for x in numbers]
        a.append(numbers)
    arr=np.array(a)
    ret=[]
    for i in range(len(arr[0])):
        index=find(arr[:,i],1)
        ret.append(index)
    print("Done!")
    return ret

"""
Break up total channel array into epoch's 
"""
def epochs(channelArray,epochSize):
    i=0
    epochs=[]
    while(i+epochSize-1<len(channelArray)):
        a=channelArray[i:i+epochSize]
        i+=epochSize
        epochs.append(a)
    return epochs

"""
Bandpass filter lowcut to highcut. 
"""
def butter_bandpass(data,lowcut,highcut,fs,order=2):
    nyqValue=0.5*fs
    lowest=lowcut/nyqValue
    highest=highcut/nyqValue
    b,a=butter(order,[lowest,highest],btype='band')
    return lfilter(b,a,data)

"""
Compute power in [bin1,bin2] for an epoch
"""
def power(epochArray,bin1,bin2):
    #powerscale=np.log10(np.abs(np.fft.rfft(epochArray)))
    bandpassed=butter_bandpass(epochArray,bin1,bin2,400)
    sumabs=sum(abs(bandpassed))
    if sumabs==0:
        return 0
    else:
        return math.log10(sumabs)
    
"""
Analyze epoch's and return features. 
"""
def analyzeEpochs(eegEpochs,emgEpochs):
    print("Calculating features...")
    delta=[]
    theta=[]
    for epoch in eegEpochs:
        delta.append(power(epoch,0.5,4))
        theta.append(power(epoch,5,10))

    emgPower=[]
    for epoch in emgEpochs:
        emgPower.append(power(epoch,0,200))

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
    return (delta,theta,ratios,emgPower)

"""
Normalizes features through zscore. 
"""
def normalize(delta,ratios,emgPower):
    print("Normalizing features...")
    normDelta=stats.zscore(delta)
    normRatios=stats.zscore(ratios)
    normEMGPower=stats.zscore(emgPower)
    print("Done!")
    return (normDelta,normRatios,normEMGPower)

"""
Classifies using unsupervised classification. 
0=Wake
1=NREM
2=REM
"""
def simpleClassify(delta,ratios,emgPower):
    data=[]
    for i in range(len(delta)):
        point=[delta[i],ratios[i],emgPower[i]]
        data.append(point)

    print("Clustering...")
    kmeans=KMeans(n_clusters=3)
    kmeans.fit(data)
    print("Done!")
    return kmeans.labels_



"""
Similarity between two state arrays. 
"""
def printSimilarity(actual,ratings):
    matches=0.0
    for i in range(len(ratings)):
        if actual[i]==2 and ratings[i]==actual[i]:
            matches+=1
    print("REM similarity")
    print(matches/len(ratings))
    matches=0.0

    for i in range(len(ratings)):
        if actual[i]==1 and ratings[i]==actual[i]:
            matches+=1
    print("NREM similarity")
    print(matches/len(ratings))
    matches=0.0

    for i in range(len(ratings)):
        if actual[i]==0 and ratings[i]==actual[i]:
            matches+=1
    print("Wake similarity")
    print(matches/len(ratings))



"""
Plot actual hypnogram and calculated hypnogram (from classification).
"""
def plotHypnogram(actual,ratings):
    print("Calculating Hypnogram plot...")
    f,axarr=plt.subplots(2,sharex=True)
    axarr[0].plot(actual)
    axarr[0].set_title("Actual")
    axarr[1].plot(ratings)
    axarr[1].set_title("Calculated")
    print("Done!")

"""
Make 3 2D plots of the features. 
"""
def plotThreeFeatures(delta,emgPower,ratios,ratings):
    print("Calculating features plot...")
    #delta=[math.log10(x) for x in delta]
    #emgPower=[math.log10(x) for x in emgPower]
    #ratios=[math.log10(x) for x in emgPower]
    colors=[]
    for i in ratings:
        if i==0:
            colors.append('r')
        elif i==1:
            colors.append('g')
        else:
            colors.append('b')

   
    plt.figure(1)
    plt.scatter(emgPower,ratios,c=colors)
    plt.xlabel("EMG Power")
    plt.ylabel("Theta Delta Ratio")

    plt.figure(2)
    plt.scatter(emgPower,delta,c=colors)
    plt.xlabel("EMG Power")
    plt.ylabel("Delta Power")

    plt.figure(3)
    plt.scatter(ratios,delta,c=colors)
    
    plt.xlabel("Theta Delta Ratio")
    plt.ylabel("Delta Power")

    print("Done!")
   

"""
Plot power spectrum transformed by real FFT. 
"""

def plotPowerSpectrum(epoch):
    print("Calculating Power Spectrum...")
    f,axarr=plt.subplots(2)
    axarr[0].plot(epoch)
    axarr[0].set_title("Signal")

    ps=np.log10(np.abs(np.fft.rfft(epoch)))
    axarr[1].plot(ps)
    axarr[1].set_title("Power Spectrum")

    print("Done!")

"""
Calculate size of epoch's given channels and number of state ratings. 
"""

def calcEpochSize(channels,ratings):
    a=np.size(channels)
    b=len(ratings)
    return a/b
##########################################

channels=readChannels('channels1032.dat')
actual=readTestStates('teststates1032.dat')
size=calcEpochSize(channels[0],actual)


eeg=epochs(channels[0],size)
emg=epochs(channels[2],size)

features=analyzeEpochs(eeg,emg) # (delta,theta,ratios,emgPower)
normalized=normalize(features[0],features[2],features[3])



ratings=simpleClassify(normalized[0],normalized[1],normalized[2])
#printSimilarity(actual,ratings)


#plotHypnogram(actual,ratings)
#plotPowerSpectrum(channels[2][0:size])
plotThreeFeatures(normalized[0],normalized[2],normalized[1],ratings)

plt.show()
