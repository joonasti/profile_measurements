
/*
Function that runs the measurement via pXar
pXar is run as a command line argument (system("command"))

pXar command syntax:
pXar -d [directory] -r [root file name] -t Xray -p "source=XX;stepseconds=XX;runseconds=XX;filtree=[0 or 1]" -T [trimming value]
e.g.: ../bin/pXar -d ../data/testModule47A/ -r pxar.root -t Xray -p "source=Co-60;stepseconds=10;runseconds=5;filltree=0" -T 50
*/


std::string profile_analyzer::measure(std::string time){

	std::string rootapp = "/home/jot/pxar-raddess/bin/pXar";
	std::string directory ="/home/jot/pxar-raddess/data/tatyana09_acrylic_box_STUK_20211221/"; 
	std::string filename ="Profile_measurement_" + time + ".root";
	std::string logfile ="Profile_measurement_" + time + ".log";
	//filename.erase(std::remove(filename.begin(), filename.end(), ':'), filename.end());
	//filename.erase(std::remove(filename.begin(), filename.end(), '-'), filename.end());
	std::string filename2 = "Profile_measurement_pos_" + std::to_string(position[0]) + "_" 
		+ std::to_string(position[1]) + "_" 
		+ std::to_string(position[2]) + ".root";
	std::replace( filename2.begin(), filename2.end()-5, '.', 'p'); // the floats in position have dots, replace with p, but not the point in .root
	std::string logfile2 = "Profile_measurement_pos_" + std::to_string(position[0]) + "_" 
		+ std::to_string(position[1]) + "_" 
		+ std::to_string(position[2]) + ".log";
	std::replace( logfile2.begin(), logfile2.end()-5, '.', 'p');
	//float runtime = get_time();
	std::string Xray = " -t Xray";
	std::string source = " -p \"source=GBX;";
	std::string stepseconds = "stepseconds=10;";
	std::string runseconds = "runseconds=75;";
	std::string filltree = "filltree=1;";
	std::string emptyevents = "skipemptyevents=1\"";
	std::string trimming = " -T 60";
	
	std::string strcommand = rootapp + " -d " + directory + " -r " + filename + Xray + source + stepseconds + runseconds + filltree + emptyevents+ trimming;
	//char command[strcommand.length()+1];
	//strcpy(command,strcommand.c_str()); // copy string to char format
	
	cout << "Starting the measurement. \n" << "...\n";
	
	//string command2 = strcommand + "\n\n";
	//printf(command2.c_str());
	system(strcommand.c_str()); // Executes a bash command
	
	cout << "Measurement and data writing complete.\n\n";
	
	/*
	// Move the root file to measdir? 
	if(std::rename((directory+filename).c_str(), (measdir+filename).c_str()) < 0) {
    std::cout << strerror(errno) << '\n'; 
    }*/
    
    // Copy the root file to measdir
    std::string copystr = "cp " + directory+filename + " " + measdir+filename2;
	system(copystr.c_str());
    std::string copystr2 = "cp " + directory+logfile + " " + measdir+logfile2;
	system(copystr2.c_str());
	    
	return measdir+filename2;
	

}
