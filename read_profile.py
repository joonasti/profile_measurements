
import ROOT as root
import numpy as np
import numpy.polynomial.polynomial as poly
import matplotlib.pyplot as pl
import os as os
import pickle
from find_fwhm import *
import copy as cp



def hist_to_matrix(histo):
	n=histo.GetNbinsX()
	m=histo.GetNbinsY()
	matrix=np.zeros((n,m))
	for i in range(0,n):
		for j in range(0,m):
			matrix[i,j]=histo.GetBinContent(i+1,j+1)
	
	return matrix
	
def get_profile(rootfile):
	global x,colw,profile,profilesum,dprofilesum,profile_norm,dp,x2,prof2,dprof2,prof0
	root.EnableImplicitMT()
	tree = root.TFile(rootfile,"read")#root.TFile("saved_profile.root","read")
	#hist = tree.Get("profile_q")
	hist = tree.Get("profile")
	profile=hist_to_matrix(hist)
	colwhist=tree.Get("colw")
	colw=hist_to_matrix(colwhist)
	
	
	#colw[0]=colw[1]=40 # Get rid of div by zero
	#colw[983:1000]=40
	
	n=hist.GetNbinsX()
	x=np.zeros(n);
	for j in range(0,n):
		x[j]=float(hist.ProjectionX().GetBinCenter(j+1))
	
	
	profilesum=np.sum(profile,1)
	
	# remove elements where there were no counts
	j=0	
	while j<n: 
		if profilesum[j]==0:
			x=np.delete(x,j)
			profilesum=np.delete(profilesum,j)
			colw=np.delete(colw,j)
			n-=1
		else:
			j+=1
		
	
	dprofilesum=np.sqrt(profilesum)
	profile_norm=np.divide( profilesum, colw)
	dp=np.divide(dprofilesum,colw)
	
	
	#prof0=np.mean(profile_norm[510:550])
	prof0=np.mean(profile_norm[950:1050])
	
	
	# Oscillations in data. Seems to be a feature of the measurement?
	# average to 0.9 mm bins
	n=np.size(profile_norm)
	x2=np.zeros(int(n/6))
	prof2=np.zeros(int(n/6))
	dprof2=np.zeros(int(n/6))
	i=0
	while i<=n-6:
		i2=int(round(i/6))
		x2[i2] = np.sum( np.multiply( profilesum[i:i+6], x[i:i+6] ) )/np.sum(profilesum[i:i+6])
		prof2[i2] = np.sum( profilesum[i:i+6] )/np.sum(colw[i:i+6])
		dprof2[i2] = np.sqrt(np.sum( profilesum[i:i+6] ))/np.sum(colw[i:i+6])
		i+=6
	return x2, prof2

# profile read with different types of flat field correction. 1: correction in every pixel, 2: correction per column, and 3: fit to column points
#get_profile("saved_profile.root")
'''
x1=x
rel1=rel
get_profile("saved_profile2.root")
x2=x
rel2=rel
get_profile("saved_profile3.root")

x=np.round(x,4)
out=open("normalized_by_column_profile_python_MayII.txt",'w')

for n in range(0,int(profile_norm.size)):
	out.write(str(x[n]) + ' ' + str(profile_norm[n]) + ' ' + str(dp[n]) + '\n')

out.close()

out=open("normalized_by_column_profile_averaged_python_May.txt",'w')

for n in range(0,int(prof2.size)):
	out.write(str(x2[n]) + ' ' + str(prof2[n]) + ' ' + str(dprof2[n]) + '\n')

out.close()
'''


#
# Do analysis on the repeated measurements of x-axis from December
#

#get_profile("saved_profile.root")
#print(len(prof2))


path1="/media/jot/Elements/Profile_scans/Profile_scan_202112"
path2=["21_144320/","21_155439/","22_092429/","22_103949/","22_114448/","22_124924/","22_140734/","22_151726/"]

num=len(path2)
'''
length=166 #len(prof2)
xx=np.zeros((length,num))
profilex=np.zeros((length,num))
dprofilex=np.zeros((length,num))

for n in range(num):
	command="root -q \'build_profile.cpp(\"" + path1 + path2[n] + "\")\'"
	os.system(command)
	print("reading the saved profile in python")
	get_profile("saved_profile.root")
	
	# Step1: compare profiles without adjusting the x-axis, and without normalization
	xx[:,n]=x2
	profilex[:,n]=prof2
	dprofilex[:,n]=dprof2
	
print("All files were read. Saving the data to files in path python_variables")
xfile=open("python_variables/repeated_x_measurements.dat",'wb')
pickle.dump([xx,profilex,dprofilex],xfile)
xfile.close()
'''

xfile=open("python_variables/repeated_x_measurements.dat",'rb') # don't know why rb instead of r
xx,profilex,dprofilex=pickle.load(xfile)
xfile.close()

# go through the data and delete faulty measurements
for m in range(2,num): # only 22.12
	for n in range(20): # 20 the number of measurements per scan
		if np.nanmean(profilex[int(n*50/6):int(n*50/6)+8,m])/np.mean(profilex[int(n*50/6):int(n*50/6)+8,4])<0.99:
			xx[int(n*50/6):int(n*50/6)+9,m]=float('nan')
			profilex[int(n*50/6):int(n*50/6)+9,m]=float('nan')
			dprofilex[int(n*50/6):int(n*50/6)+9,m]=float('nan')
			#print(m,n,np.nanmean(profilex[int(n*50/6):int(n*50/6)+8,m])/np.mean(profilex[int(n*50/6):int(n*50/6)+8,4]))

		


