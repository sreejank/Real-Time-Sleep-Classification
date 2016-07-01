import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

#Makes a 2D plot for each feature (all combinations)
def plotFeatures(data,ratings,actual,featureLabels):
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
    i=0
    for j in range(len(data[0])):
        for k in range(j+1,len(data[0])):
            plt.figure(i)
            f,axarr=plt.subplots(2)
            axarr[0].scatter(data[:,j],data[:,k],c=colorsActual)
            axarr[0].set_title("Actual")
            axarr[1].scatter(data[:,j],data[:,k],c=colorsRatings)
            axarr[1].set_title("Calculated")
            plt.xlabel(featureLabels[j])
            plt.ylabel(featureLabels[k])
            i+=1
    plt.show()
    print("Done!")

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


#Plot actual hypnogram and calculated hypnogram with all 3 states(from classification).
def plotHypnogram(actual,ratings):
    print("Calculating Hypnogram plot...")
    f,axarr=plt.subplots(2,sharex=True)
    axarr[0].plot(actual)
    axarr[0].set_title("Actual")
    axarr[1].plot(ratings)
    axarr[1].set_title("Calculated")
    print("Done!")

#Plot Hypnograms just for REM.
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