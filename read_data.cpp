
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"

// Most of this is copied from Eriks reprocesstree.C

void profile_analyzer::read_data(const char *filename){


	//if (flag==1){ return; } // why is this here?

	TFile *file = new TFile(filename,"read");
	
	TDirectory *dir = file->GetDirectory("Xray"); assert(topdir);
  	dir->cd();
  	TTree *tree = (TTree*)dir->Get("events");
  	
  	assert(tree);
  	//tree->Print();
  	cout << "Reading the root file. Number of entries to be processed: " << tree->GetEntries() << endl;
  
	Int_t CELLS = 25;
	UShort_t npix; // s
	UChar_t  roc[CELLS], col[CELLS], row[CELLS]; // b
	Double_t val[CELLS], q[CELLS];
	Int_t eventID = 0;
	
	tree->SetBranchAddress("npix", &npix);
	tree->SetBranchAddress("proc", &roc);
	tree->SetBranchAddress("pcol", &col);
	tree->SetBranchAddress("prow", &row);
	tree->SetBranchAddress("pval", &val);
	tree->SetBranchAddress("pq", &q);
	
	int entries = tree->GetEntries();
	
	for (int i=0; i < entries; ++i) {
	
		npix=0;
		tree->GetEntry(i);
		if (npix>0) {
			for (int j=0; j<npix; ++j){
				detector_data->Fill(col[j],row[j]);
				//detector_data->Fill(col[j],row[j],q[j]);
			}
		}
	
	}
	//flag=1; // WTF?
	file->Close();
    file->Delete();
	
}
