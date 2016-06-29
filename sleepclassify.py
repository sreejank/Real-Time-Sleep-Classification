import numpy as np
from pyeeg import bin_power
from scipy import stats
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
from scipy.signal import butter,lfilter
from sklearn.cluster import KMeans
from sklearn.cluster import DBSCAN
from sklearn import tree
import random

#Read test states from dat file. 
#0=EEG Channel 1
#1=EEG Channel 2
#2=EMG Channel 1

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


#Sequential search. 

def find(arr,elem):
    for i in range(len(arr)):
        if arr[i]==elem:
            return i


#Read test states from dat file. 
#0=Wake
#1=NREM
#2=REM

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


#Break up total channel array into epoch's 

def epochs(channelArray,epochSize):
    i=0
    epochs=[]
    while(i+epochSize-1<len(channelArray)):
        a=channelArray[i:i+epochSize]
        i+=epochSize
        epochs.append(a)
    return epochs


#Bandpass filter lowcut to highcut. 

def butter_bandpass(data,lowcut,highcut,fs,order=2):
    nyqValue=0.5*fs
    lowest=lowcut/nyqValue
    highest=highcut/nyqValue
    b,a=butter(order,[lowest,highest],btype='band')
    return lfilter(b,a,data)


#Compute power in [bin1,bin2] for an epoch

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




#Analyze epoch's and return features. 

def analyzeEpochs(eegEpochs,emgEpochs):
    print("Calculating features...")
    delta=[]
    theta=[]
    autocorr=[]
    for epoch in eegEpochs:
        delta.append(power(epoch,0.5,4))
        theta.append(power(epoch,5,10))
        autocorr.append(thetaAutoCorrelation(epoch))

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
    return (delta,theta,ratios,emgPower,autocorr)


#Normalizes features through zscore. 

def normalize(*args):
    print("Normalizing features...")
    fin=[]
    for arg in args:
        fin.append(stats.zscore(arg))
    print("Done!")
    return fin


#Classifies using kmeans algorithm (unsupervised). Eliminates all data >4 standard deviations in any of its features
#and assigns them random states.

def kMeansCluster(delta,ratios,emgPower):
    print("Clustering...")
    data=[]
    for i in range(len(delta)):
        point=[delta[i],ratios[i],emgPower[i]]
        data.append(point)

    i=0
    outlierIndices=[]
    mainPoints=data[:]
    for point in data:
        if abs(point[0])>4.0 or abs(point[1])>4.0 or abs(point[2])>4.0:
            outlierIndices.append(i)
            mainPoints.remove(point)
        i+=1

    kmeans=KMeans(n_clusters=3)
    kmeans.fit(mainPoints)
    labels=kmeans.labels_

    for i in outlierIndices:
        r=random.randint(0,2)
        np.insert(labels,i,r)

    print("Done!")
    return label

#Unsupervised classifying using thresholds defined by the best possible decision tree calculated. 

def thresholdClassification(X):
    calculated=[]
    if X[0]<=-0.018:
        if X[2]<=.5:
            if X[1]<=1.08:
                calculated.append(0)
            else:
                calculated.append(2)
        else:
            calculated.append(0)
    else:
        if X[2]<=.6099:
            calculated.append(1)
        else:
            calculated.append(0)

    return calculated



#Create decision tree and output into graph visualization format (dot). Exports to graphFileName

def decisionTree(data,labels,graphFileName):
    data=np.asarray(data)
    model=tree.DecisionTreeClassifier(criterion='entropy',max_depth=3)
    model=model.fit(data,labels)
    print("Score of Decision Tree: {}".format(model.score(data,labels)))
    tree.export_graphviz(model,out_file=graphFileName)
    return (model.predict(data))



#Similarity between two state arrays. 

def printSimilarity(actual,ratings):
    actualCount=[0,0,0]
    calcCount=[0,0,0]

    for i in range(len(actual)):
        actualCount[actual[i]]+=1
        calcCount[ratings[i]]+=1

    print("Wake. Calculated {} with actual {}".format(calcCount[0],actualCount[0]))
    print("NREM. Calculated {} with actual {}".format(calcCount[1],actualCount[1]))
    print("REM. Calculated {} with actual {}".format(calcCount[2],actualCount[2]))



#Plot actual hypnogram and calculated hypnogram (from classification).

