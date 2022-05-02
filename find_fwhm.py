
# Copy this from Matlab
# First, get rid of NaN
# find the fit ranges around half values
# return hml,hmr,fwhm

import numpy as np
import numpy.polynomial.polynomial as poly


def find_fwhm(x,y,maxval):
	
	
	n=np.size(y)
	maxn=n
	i=0
	notwarned=True
	while i<maxn:
		if np.isnan(y[i]):
			if notwarned:
				#print("Warning: deleting NaN elements from y")
				print("Warning: Deleting NaN elements from y")
				notwarned=False
			x=np.delete(x,i)
			y=np.delete(y,i)
			maxn-=1
		else:
			i+=1
	
	
	step=np.min([x[2]-x[1],x[3]-x[2]]) # stepsize in x. Takes into account if there were nan values
	elem=int(round(0.5/step))
	i=np.argmin( np.abs(y-maxval/2) )
	if y[i+2]>y[i]: # rising edge
		fitl=poly.polyfit(x[i-elem:i+elem],y[i-elem:i+elem],2)
		roots=np.roots([fitl[2],fitl[1],fitl[0]-maxval/2])
		k=np.argmin(abs(roots-3)) # the root that's closer to +/- 5 cm
		hml=roots[k] # half-maximum left
		
		if y[-1]<maxval/2: # see if there's also a falling edge
			j=np.argmin( np.abs(y[i+10:]-maxval/2) )
			fitr=poly.polyfit( x[j+i+10-elem:j+i+10+elem], y[j+i+10-elem:j+i+10+elem], 2 )
			roots=np.roots([fitr[2],fitr[1],fitr[0]-maxval/2])
			k=np.argmin(abs(roots-13)); # the root that's closer to +/- 5 cm
			hmr=roots[k]
		
	else: # falling edge
		fitr=poly.polyfit(x[i-elem:i+elem],y[i-elem:i+elem],2)
		roots=np.roots([fitr[2],fitr[1],fitr[0]-maxval/2])
		k=np.argmin(abs(abs(roots)-13)) # the root that's closer to +/- 5 cm
		hmr=roots[k]
		
		if y[1]<maxval/2: # see if there's also a rising edge
			j=np.argmin( np.abs(y[1:i-5]-maxval/2) )
			fitl=poly.polyfit( x[j-elem:j+elem], y[j-elem:j+elem], 2 )
			roots=np.roots([fitl[2],fitl[1],fitl[0]-maxval/2])
			k=np.argmin(abs(abs(roots)-3)); # the root that's closer to +/- 5 cm
			hml=roots[k]
	
	
	fwhm=hmr-hml
	
	# note that list can contain multiple data types (not an array)
	return hml,hmr,fwhm
	








