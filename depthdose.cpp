
#include "my_func.h"



void print1(TH1 *hist, std::string filename){

	ofstream ofile(filename);
	
	for (int i=1; i<hist->GetNbinsX(); i++){
		ofile << to_string(i-1) << " "; // depth in cm
		ofile << to_string(hist->GetBinContent(i)) << endl;
	}

}

void depthdose(){

	std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210618_130741/"; 
	std::string* dir1 = list_dir(path1.c_str());
	
	
	TH1I *d = new TH1I("depthdose","depthdose;z (cm);count",20,0,19);
	TH1I *dq = new TH1I("depthdose_q","depthdose;z (cm);count",20,0,19);
	TH1D *colq = new TH1D("column_profile","",52,1,52);
	auto canvas1 = new TCanvas;
	auto canvas2 = new TCanvas;
	
	int i=0;
	//for (int i=0; i < 100; i++){ // sizeof(dir1)/sizeof(dir1[0]) length of the string array.
	while (dir1[i]!=""){
		
		// separate the filename into parts
		std::stringstream filename(dir1[i]);
		std::string parts;
		std::vector<std::string> filenameparts;
		while(std::getline(filename, parts, '_'))
		{
   			filenameparts.push_back(parts);
		}
		
		if (filenameparts.size()!=6){ // Only the measurements files are processed
			i++;
			continue;
		}
		
		// read the position from the file name and convert to float
		std::string posstring = filenameparts[5];
		std::replace(posstring.begin(),posstring.end(),'p','.');
		int position = int(round(std::stof(posstring)));
		
		
		// read the root file
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
				for (int j=0; j<npix; ++j){
					//if (col[j]==20 && row[j]==6){continue;} // hot pixel
					if (row[j]<40){continue;} // due to the hole in the detector
					d->Fill(position);
					dq->Fill(position,q[j]);
					colq->Fill(col[j],q[j]);
				}
		
		}
		
		TFile outputFile ("column_q_data.root","RECREATE"); // Data is saved to latest created TFile by default
		colq->Write("colq");
		outputFile.Close();
		
		std::string command = "python flat_field_correction.py " + to_string(position);
		system(command.c_str()); // fits a line to the column data, point is to see how the tangent changes with depth
		
		colq->Reset("ICESM");
		
		i++;
	}
	
	canvas1->cd();
	d->Draw();
	canvas2->cd();
	dq->Draw();
	canvas1->Modified(); canvas1->Update();
	canvas2->Modified(); canvas2->Update();
	
	print1(d,"depth-dose-hit.txt");
	print1(dq,"depth-dose-charge.txt");


}
