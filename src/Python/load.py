import numpy as np
#Read test states from .dat file. 
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

#sequential search
def find(arr,elem):
    for i in range(len(arr)):
        if arr[i]==elem:
            return i

#Get true states from .dat file.
def readStates(fileName):
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
        a=channelArray[i:int(i+epochSize)]
        i+=epochSize
        epochs.append(a)
    return epochs

#Calculate how large each epoch is. 
def calcEpochSize(channels,ratings):
    a=np.size(channels)
    b=len(ratings)
    return a/b

#Get epoch arrays from files. 
def getData(channelsFile,statesFile):
	channels=readChannels(channelsFile)
	states=readStates(statesFile)
	size=calcEpochSize(channels[0],states)

	eeg=epochs(channels[0],size)
	emg=epochs(channels[2],size)

	return (eeg,emg,states)
    
#Read Testing Data in txt format.
def readTextFile(fileName):
    lineNum=0;
    featureLabels=[]
    features=[]
    states=[]
    for line in open(fileName):
        #print(line.split('\t'))

        if(lineNum==17):
            elems=line.split('\t')
            featureLabels=elems[3:6]
        elif(lineNum>18):
            elems=line.split('\t')
            if(elems[2]=='W'):
                states.append(0)
            elif(elems[2]=='NR'):
                states.append(1)
            else:
                states.append(2)
            numbers=[float(x) for x in elems[3:6]]
            features.append(numbers)
        lineNum+=1
    features=np.asarray(features)
    states=np.asarray(states)
    return (featureLabels,features,states)


