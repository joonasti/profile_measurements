

int profile_analyzer::connect_to_raspberry(){

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
        return 0;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); // portno is an int, member of profile_analyzer class
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
		return 0;
		}
    printf("Wating for the client to connect to port number %i\n",portno);
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, 
                &clilen);
    if (newsockfd < 0) {
         error("ERROR on accept");
         return 0;
    }
    printf("Connection established, waiting for a message from client: \n");
    bzero(buffer,256);
	char_read = read(newsockfd,buffer,255);
    if (char_read < 0) {error("ERROR reading from socket"); return 0;}
    printf("Message received from client: '%s' \n\n",buffer);
	
	
	return 1;
}
