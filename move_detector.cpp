#include <iostream>
#include <fstream>
#include <string.h>

int profile_analyzer::is_paused(){

	std::string ans;
	std::fstream pausefile;
	pausefile.open("/home/jot/HIP/paused.txt",std::ios::in);
	if (pausefile.is_open()){   //checking whether the file is open
		string tp;
		getline(pausefile, tp);
		pausefile.close(); //close the file object.  
		    
		system("./unpause.sh"); // write 'n' to the pause.txt
		
		if (tp=='y'){ 
      		printf("The program has been paused.");
      		printf("Do you wish to continue (y) or terminate the program (n)?: ");
      		cin >> ans;
      		if (ans=='y'){
      		
      		}
      		else if (ans=='n'){
      			return 0;
      		}
    	}
	
	}
	else {
		cout << "Could not read the pause file." << endl;
		cout << "Do you wish to continue? (y/n): ";
		cin  >> ans;
		if (ans=='n'){ 
			return 0;
		}
	}
	
	return 1;
}


/*
Function to send command to move the detector to Raspberry.
The message is of the form "x,y,z", where the variables are the displacement
of the detector in horizontal, vertical and beam axis directions in mm (can be negative)
*/
int profile_analyzer::move_detector(float x, float y, float z){

	// Check whether pausing the program has been requested
	if (is_paused()==0){
		return 0;
	}
	
	//cout << ("Press something to continue: ");
	//std::string ss;
	//cin >> ss;
	
	std::string str = std::to_string(x) + "," + std::to_string(y) + "," 
		+ std::to_string(z);
	char message[str.length()+1];
	strcpy(message,str.c_str()); // copy string to char format
	printf("Moving detector by: %s ...\n",message);
	int n = write(newsockfd,message,strlen(message));
	char done[]="Y";
	if (n<0) {
		error("ERROR writing to socket");
		return 0;
	}
	//while (1<2) { // while loop to ensure that detector has been moved before continuing
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n<0) {
		error("ERROR reading from socket");
		return 0;
	}
		//if (n>0) {break;}

	if (buffer[0]!=done[0]){ // if the message received is not 'Y'
		printf("Ok signal not read from client. Buffer = %s\n",buffer);
		return 0;
	}
	
	printf("Done.\n\n");
	return 1;

}
