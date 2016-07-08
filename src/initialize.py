import numpy as np
import matplotlib as plt
import load,processing,clustering,plotting
from importlib import reload

eeg,emg,actual=load.getData('../Data/channels1032.dat','../Data/teststates1032.dat')
features=processing.getFeatures(eeg,emg)
