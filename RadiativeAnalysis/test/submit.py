#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3
import sys
import subprocess
import datetime
import numpy as np

def checkRunning(processes) :
  nRunning = 0
  for proc, name in processes :
    status = proc.poll()
    if status==None : nRunning+=1
  return nRunning
    

lsCommand='cat ../JpsiGammaData/BsToJpsiGammaFileList.txt'
print ('command: ',lsCommand)
dir=subprocess.Popen(lsCommand, stdout=subprocess.PIPE,shell=True,text=True)
lsOutput=dir.communicate()[0]
files=[]
for f in lsOutput.split():
  #print(dataDir+f)
  files.append(f)
print ('number of files: ',len(files))

nJobs = 150
maxRunningJobs = 15 
files_splitted = np.array_split(files, nJobs)
print ('number of files: ',len(files),', submitting in', nJobs,' jobs, max running jobs: ',maxRunningJobs)

myProc=[]
for fs in range(nJobs):
  while (checkRunning(myProc) >= maxRunningJobs) : subprocess.run(['sleep','30'])

  print ('nRunning is:', checkRunning(myProc),' submitting job number:',fs,'\n', files_splitted[fs])
  jobId =str(fs).zfill(3)
  execCommand = ['cmsRun','./analysisMC_job.py', jobId, str(files_splitted[fs]).strip('[]')]
  print ('execCommand #',fs,' is: ', execCommand)
  p=subprocess.Popen(execCommand,stdout=open('JpsiGMidAprilSzGenPh_jobs/out_'+jobId+'.txt','w'), stderr=subprocess.STDOUT)
  subprocess.run(['sleep','5'])
  myProc.append( (p, jobId) )

print ('...all submitted.')
while (checkRunning(myProc)) : subprocess.run(['sleep','30'])
print ('all finished:')
for proc,name in myProc :
  status=proc.poll()
  print ('status of', proc, name, 'is: ',status) 
  subprocess.run(['sleep','1'])
  
sys.exit(0)