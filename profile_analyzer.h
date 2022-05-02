
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

class profile_analyzer{

	private:
	
	float beam_width=10; // dimensions in cm
	float beam_height=10;
	float min_depth=2;
	float max_depth=10;
	
	//float detector_width=0.78;
	//float detector_height=0.8;
	int pixelsw=52;
	int pixelsh=80;
	float pixelw=0.015;
	float pixelh=0.01;
	float detector_width=pixelsw*pixelw;
	float detector_height=pixelsh*pixelh;
	
	float stepsizex=0.4; // stepsize of the detector in cm
	float stepsizey=0.8; // 
	float depthstep=1; // 1 cm
	
	int stepsx = (int)(beam_width/stepsizex); 
	int stepsy = (int)(beam_height/stepsizey);
	int stepsz = (int) ((max_depth-min_depth)/depthstep)+1;
	
	//float measurement_time=0;
	
	public:
	
	std::string measdir;
	
	float position[3] = {0, 0, 0}; 
	
	// server connection related variables
	int sockfd, newsockfd, portno=2001;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int char_read; // number of characters read or written
    // 
	
	int flag=0;
	TH2I *detector_data=new TH2I("data", "data;row;column", pixelsw,1,pixelsw+1, 
		pixelsh,1,pixelsh+1);
	/* TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)", 
		pixelsw + pixelsw*stepsizex*stepsx,1,pixelsw + pixelsw*stepsizex*stepsx+1, 
		pixelsh + pixelsh*stepsizey*stepsy,1,pixelsh + pixelsh*stepsizey*stepsy+1,
		stepsz,1,stepsz+1); // rescale the axis to actual dimensions in the end */
	TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)",
		int(beam_width/pixelw + 1),0,int(beam_width/pixelw + 1)*pixelw,
		int(beam_height/pixelh +1),0,int(beam_height/pixelh +1)*pixelh,
		stepsz,1,stepsz+1);
	
	// #############  Functions #################
	
	// Update the histograms to new values for the constants
	void update(){
		stepsx = (int)(beam_width/stepsizex);
		stepsy = (int)(beam_height/stepsizey);
		stepsz = (int) ((max_depth-min_depth)/depthstep)+1;
		
		if (stepsx==0){stepsx=1;}
		if (stepsy==0){stepsy=1;}
		
		delete profile;
		/*TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)", 
			pixelsw + pixelsw*stepsizex*stepsx,1,pixelsw + pixelsw*stepsizex*stepsx+1, 
			pixelsh + pixelsh*stepsizey*stepsy,1,pixelsh + pixelsh*stepsizey*stepsy+1,
			stepsz,1,stepsz+1); // rescale the axis to actual dimensions in the end*/
		TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)",
			int(beam_width/pixelw + 1),0,int(beam_width/pixelw + 1)*pixelw,
			int(beam_height/pixelh +1),0,int(beam_height/pixelh +1)*pixelh,
			stepsz,1,stepsz+1);
	}
	
	float get_min_depth(){return min_depth;};
	void set_beam_width(float x){beam_width=x; update();}
	void set_beam_height(float y){beam_height=y; update();}
	void set_depth(float min, float max){
		min_depth=min;
		max_depth=max;
		profile_analyzer::update();
	}
	
	float get_beam_width(){return beam_width;}
	float get_beam_height(){return beam_height;}
	float get_stepsizex(){return stepsizex;}
	float get_stepsizey(){return stepsizey;}
	
	void set_stepsize(float x, float y,float z){
		stepsizex=x;
		stepsizey=y;
		depthstep=z;
		profile_analyzer::update();
	}
	
	int * get_steps(){
		static int stp[3] = {stepsx, stepsy, stepsz};
		return stp;
	}
	
	void set_steps(int i, int j, int k){
		if (i>0) {stepsx=i; beam_width = (stepsx-1)*stepsizex + detector_width;}
		if (j>0) {stepsy=j; beam_height = (stepsy-1)*stepsizey + detector_height;}
		if (k>0) {stepsz=k;}
		delete profile;
		/*TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)", 
			pixelsw + pixelsw*stepsizex*stepsx,1,pixelsw + pixelsw*stepsizex*stepsx+1, 
			pixelsh + pixelsh*stepsizey*stepsy,1,pixelsh + pixelsh*stepsizey*stepsy+1,
			stepsz,1,stepsz+1); // rescale the axis to actual dimensions in the end*/
		TH3I *profile = new TH3I("profile", "beam profile;x-bin; y-bin;z (cm)",
			int(beam_width/pixelw + 1),0,int(beam_width/pixelw + 1)*pixelw,
			int(beam_height/pixelh +1),0,int(beam_height/pixelh +1)*pixelh,
			stepsz,1,stepsz+1);
	}
	
	//void set_time(float t){measurement_time=t;}
	//float get_time(){return measurement_time;}
	
	void error(const char *msg)
	{
    	perror(msg);
    	exit(1);
	}
	
	
	profile_analyzer(){};
	void beam_profile_analysis();
	int connect_to_raspberry();
	std::string measure(std::string time);
	int is_paused();
	int move_detector(float x, float y, float z);
	void read_data(const char *filename);
	void read_fake_data(TRandom *r);
	bool print_profile_to_file(std::string filename);
	void end_measurement();
	void save_TH3();
	//std::string currentDateTime();
	
	void close_connection(){
		printf("Closing connection to Raspberry. Goodbye!\n");
		char closing_message[]="x";
		write(newsockfd,closing_message,1);
		close(newsockfd);
    	close(sockfd);
	}
	
	ClassDef(profile_analyzer,1);

};
