from sklearn.cluster import KMeans
from sklearn.cluster import DBSCAN
from sklearn.cluster import SpectralClustering
from sklearn.neural_network import MLPClassifier
from sklearn import tree
from sklearn import mixture
from sklearn import svm
import random
import numpy as np
from sklearn.cross_validation import KFold
from sklearn import svm,tree
from sklearn.naive_bayes import GaussianNB
from sklearn.neighbors import KNeighborsClassifier
import load,processing
import time

#Create decision tree and output into graph visualization format (dot). Exports to graphFileName and returns model. 
def decisionTree(data,labels,graphFileName):
	print("Fitting Decision Tree...")
	data=np.asarray(data)
	model=tree.DecisionTreeClassifier(criterion='entropy',max_depth=8)
	model=model.fit(data,labels)
	tree.export_graphviz(model,out_file=graphFileName)
	print("Done!")
	print("Score of Decision Tree: {}".format(model.score(data,labels)))
	return model

#Use SVM to classify data (supervised). Return the classifier.
def svmClassify(data,labels):
	print("Classifying with SVM...")
	clf=svm.SVC()
	clf.fit(data,labels)
	return clf


#Clusters data using Gaussian mixture. Ignore outliers >4 Standard Deviations. 
def gaussianMixtureClustering(data,actual):
	print("Clustering...")
	g=mixture.GMM(n_components=3)
	outlierIndices=[]
	mainPoints=data[:]
	for index in range(len(data)):
		point=data[index]
		remove=False
		for comp in point:
			if abs(comp)>4.0:
				remove=True
		if remove: 
			mainPoints=np.delete(mainPoints,(index),axis=0)
			outlierIndices.append(index)
	g.fit(mainPoints)
	labels=g.predict(mainPoints)
	for i in outlierIndices:
		r=random.randint(0,2)
		labels=np.insert(labels,i,r)

	print("Done!")
	printSimilarity(actual,labels)
	return labels

#Classifies using kmeans algorithm (unsupervised). Eliminates all data >4 standard deviations in any of its features
#and assigns them random states.
def kMeansCluster(data):
	print("Clustering...")
	outlierIndices=[]
	mainPoints=data[:]
	for index in range(len(data)):
		point=data[index]
		remove=False
		for comp in point:
			if abs(comp)>4.0:
				remove=True
		if remove:
			outlierIndices.append(index)
			mainPoints=np.delete(mainPoints,(index),axis=0)
	
	kmeans=KMeans(n_clusters=3)
	kmeans.fit(mainPoints)
	labels=kmeans.labels_

	for i in outlierIndices:
		r=random.randint(0,2)
		np.insert(labels,i,r)

	print("Done!")
	return labels

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

#Print states of the two state arrays. 
def printSimilarity(actual,ratings):
	actualCount=[0,0,0]
	calcCount=[0,0,0]

	for i in range(len(actual)):
		actualCount[actual[i]]+=1
		calcCount[ratings[i]]+=1

	print("Wake. Calculated {} with actual {}".format(calcCount[0],actualCount[0]))
	print("NREM. Calculated {} with actual {}".format(calcCount[1],actualCount[1]))
	print("REM. Calculated {} with actual {}".format(calcCount[2],actualCount[2]))

#Run two kmeans clustering algorithms, one to seperate Wake and Sleep and one to seperate NREM and REM. 
def doubleKmeansCluster(features):
	kmeans1=KMeans(n_clusters=2)
	seperateWake=features[:,2:5] #Should be: EMG Power, Large/Small Ratio, Sign Inversions
	kmeans1.fit(seperateWake)
	sleepStates=kmeans1.labels_

	group0=[seperateWake[i][0] for i in range(len(sleepStates)) if sleepStates[i]==0]
	group1=[seperateWake[i][0] for i in range(len(sleepStates)) if sleepStates[i]==1]
	#if mean EMG power for wake is < mean EMG power for sleep, switch them.
	if np.mean(group0)< np.mean(group1):
		sleepStates=[0 if i==1 else 1 for i in sleepStates[:]]
	
	sleepIndices=[i for i in range(len(sleepStates)) if sleepStates[i]==1]
	remSeperation=[]
	for i in sleepIndices:
		remSeperation.append(features[i,0:2])

	remSeperation=np.asarray(remSeperation)
	kmeans2=KMeans(n_clusters=2)
	kmeans2.fit(remSeperation)
	remLabels=kmeans2.labels_
	sleepDict={}
	for i in range(len(sleepIndices)):
		sleepDict[sleepIndices[i]]=remLabels[i]

	finalStates=sleepStates[:]
	for i in range(len(finalStates)):
		if sleepStates[i]==1:
			finalStates[i]=sleepDict[i]+1


	group1=[remSeperation[i][0] for i in range(len(remSeperation)) if finalStates[i]==1]
	group2=[remSeperation[i][0] for i in range(len(remSeperation)) if finalStates[i]==2]

	#if mean delta power for nrem is < mean delta power for rem, switch them.
	if np.mean(group1)<np.mean(group2):
		for i in range(len(finalStates[:])):
			if finalStates[i]==1:
				finalStates[i]=2
			elif finalStates[i]==2:
				finalStates[i]=1

	return finalStates

