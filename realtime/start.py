import lib.realTime as realTime
import sys

subject=sys.argv[1]
numEpochs=sys.argv[2]
c=realTime.SleepClassification(subject)
c.loadClassifiers()
for i in range(numEpochs):
	c.classifyEpochOnline()

c.exportPredictions()

