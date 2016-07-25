import numpy as np
from sklearn import tree
from sklearn import svm
from sklearn.naive_bayes import GaussianNB
from sklearn.neighbors import KNeighborsClassifier
import load,processing
import sys
import os
import pickle


class SleepClassify:

	def __init__(self,trainingFileName,subject):
		self.epochFeatures=np.array([])
		self.totalFeatures=[]
		self.subjectNum=subject
		self.calcLabels=[]

		self.clfs=[]
		self.clfs.append(svm.SVC())
		self.clfs.append(tree.DecisionTreeClassifier(criterion='entropy',max_depth=4))
		self.clfs.append(GaussianNB())
		self.clfs.append(KNeighborsClassifier())

		trainingData=load.readTestFile(trainingFileName)
		self.featureNames=trainingData[0]
		for clf in self.clfs:
			print("training {}...".format(clf.__class__.__name__))
			clf.fit(trainingData[1],trainingData[2])
			print("Done!")

		self.saveClassifiers()

	def __repr__(self):
		return "<SleepClassify Subject "+str(self.subjectNum)+">"

	#Get features of a single epoch from a file (meant to be from real time Plexon NIDAQ reading).
	def getEpochFeatures(self):
		channels=load.getTimeSeries("data2.csv")
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
		self.epochFeatures=[delta,dtRatio,emgPower,emgMed,largeRatio,signInv]
		self.totalFeatures.append(self.epochFeatures)
		return self.epochFeatures

	#Save classifiers in pickle files. 
	def saveClassifiers(self):
		for clf in self.clfs:
			fileName="Classifier_"+str(self.subjectNum)+"__"+str(clf.__class__.__name__)
			pickle.dump(clf,open(fileName,'wb'))

	#Load classifiers from pickle files. 
	def loadClassifiers(self):
		self.clfs[0]=pickle.load(open("Classifier_"+str(self.subjectNum)+"__"+"SVC",'rb'))
		self.clfs[1]=pickle.load(open("Classifier_"+str(self.subjectNum)+"__"+"DecisionTreeClassifier",'rb'))
		self.clfs[2]=pickle.load(open("Classifier_"+str(self.subjectNum)+"__"+"GaussianNB",'rb'))
		self.clfs[3]=pickle.load(open("Classifier_"+str(self.subjectNum)+"__"+"KNeighborsClassifier",'rb'))

	#Classify a single 5s epoch. 
	def classifyRealTime(self,clfNum=0):
		os.system("NIDAQRead.exe")
		self.getEpochFeatures()
		feats=[self.epochFeatures]
		self.calcLabels.append(self.clfs[0].predict(feats)[0])

	#Export predictions for each epoch read.
	def exportPredictions(self):
		file=open("Predictions_subject__"+str(self.subjectNum),'w')
		for i in range(len(self.calcLabels)):
			feats=self.totalFeatures[i]
			label=self.calcLabels[i]
			for f in feats:
				file.write(f)
				file.write(',')
			file.write(str(label))
			file.write('\n')



"""
if __name__=='__main__':
	features=[]
	for i in range(10):
		os.system("NIDAQRead.exe")
		features.append(getEpochFeatures("data2.csv"))
"""
