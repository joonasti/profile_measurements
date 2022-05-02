
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include "my_func.h"



bool is_dead(int n, int m, std::vector<int> deadcol, std::vector<int> deadrow, std::vector< std::vector<int> > deadpix){

	for (int i=0; i<deadcol.size(); i++){
		if (n==deadcol[i]) {return true;}
	}
	for (int i=0; i<deadrow.size(); i++){
		if (m==deadrow[i]) {return true;}
	}
	for (int i=0; i<deadpix.size(); i++){
		if (n==deadpix[i][0] && m==deadpix[i][1]) {return true;}
	}
	
	return false;

}


// Function to save and plot the dose profile (or "hit" profile) from profile measurements with 
// the CMS Standard Pixel and DOSCAN
void build_profile(std::string path1){

	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210531_133853/"; 
	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210617_151246/"; 
	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20211013_123516/";  
	std::string* dir1 = list_dir(path1.c_str());
	
	float width=16;
	float height=0.8;
	
	
	TH2I* detector_data = new TH2I("hitmap","col:row:counts",52,0,52, 80,0,80); 
	TH2I* detector_data_q = new TH2I("qmap","col:row:counts",52,0,52, 80,0,80); 
	//TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", (int)(width/0.015), 0, width, (int)(height/0.01), 0, height);
	TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", (int)(width/0.005), 0, width, (int)(height/0.01), 0, height);
	//TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", 1000, 0, 15, 40, 0, 0.4); // 14.01=934*0.015
	//TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", 1000, 0, 15, 10, 0, 0.1); 
	//TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", (int)(width/0.015), 0, width,(int)(height/0.01), 0, height);
	TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", (int)(width/0.005), 0, width,(int)(height/0.01), 0, height);
	//TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", 1000, 0, 15, 40, 0, 0.4); // 14.01=934*0.015
	//TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", 1000, 0, 15, 10, 0, 0.1); // 14.01=934*0.015
	
	// how many alive pixels per row or column in profile
	TH1I* roww = new TH1I("rowweights","rowweights",(int)(height/0.01), 0, height);
	//TH1I* colw = new TH1I("colweights","colweights",(int)(width/0.015), 0, width);
	TH1I* colw = new TH1I("colweights","colweights",(int)(width/0.005), 0, width);

		
	auto canvas1 = new TCanvas;
	auto canvas2 = new TCanvas;
	auto canvas3 = new TCanvas;
	auto canvas4 = new TCanvas;
	auto canvas5 = new TCanvas;
	
	float pixelw=0.015;
	float pixelh=0.01;
	
	std::vector<int> deadrow = {80};
	std::vector<int> deadcol = {1,52};
	//std::vector< std::vector<int> > deadpix = {{20,6}, {20,7}, {26,67}, {4,74}}; // May
	//std::vector< std::vector<int> > deadpix = {{3,72}, {25,66}};;
	std::vector< std::vector<int> > deadpix = {{35,19}}; // October (and December?)
	
	
	// variables for a scatter plot
	int n1 = 52*100;
	Double_t x[n1], y[n1], yq[n1], dx[n1], dy[n1]; 
	
	for (int i=0;i<n1; i++){ // odd behaviour if not initialized
		x[i]=0;
		y[i]=0;
		yq[i]=0;
		dx[i]=0.01;
		dy[i]=0;
	}
	
	
	// Flat field correction from center of the beam
	/*double ffc[52][80];
	ifstream f("ffc_correction_from_python.txt");
	for (int i=0; i<52; i++){
		for (int j=0; j<80; j++){
			f >> ffc[i][j];
		}
	}*/
	/*double ffc[52];
	ifstream f("ffc_correction_from_python.txt");
	for (int i=0; i<52; i++){
		f >> ffc[i];
	}*/
	double val;
	double ffc[52][80];
	for (int i=0; i<52; i++){
		//val=1.02528698 - 1.01147926e-03*i; // May measurements, same in June?
		val=1.03698369 - 0.00147935*i; // October measurements
		for (int j=0; j<80; j++){
			//ffc[j][]=1.02283 - 0.000913269*j; // average parameters from the depth-dose measurements, June measurements?
			ffc[i][j]=val; // value copied to each row. Could be 1D but now there's no need to modify code at multiple places
		}
	}
	
	
	
	// Loop over the files
	int i=0;
	while (dir1[i]!=""){
		
		// separate the filename into parts
		std::stringstream filename(dir1[i]);
		std::string parts;
		std::vector<std::string> filenameparts;
		while(std::getline(filename, parts, '_'))
		{
   			filenameparts.push_back(parts);
		}
		
		if (filenameparts.size()!=6){ 
			i++;
			continue;
		}
		if (filenameparts[2]!="pos"){ 
			i++;
			continue;
		}
		
		// read the position from the file name and convert to float
		std::string posstring = filenameparts[3]; // x
		std::replace(posstring.begin(),posstring.end(),'p','.');
		float position = std::stof(posstring);
		
		// Test: no overlapping of the detector
		/*if (int(round(10*position))%8!=0){
			i++;
			continue;
		}*/
		
		if (path1=="/home/jot/HIP/measurement_files/Profile_scan_20210531_133853/"){ // correct to the actual position
			//cout << "Warning: correcting position" << endl; // paranoia
			int a = int(round(position/0.39));
			if (position>0){ // zero and first position are ok, if motor initially at 0 stage
				position = position + (a-1)*0.01; 
			}
		} 
		
		
		/*if (round(10*position)/10==8.0 || round(10*position)/10==10.0){ // bad data in two files. round(100*x)/100 is a trick to round to two decimal precision
			i++;
			continue;
		} */
		
		// read the root file
		TFile *file = new TFile((path1+dir1[i]).c_str(),"read");
	
		TDirectory *dir = file->GetDirectory("Xray"); assert(topdir);
	  	dir->cd();
  		TTree *tree = (TTree*)dir->Get("events");
  		
  		assert(tree);
  		//tree->Print();
  		cout << "Reading the root file number " << i+1 << ". Number of entries to be processed: " << tree->GetEntries() << endl;
  		cout << "Position is " <<to_string(position) << endl;
  	
		Int_t CELLS = 25;
		UShort_t npix; // s
		UChar_t  roc[CELLS], col[CELLS], row[CELLS]; // b
		Double_t val[CELLS], q[CELLS];
		Int_t eventID = 0;
		
		
		// Note: npix is the number of hit pixels per event, and col, row, etc are arrays of that length
		tree->SetBranchAddress("npix", &npix);
		//tree->SetBranchAddress("proc", &roc);
		tree->SetBranchAddress("pcol", &col);
		tree->SetBranchAddress("prow", &row);
		//tree->SetBranchAddress("pval", &val);
		tree->SetBranchAddress("pq", &q);
		
		int entries = tree->GetEntries();
		
		for (int n=0; n < entries; ++n) {
		
			npix=0;
			tree->GetEntry(n);
			//if (npix<20) {
				for (int j=0; j<npix; ++j){
					detector_data->Fill(col[j]+0.5,row[j]+0.5); // row and col from 0, 0.5 the center of bin 1 in detector_data
					detector_data_q->Fill(col[j]+0.5,row[j]+0.5,q[j]);
					//detector_data_q->Fill(col[j],row[j],val[j]);
				}
			//}
		
		}
		
		//detector_data_q->SetBinContent(20,6,0); // hot pixel
		
		file->Close();
    	file->Delete();
		
		int count=0;
		for (int n=0;n<52;++n) {
		
			x[i*52+n]=position+(n+0.5)*pixelw;
			//if (x[i*52+n]>20){cout << x[i*52+n] << endl; }
		
			for (int m=0;m<80;++m) { // cut the lower half with the hole (m from 40 onwards and inverted)
			//for (int m=0;m<10;++m) { // cut the lower half with the hole (m from 40 onwards and inverted)
				
				// Skip the dead pixels
				if (is_dead(52-n,80-m,deadcol,deadrow,deadpix)){
					continue;
				}
				if (ffc[51-n][79-m]<0.5){
					printf("FFC<0.5. col = %i, row = %i\n",51-n,79-m);
					return;
				}
				
				if (detector_data->GetBinContent(52-n,80-m)>10){ // double check for dead pixels 
				// The detector is "upside down". Therefore pixel (52,80) is the lower left corner of the detector 
					profile->Fill( position+(n+0.5)*pixelw,
						(m+0.5)*pixelh,
						detector_data->GetBinContent(52-n,80-m));
					profile_q->Fill( position+(n+0.5)*pixelw,
						(m+0.5)*pixelh,
						detector_data_q->GetBinContent(52-n,80-m)/ffc[51-n][79-m]); // ffc = flat field correction, essentially calibration from mid of beam
					// Count the number of read values in each row and col
					roww->Fill((m+0.5)*pixelh);
					colw->Fill(position+(n+0.5)*pixelw);
					count+=1;
					
					y[i*52+n]+=detector_data->GetBinContent(51-n,79-m);
					yq[i*52+n]+=detector_data_q->GetBinContent(51-n,79-m)/ffc[51-n][79-m];
				}
			}
			
			dy[i*52+n]=sqrt(y[i*52+n]);
			if (count!=0){
				y[i*52+n]/=count; // normalize per num of alive pixels in the column
				yq[i*52+n]/=count;
				dy[i*52+n]/=count;
			}
			count=0;
		}
		
		TGraphErrors *gr = new TGraphErrors(n1,x,y,dx,dy);
		TGraph *grq = new TGraph(n1,x,yq);
		
		gr->SetTitle("Hit profile; x (cm); counts");
		grq->SetTitle("Charge profile; x (cm); charge (arb.)");
		
		/*
		// "error lines"
		int bin1=gr->FindBin(7); // bin corresponding to x value of 7 cm
		int bin2=gr->FindBin(9); 
		double ave=gr->Integral(bin1,bin2)/(bin2-bin1);
		
		TGraph *gre1 = new TGraph(2,{2,13},{ave - 2*dy[gr->FindBin()],gr->Integral(bin1,bin2)/(bin2-bin1)});
		gre1->LineColor(2);
		*/
		
		gr->SetMarkerStyle(7);
		grq->SetMarkerStyle(7);
		
		canvas1->cd();
		detector_data_q->Draw("Colz");
		//detector_data_q->SetMinimum(250e3);
		//detector_data_q->SetMaximum(detector_data_q->GetMaximum()+10e3);
		canvas2->cd();
		//profile->Draw("Colz");
		gr->Draw("ape");
		canvas3->cd();
		profile->ProjectionX()->Draw("hist");
		canvas4->cd();
		//profile_q->Draw("Colz");
		grq->Draw("ap");
		canvas5->cd();
		profile_q->ProjectionX()->Draw("hist");
		//colw->Draw("hist");
		canvas1->Modified(); canvas1->Update();
		canvas2->Modified(); canvas2->Update();
		canvas3->Modified(); canvas3->Update();
		canvas4->Modified(); canvas4->Update();
		canvas5->Modified(); canvas5->Update();
		
		
		if (dir1[i+1]==""){ break; } // does not clear the detector data histograms on last loop
		
		detector_data->Reset("ICESM");
		detector_data_q->Reset("ICESM");
		
		
		/*std::string ans;
		cout << "Press something to continue, q to quit: ";
		cin >> ans;
		if (ans=="q"){ return; }*/
		
		
		i++;
	
	}
	
	//canvas2->SaveAs("/home/jot/HIP/figures/hit-profile.png");
	//canvas4->SaveAs("/home/jot/HIP/figures/charge-profile.png");
	
	
	// save profile
	
	std::string ofilename =  "saved_profile.root";

	TFile outputFile (ofilename.c_str(),"RECREATE"); // Data is saved to latest created TFile by default
	profile->Write("profile");
	profile_q->Write("profile_q");
	roww->Write("roww");
	colw->Write("colw");
	//gr->Write("gr");
	//grq->Write("grq");

	outputFile.Close();


}