refprofile=np.nanmean(profilex[:,2:],1) # only measurements from 22.12
drefprofile=np.sqrt( np.nansum(pow(dprofilex[:,2:]/num,2),1) )
stdrefprofile=np.nanstd(profilex[:,2:],1)

pl.ion()
#pl.plot(xx,profilex)
#pl.errorbar(xx[:,1],refprofile,yerr=2*dprofilex[:,1],marker='.')

# Step2: shift x-axis with fwhm (needed at least to compare 21.12 and 22.12.2021 results)
xx2=cp.deepcopy(xx)
profrel=cp.deepcopy(profilex)
hml=np.zeros(num)
hmr=np.zeros(num)
fwhm=np.zeros(num)
for j in range(num):
	maxval=np.nanmean(profilex[83:87,j])
	profrel[:,j]=profilex[:,j]/maxval
	hml[j],hmr[j],fwhm[j]=find_fwhm(xx[:,j],profilex[:,j],maxval)
	xx2[:,j]=xx[:,j]-(hml[j]+hmr[j])/2

# Make a piecewise polynomial to the profile and compare results to this curve
# This did not really lead to good visualization of the scatter in the data. Better to compare results only form 22.12

xp1=np.matrix.flatten(xx2[0:25,:]) # in polyfit, data (x) has to be 1D
yp1=np.matrix.flatten(profilex[0:25,:])
p1=np.polyfit(xp1[~np.isnan(yp1)],yp1[~np.isnan(yp1)],5) # get rid of nan values

xp2=np.matrix.flatten(xx2[25:47,:])
yp2=np.matrix.flatten(profilex[25:47,:])
p2=np.polyfit(xp2[~np.isnan(yp2)],yp2[~np.isnan(yp2)],5)

xp3=np.matrix.flatten(xx2[45:62,:])
yp3=np.matrix.flatten(profilex[45:62,:])
p3=np.polyfit(xp3[~np.isnan(yp3)],yp3[~np.isnan(yp3)],4)

xp4=np.matrix.flatten(xx2[60:115,:])
yp4=np.matrix.flatten(profilex[60:115,:])
p4=np.polyfit(xp4[~np.isnan(yp4)],yp4[~np.isnan(yp4)],4)

xp5=np.matrix.flatten(xx2[115:128,:])
yp5=np.matrix.flatten(profilex[115:128,:])
p5=np.polyfit(xp5[~np.isnan(yp5)],yp5[~np.isnan(yp5)],4)

xp6=np.matrix.flatten(xx2[127:147,:])
yp6=np.matrix.flatten(profilex[127:147,:])
p6=np.polyfit(xp6[~np.isnan(yp6)],yp6[~np.isnan(yp6)],4)

xp7=np.matrix.flatten(xx2[146:,:])
yp7=np.matrix.flatten(profilex[146:,:])
p7=np.polyfit(xp7[~np.isnan(yp7)],yp7[~np.isnan(yp7)],4) # huhhuh mikä homma...


'''
pl.plot(xx2,profilex,'.')
pl.plot(xx2[0:25,1],np.polyval(p1,xx2[0:25,1]))
pl.plot(xx2[24:50,1],np.polyval(p2,xx2[24:50,1]))
pl.plot(xx2[45:65,1],np.polyval(p3,xx2[45:65,1]))
pl.plot(xx2[58:117,1],np.polyval(p4,xx2[58:117,1]))
pl.plot(xx2[115:130,1],np.polyval(p5,xx2[115:130,1]))
pl.plot(xx2[126:149,1],np.polyval(p6,xx2[126:149,1]))
pl.plot(xx2[145:,1],np.polyval(p7,xx2[145:,1]))
'''

diff=np.zeros((166,8))
ranges=[0,25,45,58,117,128,146,166]
pall=[p1,p2,p3,p4,p5,p6,p7]
for n in range(len(ranges)-1):
	diff[ranges[n]:ranges[n+1],:] = profilex[ranges[n]:ranges[n+1],:] - np.polyval(pall[n],xx2[ranges[n]:ranges[n+1],:])

diff2=diff/np.polyval(p4,0) # normalize to show relative deviation
#pl.plot(xx2,diff2)


# compare the results from 22.12
diff=( profilex[:,2:] - np.transpose(np.tile( refprofile,(6,1) )) )/np.polyval(p4,0) # calculate the difference to refprofile and normalize

fig=pl.figure(figsize=[6.4*2, 4.8*2]) # default size x 2

pl.plot(xx[:,1],np.nanmin(diff,1),'k', label='minimum and maximum difference to mean')
pl.plot(xx[:,1],np.nanmax(diff,1),'k', label='_nolegend_')

ddiffmin=np.sqrt(pow(drefprofile,2) + pow(np.nanmin(dprofilex,1),2))/np.polyval(p4,0)
ddiffmax=np.sqrt(pow(drefprofile,2) + pow(np.nanmax(dprofilex,1),2))/np.polyval(p4,0)

pl.plot(xx[:,1],np.transpose(-ddiffmin),color='0.5', label='statistical uncertainty')
pl.plot(xx[:,1],np.transpose(ddiffmin),color='0.5', label='_nolegend_')
#pl.plot(xx[:,1],ddiffmax,color='0.5')
pl.xlabel('x (cm)', fontsize=15)
pl.xlim(-0.5,16)
pl.ylabel('difference',fontsize=15)
pl.ylim(-0.008,0.008)
pl.legend()
pl.grid()
pl.savefig('repeated_scans_minmax_difference.pdf')


# Step3: Comapare relative profiles (if needed). 
	