def doubleSpectralCluster(features):
	spectral1=SpectralClustering(n_clusters=2)
	seperateWake=features[:,3:] #Should be: EMG Power, Large/Small Ratio, Sign Inversions
	sleepStates=spectral1.fit_predict(seperateWake)

	group0=[seperateWake[i][0] for i in range(len(sleepStates)) if sleepStates[i]==0]
	group1=[seperateWake[i][0] for i in range(len(sleepStates)) if sleepStates[i]==1]
	#if mean EMG power for wake is < mean EMG power for sleep, switch them.
	if np.mean(group0)< np.mean(group1):
		sleepStates=[0 if i==1 else 1 for i in sleepStates[:]]
	
	sleepIndices=[i for i in range(len(sleepStates)) if sleepStates[i]==1]
	remSeperation=[]
	for i in sleepIndices:
		remSeperation.append(features[i,:3])

	remSeperation=np.asarray(remSeperation)
	spectral2=SpectralClustering(n_clusters=2)
	remLabels=spectral2.fit_predict(remSeperation)
	sleepDict={}
	for i in range(len(sleepIndices)):
		sleepDict[sleepIndices[i]]=remLabels[i]

	finalStates=sleepStates[:]
	for i in range(len(finalStates)):
		if sleepStates[i]==1:
			finalStates[i]=sleepDict[i]+1


	group1=[remSeperation[i][0] for i in range(len(remSeperation)) if finalStates[i]==1]
	group2=[remSeperation[i][0] for i in range(len(remSeperation)) if finalStates[i]==2]

	#if mean delta power for nrem is < mean delta power for rem, switch them.
	if np.mean(group1)<np.mean(group2):
		for i in range(len(finalStates[:])):
			if finalStates[i]==1:
				finalStates[i]=2
			elif finalStates[i]==2:
				finalStates[i]=1

	return finalStates

def supervisedCrossValidate(subjectnum):
    eeg,emg,actual=load.getData('../Data/channels{}.dat'.format(subjectnum),'../Data/teststates{}.dat'.format(subjectnum))
    features=processing.getFeatures(eeg,emg)
    features=np.asarray(features)
    actual=np.asarray(actual)
    
    clfsvm=svm.SVC(kernel='linear')
    clftree=tree.DecisionTreeClassifier(criterion='entropy',max_depth=4)
    clfnb=GaussianNB()
    clfknn=KNeighborsClassifier()
    clfNN=MLPClassifier()
    print("Subject {}, Mean Classifier Accuracy, Mean REM Sensitivity, Mean REM Specificity, Mean NREM Sensitivity, Mean NREM Specificity".format(subjectnum))
    classifiers=[clfsvm,clftree,clfnb,clfknn,clfNN]
    clfTime=[]
    for clf in classifiers:
        scores=[]
        sensitivitiesrem=[]
        specificitiesrem=[]
        sensitivitiesnrem=[]
        specificitiesnrem=[]
        print(clf.__class__.__name__,end=',')
        ctime=0
        kf=KFold(len(features),n_folds=4)
        for trainIndex,testIndex in kf:
            features_train,features_test=features[trainIndex],features[testIndex]
            labels_train,labels_test=actual[trainIndex],actual[testIndex]
            
            begin=time.time()
            clf.fit(features_train,labels_train)
            ctime+=time.time()-begin
            
            scores.append(clf.score(features_test,labels_test))
            calculatedLabels=clf.predict(features_test)
            
            remtp,remfp,remtn,remfn=0,0,0,0
            nremtp,nremfp,nremtn,nremfn=0,0,0,0
            
            for i in range(len(calculatedLabels)):
                if labels_test[i]==2:
                    if calculatedLabels[i]==2:
                        remtp+=1
                    else:
                        remfn+=1
                else:
                    if calculatedLabels[i]==2:
                        remfp+=1
                    else:
                        remtn+=1
                        
                if labels_test[i]==1:
                    if calculatedLabels[i]==1:
                        nremtp+=1
                    else:
                        nremfn+=1
                else:
                    if calculatedLabels[i]==1:
                        nremfp+=1
                    else:
                        nremtn+=1
                
            sensitivitiesrem.append(float(remtp)/(remtp+remfn))
            specificitiesrem.append(float(remtn)/(remtn+remfp))
            
            sensitivitiesnrem.append(float(nremtp)/(nremtp+nremfn))
            specificitiesnrem.append(float(nremtn)/(nremtn+nremfp))
            
        print((np.mean(scores)),end=',')
        print((np.mean(sensitivitiesrem)),end=',')
        print((np.mean(specificitiesrem)),end=',')
        print((np.mean(sensitivitiesnrem)),end=',')
        print((np.mean(specificitiesnrem)),end='')
        print()
        clfTime.append(ctime)
    
    print("-----------------")
    print("Classifier Time Array: ")
    print(clfTime)
    print("-----------------")








