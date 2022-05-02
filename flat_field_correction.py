
import ROOT as root
import numpy as np
import matplotlib.pyplot as pl
import numpy.polynomial.polynomial as poly
import sys
import os
#from time import sleep

def hist_to_matrix(histo):
	n=histo.GetNbinsX()
	m=histo.GetNbinsY()
	matrix=np.zeros((n,m))
	for i in range(0,n):
		for j in range(0,m):
			matrix[i,j]=histo.GetBinContent(i+1,j+1)
	
	return matrix


def flat_field_correction(filename,depth):
	
	
	'''
	file = root.TFile(filename,"read")
	dir = file.GetDirectory("Xray")
	dir.cd()
	tree = root.TTree()
	tree=dir.Get("events")
	'''
	
	################### 
	# The FFC can be 1: calculated for each pixel, 2: for each column from summed data, or 3: fit to summed column values
	###################
	
	global x, hitmap, qmap, hitcol, dhitcol, qcol, fith, fitq, ffc, ffcq, ddd, offvalues
	hitmap=np.zeros((52,80))
	qmap=np.zeros((52,80)) # per pixel ffc correction --> alot of noise
	hitcol=np.zeros(52)
	qcol=np.zeros(52)
	colw=np.zeros(52)
	
	# analyze the file with C++. Much faster. Result written in temp.root
	commandstr="root -q -l 'read_result_file.cpp(\"" + filename + "\")'" # -q closes root after excecution, -l hides the root banner
	os.system(commandstr)
	print("Done, back to python.")
	
	root.EnableImplicitMT()
	tree = root.TFile("temp.root","read")
	hist = tree.Get("detector_data")
	hitmap=hist_to_matrix(hist)
	histq = tree.Get("detector_data_q")
	qmap=hist_to_matrix(hist)
	
	
	'''
	count2=0
	for event in tree:
	
		
		if count2>10000:
			break
		count2+=1
		
	
		npix=np.asarray(event.npix)
		col=np.asarray(event.pcol)
		row=np.asarray(event.prow)
		q=np.asarray(event.pq)
	
		for n in range(npix):
			#if row[n]>40: # don't take data from the hole in the detector
			hitmap[col[n]][row[n]]+=1
			qmap[col[n]][row[n]]+=q[n]
			#hitcol[col[n]]+=1
			#qcol[col[n]]+=q[n]
			
	
	file.Close()
	file.Delete()
	'''
	
	
	# correct for dead pixels
	for n in range(52):
		for m in range(80):
			if hitmap[n,m]>10:
				hitcol[n]+=hitmap[n,m]
				qcol[n]+=qmap[n,m]
				colw[n]+=1
		if colw[n]==0:
			colw[n]=1 # get rid of div by zero
	
	dhitcol=np.sqrt(hitcol)
	hitcol=np.divide(hitcol,colw)
	dhitcol=np.divide(dhitcol,colw)
	qcol=np.divide(qcol,colw)
	
	x=np.arange(0,52)
	hit_mean=np.mean(hitmap[1:50][1:78])
	q_mean=np.mean(qmap[1:50][1:78])
	hit_mean2=np.mean(hitcol[1:50])
	q_mean2=np.mean(qcol[1:50])
	
	#ffc=np.divide(qcol,q_mean2)
	ffc=np.divide(hitmap,hit_mean)
	ffcq=np.divide(qmap,q_mean)
	#pl.plot(ffc)
	#pl.show()
	
	'''
	f=open('ffc_correction_from_python.txt','w')
	for line in ffc:
		np.savetxt(f,line,fmt='%.4f')
	f.close()
	'''
	
	
	fith=np.polyfit(x[1:50],hitcol[1:50]/hit_mean2,1)
	fitq=np.polyfit(x[1:50],qcol[1:50]/q_mean2,1)
	print('Fit parameters:\n',fith)
	colfit=np.zeros(52)
	
	for n in range(0,52):
		colfit[n]=np.polyval(fith,n)
	
	pl.ion()
	#pl.plot(x,qcol/q_mean2,'.',x,colfit)
	#pl.plot(x,hitcol/hit_mean2,'.',x,colfit)
	#pl.show()
	
	#ddd=np.sqrt(np.mean(np.sum(hitmap[1:50,:],1)))
	#ddd=ddd/79
	ddd=np.max(dhitcol)
	offvalues=0
	for i in range(1,51):
		#if hitcol[i]<hit_mean2-2*ddd or hitcol[i]>hit_mean2+2*ddd:
		if hitcol[i]-2*dhitcol[i]>hit_mean2 or hitcol[i]+2*dhitcol[i]<hit_mean2: # practically the same thing
			offvalues+=1
	
	print("Values outside k=2:", offvalues, "/50 =", offvalues/50)
	
	#pl.plot(x,hitcol,'.',[0,52],[hit_mean2-2*ddd,hit_mean2-2*ddd],[0,52],[hit_mean2+2*ddd,hit_mean2+2*ddd])
	pl.plot(x,hitcol,'.',x,hit_mean2-2*dhitcol,x,hit_mean2+2*dhitcol)
	
	
	'''
	out=open("FFC_values_at_depth.txt",'a')
	out.write(depth + ' ' + str(fitq[0]) + ' ' + str(fitq[1]) + '\n')
	out.close()
	'''

######## 'main' ###########

input=sys.argv

#file1="/home/jot/HIP/measurement_files/Profile_scan_20210617_151246/Profile_measurement_pos_8p000001_0p000000_0p000000.root"
#file1="/home/jot/HIP/measurement_files/Profile_scan_20211013_123516/Profile_measurement_pos_8p000001_0p000000_0p000000.root"
#file1="/home/jot/HIP/measurement_files/Profile_scan_20210531_133853/Profile_measurement_pos_7p409998_0p000000_0p000000.root"
file1="/home/jot/HIP/measurement_files/Profile_scan_20211019_112851/Profile_measurement_pos_7p200001_0p000000_0p000000.root"
#file1="column_q_data.root"
flat_field_correction(file1,input[1])

	
