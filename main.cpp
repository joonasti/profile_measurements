#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <sys/stat.h>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include "profile_analyzer.h"
#include "measure.cpp"
#include "read_fake_data.cpp"
#include "read_data.cpp"
#include "connect_to_raspberry.cpp"
#include "move_detector.cpp"
#include "print_profile_to_file.cpp"

/*
Improvements:
 Plot results after each measurement
 Output from pXar to separate log file
 double output trimming
 
*/

ClassImp(profile_analyzer);

// Get current date/time, format is YYYY-MM-DD_HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);

    return buf;
}

void profile_analyzer::save_TH3() {

	
	std::string ofilename = measdir + "3D_profile_" + currentDateTime()+".root";

	TFile outputFile (ofilename.c_str(),"CREATE"); // Data is saved to latest created TFile by default
	profile->Write();
	outputFile.Close();
	
	return;

}


void profile_analyzer::end_measurement() {

	// Draw something
	//auto canvas = new TCanvas;
	//detector_data->Draw("Colz");
	//profile->Project3D("yx")->Draw("Colz");

	// Save the profile (TH3I)
	save_TH3();
	
	// Print the profile into an ascii file
	std::string txtfilename = measdir + "profile_in_ascii" + currentDateTime()+".txt";
	print_profile_to_file(txtfilename);
	
	std::string msg;
	while (true){
		cout << "Do you want to move the detector to the starting position? (y/n): ";
		cin >> msg;
		if ( msg=='y' || msg=='Y'){
			if (move_detector(-position[0],-position[1],-position[2])==0){
				printf("Could not move detector. \n");
				break;
			}
			else{
				break;
			}
		}
		else if ( msg=='n' || msg=='N'){
			break;
		}
		else {
			cout << "Invalid command. ";
		}
	}
	
	close_connection(); // in profile_analyzer.h
	
	return;

}


// ########################################

// This function does most of the work and calls functions from other files. The main function is mostly used to call this function
void profile_analyzer::beam_profile_analysis() {

	printf("\n\n\n######## Welcome! Let's measure some profiles! #########\n\n\n");
	
	if (connect_to_raspberry()==0) {
		printf("Could not connect to Raspberry. Terminating program.");
		return;
	}

	
	set_beam_width(16);
	set_beam_height(0.8);
	set_depth(1,1);
	set_stepsize(0.8,0.8,1);
	//set_steps(1,1,41);
	
	// stepsize of the movement of the detector in each direction (cm)
	//float incx = stepsizex*detector_width; // = detector_width/2 = 0.39
	float incx = get_stepsizex(); // = detector_height/2 = 0.4
	float incy = get_stepsizey(); // = detector_width/2 = 0.39
	
	int *stp;
	stp=get_steps();
	std::string msg;
	while (true){
		printf("The number of steps is (%i,%i,%i).\n", stp[0],stp[1],stp[2]);
		printf("The scan width is (%f,%f,%d).\n", get_beam_width(),get_beam_height(),stp[2]);
		printf("Has the detector been positioned, and do you wish to continue (y/n)?: ");
		cin >> msg;
		if ( msg=='y' || msg=='Y'){
			break;
		}
		else if ( msg=='n' || msg=='N'){
			close_connection();
			return;
		}
		else {
			cout << "Invalid command. ";
		}
	}
	
	// New directory where all the files are saved (measdir public member of profile_analyzer)
	measdir = "/home/jot/HIP/measurement_files/Profile_scan_" + currentDateTime() + "/";
	if (mkdir(measdir.c_str(),S_IRWXU)==-1){
		printf("Unable to create directory\n"); 
        exit(1); 
        }
    else { 
        printf("Directory %s created.\n\n",measdir.c_str()); 
    } 
	
	
	auto canvas1 = new TCanvas;
	auto canvas2 = new TCanvas;
	auto canvas3 = new TCanvas;
	TRandom *r=new TRandom();
	
	std::string file;
	
	for (int scanz=1; scanz<=stepsz; ++scanz) {
		
		for (int scany=1; scany<=stepsy; ++scany) {
		
			for (int scanx=1; scanx<=stepsx; ++scanx) {
			
				
				file = measure(currentDateTime());
				read_data(file.c_str());
				//read_data("../data/47a_stdpix_water_20210528/pxar.root");
				//read_fake_data(r);
				for (int i=1;i<=pixelsw;++i) {
					for (int j=1;j<=pixelsh;++j) {
						
						// The detector is "upside down". Therefore pixel (52,80) is the lower left corner of the detector 
						/*
						profile->Fill( position[0]/pixelw+i,
							position[1]/pixelh+j,
							scanz,
							detector_data->GetBinContent(53-i,81-j));
						*/
						profile->Fill( position[0]+(i-0.5)*pixelw,
							position[1]+(j-0.5)*pixelh,
							scanz,
							detector_data->GetBinContent(53-i,81-j));
							
					}
				}
				
				canvas1->cd();
				detector_data->Draw("Colz");
				canvas2->cd();
				profile->Project3D("yx")->Draw("Colz");
				//profile->Project3D("yz")->Draw("Colz");
				canvas3->cd();
				profile->Project3D("x")->Draw("hist p");
				canvas1->Modified(); canvas1->Update();
				canvas2->Modified(); canvas2->Update();
				canvas3->Modified(); canvas3->Update();
				detector_data->Reset("ICESM");
				if (scanx<stepsx) { // Don't move the detector on last iteration
					if (move_detector(incx,0,0)==0){
						printf("Could not move detector. Terminating program. \n");
						end_measurement();
						return;
					}
					//printf("Current detector position is (%f %f %f)\n", position[0], position[1], position[2]);
					position[0]+=incx;
				}
				
			}
			
			
			incx=-incx; // move to opposite direction on next y loop
			
			if (scany<stepsy) { // Don't move the detector on last iteration
				if (move_detector(0,incy,0)==0){
					printf("Could not move detector. Terminating program. \n");
					end_measurement();
					return;
				}
				position[1]+=incy; // Remember to check that coordinate systems are in sync
				
			}
			
		}
		
		incy=-incy; // move to opposite direction on next z loop
		
		if (scanz<stepsz) { // Don't move the detector on last iteration
			
			if (move_detector(0,0,depthstep)==0){
				printf("Could not move detector. Terminating program. \n");
				end_measurement();
				return;
			}
			position[2]+=depthstep;
			
		}
		
	}
	
	cout << "Scan completed!" << endl;
	
	// Ask wheather to move detector to starting position or not
	
	
	
	end_measurement();
	
	
	
	/*
	cout << profile->GetBinContent(1,1,1)<< endl;
	cout << profile->GetBinContent(1,40,1)<< endl;
	cout << profile->GetBinContent(1,41,1)<< endl;
	cout << profile->GetBinContent(26,1,2)<< endl;
	cout << profile->GetBinContent(27,1,2)<< endl;
	cout << profile->GetBinContent(26,40,1)<< endl;
	cout << profile->GetBinContent(27,41,1)<< endl;
	cout << profile->GetBinContent(100,100,2)<< endl;
	*/
	

}


// ####################################################




int main() {
	
	auto t1 = std::chrono::high_resolution_clock::now();

	profile_analyzer analyzer;
	analyzer.beam_profile_analysis(); // this is where the magic happens
	
	auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();
    cout << "Excecution time: " << duration << " s" << endl;
    
    
    
	return 0;

}
