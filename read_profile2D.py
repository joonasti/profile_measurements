import ROOT as root
import numpy as np
import numpy.polynomial.polynomial as poly
import matplotlib.pyplot as pl
from mpl_toolkits.mplot3d import Axes3D
import math
#from scipy.optimize import fsolve


def hist_to_matrix(histo):
	n=histo.GetNbinsX()
	m=histo.GetNbinsY()
	matrix=np.zeros((n,m))
	for i in range(1,n+1):
		for j in range(1,m+1):
			matrix[i-1,j-1]=histo.GetBinContent(i,j)
	
	return matrix
	
def get_profile(rootfile):
	global x,y,x2,y2,profile,prof2,dprof2,weights
	root.EnableImplicitMT()
	tree = root.TFile(rootfile,"read")#root.TFile("saved_profile.root","read")
	#hist = tree.Get("profile")
	hist = tree.Get("profile_q")
	profile=hist_to_matrix(hist)
	
	
	n=hist.GetNbinsX()
	m=hist.GetNbinsY()
	x=np.zeros(n);
	y=np.zeros(m);
	for j in range(0,n):
		x[j]=float(hist.ProjectionX().GetBinCenter(j+1))
	for j in range(0,m):
		y[j]=float(hist.ProjectionY().GetBinCenter(j+1))
	
	
	# sum 6x10 pixels (0.9mmx1mm).
	prof2=np.zeros((int(n/6),int(m/10)))
	dprof2=np.zeros((int(n/6),int(m/10)))
	weights=np.zeros((int(n/6),int(m/10)))
	x2=np.zeros((int(n/6),int(m/10)))
	y2=np.zeros((int(n/6),int(m/10)))
	sump=0
	weight=0
	yt=0
	j=0
	while j<=m-10:
		i=0
		j2=int(round(j/10))
		while i<n-6:
			i2=int(round(i/6))
			for k in range(j,j+10):
				for l in range(i,i+6):
					if profile[l,k]>10: # skip dead pixels
						sump+=profile[l,k]
						weight+=1 # take into account dead pixels
						
			if weight==0:
				print(i,j,i2,j2,n,m)
			prof2[i2,j2]=sump/weight
			dprof2[i2,j2]=math.sqrt(sump)/weight
			weights[i2,j2]=weight
			#x2[i2]=np.mean(x[i:i+5])
			x2[i2,j2]=np.sum( np.multiply(x[i:i+6], np.sum(profile[i:i+6,j:j+10],1)) )/np.sum(profile[i:i+6,j:j+10]) # i+6th element not included!
			y2[i2,j2]=np.sum( np.multiply(y[j:j+10], np.sum(profile[i:i+6,j:j+10],0)) )/np.sum(profile[i:i+6,j:j+10])
			sump=0
			weight=0
			i+=6
		#y2[j2]=np.mean(y[j:j+9])
		j+=10
	
	x2=np.round(x2,3)
	y2=np.round(y2,3)
				
	pl.ion()
	#pl.plot(x2,np.sum(prof2[:,1:10],1))
	#pl.grid()
	
	fig = pl.figure()
	ax = fig.add_subplot(111, projection='3d')
	ax.scatter(x2,y2,prof2)
	#pl.matshow(weights)
	#pl.colorbar()
	pl.show()
	
	string="2D_charge_profile_October"
	xstring="_x-values.txt"
	ystring="_y-values.txt"
	ustring="_uncertainty.txt"
	
	outx=open(string+xstring,'w')
	outy=open(string+ystring,'w')
	outp=open(string+".txt",'w')
	outdp=open(string+ustring,'w')
	
	
	for n in range(np.size(prof2,1)):
		for m in range(np.size(prof2,0)):
			outx.write(str(x2[m,n]) + ' ')
			outy.write(str(y2[m,n]) + ' ')
			outp.write(str(prof2[m,n]) + ' ')
			outdp.write(str(dprof2[m,n]) + ' ')
		outx.write('\n')
		outy.write('\n')
		outp.write('\n')
		outdp.write('\n')
	
	outx.close()
	outy.close()
	outp.close()
	outdp.close()
	



get_profile("saved_profile_2D_2.root")




