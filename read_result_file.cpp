

#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include "my_func.h"

// Read a single profile measurement (or some other pXar measurement) file and save the detector hit map and charge map into a file called temp.root
void read_result_file(std::string filename){

	
	TH2I* detector_data = new TH2I("hitmap","col:row:counts",52,0,52, 80,0,80); 
	TH2I* detector_data_q = new TH2I("qmap","col:row:counts",52,0,52, 80,0,80); 
	
	// read the root file
	TFile *file = new TFile((filename).c_str(),"read");
	TDirectory *dir = file->GetDirectory("Xray"); assert(topdir);
 	dir->cd();
  	TTree *tree = (TTree*)dir->Get("events");
  	
  	assert(tree);
  	//tree->Print();
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
				detector_data->Fill(col[j]+0.5,row[j]+0.5); // row and col from 0, 0.5 the center of bin 1 in detector_data
				detector_data_q->Fill(col[j]+0.5,row[j]+0.5,q[j]);
				//detector_data_q->Fill(col[j],row[j],val[j]);
			}
		//}
	
	}
	
	//detector_data_q->SetBinContent(20,6,0); // hot pixel
	
	file->Close();
    file->Delete();
    
    std::string ofilename =  "temp.root";

	TFile outputFile (ofilename.c_str(),"RECREATE"); // Data is saved to latest created TFile by default
	detector_data->Write("detector_data");
	detector_data_q->Write("detector_data_q");
	
	outputFile.Close();
	
	
	

}




