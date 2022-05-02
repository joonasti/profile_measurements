#include "my_func.h"


void print_spectra_to_file(TH1 *spec){

	ofstream ofile("Si_spectrum_3x5_pixels.txt");
	
	for (int i=0; i<spec->GetNbinsX(); i++){
		ofile << to_string(spec->GetBinCenter(i)*50*3.6/1000) << " "; // bin center in keV
		ofile << to_string(spec->GetBinContent(i)) << endl;
	}

}



void plot_spectra(){

	//string file="omat_sahlaykset/testfile.root";
	
	int nbin=90, endbin=5000;
	
	TH1I* spec1 = new TH1I("spec1","spectra;Energy (VCal);counts",nbin,0,endbin);
	TH1I* spec2 = new TH1I("spec2","spec2",nbin,0,endbin);
	TH1I* spec3 = new TH1I("spec3","spec3",nbin,0,endbin);
	TH1I* spec4 = new TH1I("spec4","spec4",nbin,0,endbin);
	
	// has to be declared here?
	//TH1I *spec5;
	//TH1I *spec6;
	
	spec3->SetLineColor(2);
	spec4->SetLineColor(6);
	
	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210617_151246/"; 
	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210531_133853/"; 
	//std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20210618_112430/";
	std::string path1="/home/jot/HIP/measurement_files/Profile_scan_20211013_150008/"; 
	std::string* dir1 = list_dir(path1.c_str());
	
	auto canvas1 = new TCanvas("canv","canv",1000,800);
	auto canvas2 = new TCanvas("canv2","canv2",1000,800);
	
	int i=0;
	//int i=13;
	
	// loop through all the result files in path1
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
		std::string posstring = filenameparts[3]; // x-axis
		//std::string posstring = filenameparts[5]; // z-axis
		
		std::replace(posstring.begin(),posstring.end(),'p','.');
		float position = std::stof(posstring);
		
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
		
		Double_t q1, q2, q3, q4;
		for (int n=0; n < entries; ++n) {
		//for (int n=0; n < 100000; ++n) {
		
			npix=0;
			q1=0; q2=0; q3=0; q4=0;
			
			// get next event
			tree->GetEntry(n);
			
			// score spectra, here four different ones from different parts of the detector
			for (int j=0; j<npix; ++j){
				if (col[j]>=5 && col[j]<=7 && row[j]>=70 && row[j]<=74){
					q1+=q[j];
					//spec1->Fill(q[j]);
				}
				if (col[j]>=5 && col[j]<=7 && row[j]>=41 && row[j]<=45){
					q2+=q[j];
					//spec2->Fill(q[j]);
				}
				//if (col[j]>=24 && col[j]<=26 && row[j]>=41 && row[j]<=45){
				if (col[j]>=20 && col[j]<=30 && row[j]>=41 && row[j]<=50){
					q3+=q[j];
					//spec3->Fill(q[j]);
				}
				if (col[j]>=47 && col[j]<=49 && row[j]>=5 && row[j]<=9){
					q4+=q[j];
					//spec4->Fill(q[j]);
				}
			}
			if (q1>0){spec1->Fill(q1);}
			if (q2>0){spec2->Fill(q2);}
			if (q3>0){spec3->Fill(q3);}
			if (q4>0){spec4->Fill(q4);}
		
		}
		
		//detector_data_q->SetBinContent(20,6,0); // hot pixel
		
		printf("Position is %f\n",position);
		printf("%f %f %f %f\n",spec1->Integral(),spec2->Integral(),spec3->Integral(),spec4->Integral());
		
		canvas1->cd();
		/*spec1->Draw("hist");
		spec1->SetMaximum(spec1->GetMaximum()*1.2);
		spec2->Draw("same");
		spec3->Draw("same");
		spec4->Draw("same");*/
		spec3->Draw("hist");
		canvas1->Modified(); canvas1->Update();
		
		/*
		// Comapre spectra at the edges and at the center of the beam
		if (round(10*position)/10 == 0){
			TH1I *spec5 = (TH1I*) spec3->Clone();
			spec5->SetName("spec5");
			spec5->SetLineColor(2);
			cout << spec5->Integral() << endl;
			spec5->Scale(10000/spec5->Integral());
			cout << spec5->Integral() << endl;
			canvas2->cd();
			spec5->Draw("hist");
			canvas2->Modified(); canvas2->Update();
		}
		if (round(10*position)/10 == 14){
			TH1I *spec6 = (TH1I*) spec3->Clone();
			spec6->SetName("spec6");
			spec6->SetLineColor(6);
			cout << spec6->Integral() << endl;
			spec6->Scale(10000/spec6->Integral());
			cout << spec6->Integral() << endl;
			spec6->SetMaximum(spec6->GetMaximum()*1.5);
			canvas2->cd();
			spec6->Draw("same");
			canvas2->Modified(); canvas2->Update();
		}*/
		
		
		
		std::string ans;
		cout << "Press something to continue, s to save figure, p to print to file, q to quit: ";
		cin >> ans;
		if (ans=="c"){
			TH1I *spec5 = spec3;
		}
		
		if (ans=="s"){
			canvas1->SaveAs("/home/jot/HIP/figures/spectra.png");
		}
		if (ans=="p"){
			print_spectra_to_file(spec3);
		}
		if (ans=="q"){ return; }
		
		spec1->Reset("ICESM");
		spec2->Reset("ICESM");
		spec3->Reset("ICESM");
		spec4->Reset("ICESM");
		
		i++;
	
	}
	
	

}
