
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
void build_profile2D(){

	
	std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20211019_112851/"; 
	std::string* dir1 = list_dir(path1.c_str());
	
	float width=8;
	float height=8;
	float pixelw=0.015;
	float pixelh=0.01;
	
	// take into account the gaps in x direction
	double xbins[10*52+10];
	for (int i=0; i<10; i++){
		for (int j=0; j<53; j++){ // 53: lower edge of the gap
			xbins[i*53+j]=i*0.8+j*pixelw;
		}
	}
	//TGraph *gr = new TGraph(10*52+10,xtemp,xbins);
	
	
	TH2I* detector_data = new TH2I("hitmap","col:row:counts",52,0,52, 80,0,80); 
	TH2I* detector_data_q = new TH2I("qmap","col:row:counts",52,0,52, 80,0,80); 
	int det[52][80];
	float detq[52][80];
	//TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", (int)(width/0.015), 0, width, (int)(height/0.01), 0, height);
	TH2I* profile = new TH2I("profile", "beam profile;x-bin; y-bin", 10*53-1, xbins, (int)(height/0.01), 0, height);
	//TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", (int)(width/0.015), 0, width,(int)(height/0.01), 0, height);
	TH2I* profile_q = new TH2I("charge-profile", "dose profile;x-bin; y-bin", 10*53-1, xbins,(int)(height/0.01), 0, height);

		
	auto canvas1 = new TCanvas;
	auto canvas2 = new TCanvas;
	auto canvas3 = new TCanvas;
	auto canvas4 = new TCanvas;
	//auto canvas5 = new TCanvas;
	
	/*canvas5->cd();
	gr->Draw("ape");
	canvas5->Modified(); canvas5->Update();
	char ans;
	cin >> ans;*/
	
	/*FFC->Draw();
	char ans;
	cin >> ans;
	return;*/
	
	std::vector<int> deadrow = {80};
	std::vector<int> deadcol = {1,52};
	//std::vector< std::vector<int> > deadpix = {{3,72}, {25,66}};;
	std::vector< std::vector<int> > deadpix = {{35,19}};
	
	
	// Flat field correction from center of the beam, for each pixel
	double ffc[52][80];
	ifstream f("ffc_correction_from_python.txt");
	for (int i=0; i<52; i++){
		for (int j=0; j<80; j++){
			f >> ffc[i][j];
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
		
		// check whether the filename is of correct syntax
		if (filenameparts.size()!=6){ 
			i++;
			continue;
		}
		if (filenameparts[2]!="pos"){ 
			i++;
			continue;
		}
		
		// read the position from the file name and convert to float
		std::string posstringx = filenameparts[3]; // x
		std::replace(posstringx.begin(),posstringx.end(),'p','.');
		std::string posstringy = filenameparts[4]; // x
		std::replace(posstringy.begin(),posstringy.end(),'p','.');
		//std::string posstringz = filenameparts[5]; // x
		//std::replace(posstringz.begin(),posstringz.end(),'p','.');
		float posx = std::stof(posstringx);
		float posy = std::stof(posstringy);
		//float posz = std::stof(posstringz);
		
		TFile *file = new TFile((path1+dir1[i]).c_str(),"read");
		TDirectory *dir = file->GetDirectory("Xray"); assert(topdir);
	  	dir->cd();
  		TTree *tree = (TTree*)dir->Get("events");
  		
  		assert(tree);
  		//tree->Print();
  		cout << "Reading the root file number " << i+1 << ". Number of entries to be processed: " << tree->GetEntries() << endl;

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
			//if (npix>0) {
				for (int j=0; j<npix; ++j){
					if (row[j]==80){ // values with col==80! These are fortunately rare
						//cout << to_string(q[j]) << " Shit!\n"; 
						//return;
					}
					detector_data->Fill(col[j]+0.5,row[j]+0.5); // row and col from 0, 0.5 the center of bin 1 in detector_data
					detector_data_q->Fill(col[j]+0.5,row[j]+0.5,q[j]);
					//det[col[j]-1][row[j]-1]+=1; // col and row start from 0 or 1?
					//detq[col[j]-1][row[j]-1]+=q[j];
					//detector_data_q->Fill(col[j],row[j],val[j]);
				}
			//}
		
		}
		
		//detector_data_q->SetBinContent(20,6,0); // hot pixel
		file->Close();
    	file->Delete();
    	
    	
    	int count=0;
		for (int n=0;n<52;++n) {
			
			//x[i*52+n]=posx+(n+0.5)*pixelw;
			//if (x[i*52+n]>20){cout << x[i*52+n] << endl; }
		
			for (int m=0;m<80;++m) { // cut the lower half with the hole (m from 40 onwards and inverted)
			//for (int m=0;m<10;++m) { // cut the lower half with the hole (m from 40 onwards and inverted)
				
				// Skip the dead pixels
				if (is_dead(52-n,80-m,deadcol,deadrow,deadpix)){
					continue;
				}
				/*if (ffc[51-n][79-m]<0.5){
					printf("FFC<0.5. col = %i, row = %i\n",51-n,79-m);
					return;
				}*/
				
				if (detector_data->GetBinContent(52-n,80-m)>10){ // double check for dead pixels 
				// The detector is "upside down". Therefore pixel (52,80) is the lower left corner of the detector 
					profile->Fill( posx+(n+0.5)*pixelw,
						posy+(m+0.5)*pixelh,
						detector_data->GetBinContent(52-n,80-m));
						//det[51-n][79-m]);
					profile_q->Fill( posx+(n+0.5)*pixelw,
						posy+(m+0.5)*pixelh,
						detector_data_q->GetBinContent(52-n,80-m))/ffc[51-n][79-m]; // detector_data index from 1-->, ffc from 0-->
						//detq[51-n][79-m]);///ffc[51-n][79-m]); // ffc = flat field correction, essentially calibration from mid of beam
					
				}
			}
		}
		
		if(detector_data->ProjectionX()->GetBinContent(0)>0 || detector_data->ProjectionY()->GetBinContent(0)>0){
			cout << "Counts in bin zero!\n";
			return;
		}
			
		canvas1->cd();
		detector_data_q->Draw("Colz");
		//detector_data_q->SetMinimum(250e3);
		//detector_data_q->SetMaximum(detector_data_q->GetMaximum()+10e3);
		canvas2->cd();
		profile->Draw("Colz");
		canvas3->cd();
		profile_q->Draw("Colz");
		canvas4->cd();
		detector_data->Draw("Colz");
		canvas1->Modified(); canvas1->Update();
		canvas2->Modified(); canvas2->Update();
		canvas3->Modified(); canvas3->Update();
		canvas4->Modified(); canvas4->Update();
		
		detector_data->Reset("ICESM");
		detector_data_q->Reset("ICESM");
		
		/*std::string ans;
		cout << "Press something to continue, q to quit: ";
		cin >> ans;
		if (ans=="q"){ return; }*/
		
		i++;
			
			
	}
		
	
	// save profile 
	std::string ofilename =  "saved_profile_2D.root";

	TFile outputFile (ofilename.c_str(),"RECREATE"); // Data is saved to latest created TFile by default
	profile->Write("profile");
	profile_q->Write("profile_q");
	outputFile.Close();
	
}