def plotHypnogram(actual,ratings):
    print("Calculating Hypnogram plot...")
    f,axarr=plt.subplots(2,sharex=True)
    axarr[0].plot(actual)
    axarr[0].set_title("Actual")
    axarr[1].plot(ratings)
    axarr[1].set_title("Calculated")
    print("Done!")

#Plot REM Hypnogram (goes up to 1 if REM, 0 if not)

def plotREMHypnogram(actual,ratings):
    actualREM=[]
    ratingREM=[]
    for i in range(len(actual)):
        calc=ratings[i]
        real=actual[i]
        if calc==2:
            ratingREM.append(1)
        else:
            ratingREM.append(0)
        if real==2:
            actualREM.append(1)
        else:
            actualREM.append(0)

    f,axarr=plt.subplots(2,sharex=True)
    axarr[0].plot(actualREM)
    axarr[0].set_title("Actual")
    axarr[1].plot(ratingREM)
    axarr[1].set_title("Calculated")


#Make 3 2D plots of the features. 

def plotThreeFeatures(delta,emgPower,ratios,ratings,actual):
    print("Calculating features plot...")
    colorsRatings=[]
    colorsActual=[]
    for i in ratings:
        if i==0:
            colorsRatings.append('r')
        elif i==1:
            colorsRatings.append('g')
        else:
            colorsRatings.append('b')


    for i in actual:
        if i==0:
            colorsActual.append('r')
        elif i==1:
            colorsActual.append('g')
        else:
            colorsActual.append('b')

    plt.figure(1)
    f,axarr=plt.subplots(2)
    axarr[0].scatter(emgPower,ratios,c=colorsActual)
    axarr[0].set_title("Actual")
    axarr[1].scatter(emgPower,ratios,c=colorsRatings)
    axarr[1].set_title("Calculated")
    plt.xlabel("EMG Power")
    plt.ylabel("Theta Delta Ratio")
    
    plt.figure(2)
    f,axarr=plt.subplots(2)
    axarr[0].scatter(emgPower,delta,c=colorsActual)
    axarr[0].set_title("Actual")
    axarr[1].scatter(emgPower,delta,c=colorsRatings)
    axarr[1].set_title("Calculated")
    plt.xlabel("EMG Power")
    plt.ylabel("Delta Power")

    plt.figure(3)
    f,axarr=plt.subplots(2)
    axarr[0].scatter(ratios,delta,c=colorsActual)
    axarr[0].set_title("Actual")
    axarr[1].scatter(ratios,delta,c=colorsRatings)
    axarr[1].set_title("Calculated")
    
    plt.xlabel("Theta Delta Ratio")
    plt.ylabel("Delta Power")
    
    print("Done!")



#Plot how effective each feature is on bar graph. Does statistical tests that return a p value for each. 

def evalFeatureEffectiveness(data,actual):
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
        print("Feature {} has F={} and p={}".format(index,anovaResults[0],anovaResults[1]))






#Plot power spectrum transformed by real FFT. 

def plotPowerSpectrum(epoch):
    print("Calculating Power Spectrum...")
    f,axarr=plt.subplots(2)
    axarr[0].plot(epoch)
    axarr[0].set_title("Signal")

    ps=np.log10(np.abs(np.fft.rfft(epoch)))
    axarr[1].plot(ps)
    axarr[1].set_title("Power Spectrum")

    print("Done!")


#Calculate size of epoch's given channels and number of state ratings. 

def calcEpochSize(channels,ratings):
    a=np.size(channels)
    b=len(ratings)
    return a/b

################################################################################################################################################

channels=readChannels('channels1040.dat')
actual=readTestStates('teststates1040.dat')
size=calcEpochSize(channels[0],actual)


eeg=epochs(channels[0],size)
emg=epochs(channels[2],size)

features=analyzeEpochs(eeg,emg) # (delta,theta,ratios,emgPower,autocorr)
normalized=normalize(features[0],features[2],features[3],features[4])
data=[]
for i in range(len(features[0])):
    a=[normalized[0][i],normalized[1][i],normalized[2][i],normalized[3][i]]
    data.append(a)
data=np.asarray(data)

ratings=decisionTree(data,actual,'Entropy_Decision_Tree_Depth_3_With_Autocorrelation.dot')
printSimilarity(actual,ratings)
plotREMHypnogram(actual,ratings)

plt.show()
