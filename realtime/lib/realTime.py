import numpy as np
from sklearn import tree
from sklearn import svm
from sklearn.naive_bayes import GaussianNB
from sklearn.neighbors import KNeighborsClassifier
import load
import processing
import sys
import os
import pickle
from pypl2api import *
from pypl2lib import *


class SleepClassify:

	#Constructor
	def __init__(self,subject):
		self.epochFeatures=np.array([])
		self.totalFeatures=[]
		self.subjectNum=subject
		self.calcLabels=[]

		self.clfs=[]
		self.clfs.append(svm.SVC())
		self.clfs.append(tree.DecisionTreeClassifier(criterion='entropy',max_depth=4))
		self.clfs.append(GaussianNB())
		self.clfs.append(KNeighborsClassifier())

	#Object representation
	def __repr__(self):
		return "<SleepClassify Subject "+str(self.subjectNum)+">"

	#Train classifiers given training data files. 
	def trainClassifiers(self,channelFileName,statesFileName):
		training=load.getData(channelFileName,statesFileName)
		eeg=training[0]
		emg=training[1]

		trainingFeatures=[processing.getEpochFeatures(eeg[i],emg[i]) for i in range(len(eeg))]
		trainingStates=training[2]

		self.featureNames=['Delta','Delta Theta Ratio', 'EMG Median', 'EMG Power', 'Large Ratio', 'Sign Inversions']
		for clf in self.clfs:
			print("training {}...".format(clf.__class__.__name__))
			clf.fit(trainingFeatures,trainingStates)
			print("Done!")
		self.saveClassifiers()

	#Save classifiers in pickle files. 
	def saveClassifiers(self):
		for clf in self.clfs:
			fileName="../ClassifierData/Classifier_"+str(self.subjectNum)+"__"+str(clf.__class__.__name__)
			pickle.dump(clf,open(fileName,'wb'))

	#Load classifiers from pickle files. 
	def loadClassifiers(self):
		self.clfs[0]=pickle.load(open("../ClassifierData/Classifier_"+str(self.subjectNum)+"__"+"SVC",'rb'))
		self.clfs[1]=pickle.load(open("../ClassifierData/Classifier_"+str(self.subjectNum)+"__"+"DecisionTreeClassifier",'rb'))
		self.clfs[2]=pickle.load(open("../ClassifierData/Classifier_"+str(self.subjectNum)+"__"+"GaussianNB",'rb'))
		self.clfs[3]=pickle.load(open("../ClassifierData/Classifier_"+str(self.subjectNum)+"__"+"KNeighborsClassifier",'rb'))

	#Classify a single 5s epoch online through reading directly from the OmniPlex. clfNum is to specifiy classifier 
	# (0=SVM, 1=Decision Tree, 2= Naive Bayes, 3=k-Nearest) EEG Channel can be either 0 or 1.
	def classifyEpochOnline(self,clfNum=0, eegChannel=0):
		os.system("..\\bin\\NIDAQRead.exe")
		channels=load.getTimeSeries("data2.csv")
		eeg=channels[eegChannel]
		emg=channels[2]
		self.epochFeatures=processing.getEpochFeatures(eeg,emg)
		self.totalFeatures.append(self.epochFeatures)
		feats=[self.epochFeatures]
		self.calcLabels.append(self.clfs[0].predict(feats)[0])

	#Export predictions for each epoch read.
	def exportPredictions(self):
		file=open("../Data/Predictions_subject__"+str(self.subjectNum)+".csv",'w')
		file.write('Delta,Delta Theta Ratio, EMG Median, EMG Power, Large Ratio, Sign Inversions,state\n')
		for i in range(len(self.calcLabels)):
			feats=self.totalFeatures[i]
			label=self.calcLabels[i]
			for f in feats:
				file.write(str(f))
				file.write(',')
			file.write(str(label))
			file.write('\n')

	#Classify offline through reading in a pl2 file.
	def classifyEpochOffline(self, clfNum=0, eegChannel=0, pl2FileName):
		if eegChannel==0:
			eegTup=pl2_ad(pl2FileName,'FP01')[4]
		else:
			eegTup=pl2_ad(pl2FileName,'FP02')[4]
		
		emgTup=pl2_ad(pl2FileName,'FP03')

		eeg=list(eegTup)
		emg=list(emgTup)
		self.epochFeatures=processing.getEpochFeatures(eeg,emg)
		self.totalFeatures.append(self.epochFeatures)
		feats=[self.epochFeatures]
		self.calcLabels.append(self.clfs[0].predict(feats)[0])


