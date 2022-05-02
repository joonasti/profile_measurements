


void profile_analyzer::read_fake_data(TRandom *r){
	
	for (int i=1;i<=52;++i) {
		for (int j=1;j<=80;++j) {
			//detector_data->SetBinContent(i,j,(int) (position[0]*100+position[1]*100+position[2]*100+1));
			//detector_data->SetBinContent( i, j,( (position[0]*100+1)*(position[1]*100+1) )*r->Integer(10) );
			detector_data->Fill(i,j,i+j);
		}
	}
	

};
