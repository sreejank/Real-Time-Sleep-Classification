import numpy as np
import matplotlib as plt
import load,processing,clustering,plotting
from importlib import reload

eeg,emg,actual=load.getData('channels1032.dat','teststates1032.dat')

features=processing.getFeatures(eeg,emg)
components=processing.principalComponents(eeg,emg)
