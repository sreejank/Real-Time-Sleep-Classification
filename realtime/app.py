import tkinter as tk
import tkinter.ttk as ttk
import tkinter.messagebox as messagebox
import tkinter.filedialog as filedialog
from threading import Thread, Event


from lib.realTime import SleepClassify

class Controller(tk.Tk):
    
    def __init__(self, *args, **kwargs):
        
        tk.Tk.__init__(self,*args,**kwargs)
            self.classifier=[SleepClassify(-1)]
                container=tk.Frame(self)
                
                container.pack(side="top", fill="both", expand = True)
                
                container.grid_rowconfigure(0, weight=1)
                container.grid_columnconfigure(0, weight=1)
                
                self.frames={}
                
                #Add new pages here.
                for F in (HomePage, MainMenu, TrainScreen, FileTrainScreen, TestingScreen):
                    frame=F(container,self, self.classifier)
                        self.frames[F]=frame
                        frame.grid(row=0, column=0, sticky='nsew')
            
            self.show_frame(HomePage)
    
    def show_frame(self, cont):
        frame=self.frames[cont]
            frame.tkraise()



class HomePage(tk.Frame):
    
    def __init__(self, parent, controller, classifier):
        tk.Frame.__init__(self,parent)
            self.controller=controller
                
                lbl=tk.Label(self,text='Enter subject name')
                lbl.grid(row=0)
                lbl.pack(anchor='center')
                
                subjectField=tk.Entry(self)
                subjectField.place(rely=0.1,relx=0.4,x=0,y=0)
                
                def start():
                    try:
                        i=int(subjectField.get())
                        except:
                            messagebox.showwarning("Number Error", "Please enter a number")
                                return
                    
                        classifier[0]=SleepClassify(i)
                    controller.show_frame(MainMenu)
        
            goButton=tk.Button(self, text='GO', command=start)
                goButton.place(rely=0.2,relx=0.47,x=0,y=0)

class MainMenu(tk.Frame):
    
    def __init__(self,parent, controller, classifier):
        tk.Frame.__init__(self, parent)
            
            lbl=tk.Label(self,text='Choose what you want to do.')
                lbl.pack(anchor='center')
                
                
                def back():
                    controller.show_frame(HomePage)
                
                backButton=tk.Button(self,text='BACK',command=back)
                backButton.place(rely=1.0, relx=0.0, x=0, y=0, anchor='sw')
                
                
                def toTraining():
                    controller.show_frame(TrainScreen)
                
                trainButton=tk.Button(self,text='Train',command=toTraining)
                trainButton.place(rely=0.4,relx=0.2,x=0,y=0)
                
                def toTesting():
                    controller.show_frame(TestingScreen)
                
                testButton=tk.Button(self,text='Test',command=toTesting)
                testButton.place(rely=0.4,relx=0.8,x=0,y=0)

class TrainScreen(tk.Frame):
    
    def __init__(self,parent,controller, classifier):
        tk.Frame.__init__(self,parent)
            lbl=tk.Label(self,text='Choose what you want to do.')
                lbl.pack(anchor='center')
                
                def back():
                    controller.show_frame(MainMenu)
                
                backButton=tk.Button(self,text='BACK',command=back)
                backButton.place(rely=1.0, relx=0.0, x=0, y=0, anchor='sw')
                
                def trainWithFile():
                    controller.show_frame(FileTrainScreen)
                
                
                fileButton=tk.Button(self,text='Train with new data file',command=trainWithFile)
                fileButton.place(rely=0.4,relx=0.2,x=0,y=0)
                
                def recoverClassifier():
                    try:
                        classifier[0].loadClassifiers()
                        except:
                            messagebox.showwarning("File error","Classifier data could not be found. Please retrain.")
                                return
                        
                    messagebox.showinfo("Success","Reloaded classifier")
        
        
            recoverButton=tk.Button(self,text='Recover saved classifier',command=recoverClassifier)
                recoverButton.place(rely=0.4,relx=0.6,x=0,y=0)

class FileTrainScreen(tk.Frame):
    def __init__(self,parent,controller, classifier):
        tk.Frame.__init__(self,parent)
            lbl=tk.Label(self,text='Choose what you want to do.')
                lbl.pack(anchor='center')
                
                def back():
                    controller.show_frame(TrainScreen)
                
                backButton=tk.Button(self,text='BACK',command=back)
                backButton.place(rely=1.0, relx=0.0, x=0, y=0, anchor='sw')
                
                self.inputchannels=-1
                self.inputstates=-1
                
                def inputChannels():
                    self.inputchannels=filedialog.askopenfilename()
                
                channelButton=tk.Button(self,text='Input Channels File', command=inputChannels)
                channelButton.place(rely=0.4,relx=0.2,x=0,y=0)
                
                def inputStates():
                    self.inputstates=filedialog.askopenfilename()
                
                stateButton=tk.Button(self,text='Input States File', command=inputStates)
                stateButton.place(rely=0.4,relx=0.6,x=0,y=0)
                
                def trainWithFiles():
                    try:
                        classifier[0].trainClassifiers(str(self.inputchannels),str(self.inputstates))
                        except:
                            print(str(self.inputchannels))
                                print(str(self.inputstates))
                                messagebox.showwarning("File error", "Classifier failed to train with Channel file "+str(self.inputchannels)+" and state file "+str(self.inputstates))
                                return
                        
                    messagebox.showinfo("Success", "Classifier successfully trained")
            finalizeButton=tk.Button(self,text='Train Classifier', command=trainWithFiles)
                finalizeButton.place(rely=0.7,relx=0.4,x=0,y=0)

class TestingScreen(tk.Frame):
    def __init__(self,parent,controller,classifier):
        tk.Frame.__init__(self,parent)
            lbl=tk.Label(self,text='Click start to begin recording. Click stop to halt recording.')
                lbl.pack(anchor='center')
                
                def back():
                    controller.show_frame(MainMenu)
                
                backButton=tk.Button(self,text='BACK',command=back)
                backButton.place(rely=1.0, relx=0.0, x=0, y=0, anchor='sw')
                
                self.recordThread= None
                
                def start():
                    self.stop_thread=Event()
                        while not self.stop_thread.is_set():
                            try:
                                classifier[0].classifyEpochOnline()
                                #print("Recording")
                                except:
                                    messagebox.showwarning("Error","Unable to record data. Make sure you trained the classifier or have the data acquisition machine on.")
                                        return
                def startThread():
                    self.recordThread=Thread(target=start)
                        self.recordThread.start()
                
                
                
                startButton=tk.Button(self,text='START',command=startThread)
                startButton.place(rely=0.7,relx=0.3,x=0,y=0)
                
                def halt():
                    self.stop_thread.set()
                        self.recordThread=None
                        messagebox.showinfo("Stop","Successfully halted recording.")
                
                stopButton=tk.Button(self,text='STOP',command=halt)
            stopButton.place(rely=0.7,relx=0.6,x=0,y=0)



def main():
    root=Controller()
        root.geometry("700x450+100+100")
        root.mainloop()

if __name__=='__main__':
    main()
