#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT 8888
#define MSG_SIZE 500 
#define SERVER_RESPONSE "Received your message. Thank you!"
#define SAMPLE_FILENAME "sample.txt"


void timedelay() {
	int time=0;
	while(time <= 3000) {
		time++;
	}
}


void send_file(int socket_fd) {

    int n;

    FILE *fd;
    char *filename = SAMPLE_FILENAME;
    char buffer[MSG_SIZE];

    bzero(buffer, MSG_SIZE);

    fd = fopen(filename, "r");

    while(fgets(buffer, MSG_SIZE, fd) != NULL) {

        if (send(socket_fd, buffer, sizeof(buffer), 0) == -1) {

            perror("Failed to send file. Please try again.");
            exit(1);
        }

        bzero(buffer, MSG_SIZE);
    
    }

    strcpy(buffer, "END");
    send(socket_fd, buffer, sizeof( buffer), 0);

	timedelay();
    
}

void chat(int socket_fd) 
{ 
	char client_message[100];
	char server_message[100];
	
	// Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    strcpy(server_message, SERVER_RESPONSE);
	int n; 
	
	while(1) { 
		
		recv(socket_fd, client_message, sizeof( client_message), 0); 
		printf("Client : %s\n", client_message); 

        if ((strcmp( client_message, "GivemeyourVideo")) == 0) {

            printf("file transfer initiated....\n");
            send_file(socket_fd);
            printf("file transfer complete\n");
            
        } else {

            if ((strcmp(client_message, "Bye")) == 0) { 
                close(socket_fd);
                printf("CONNECTION WITH CLIENT CLOSED\n"); 
                break; 
            } 
            send(socket_fd, server_message, sizeof( server_message), 0);

        }
	} 
} 


// creates a new socket
int createSocket() {

    int socket_fd, connection_fd; 
	
    // create socket:
	socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	
	if (socket_fd == -1) { 
		printf("socket creation failed\n"); 
		exit(0); 
	} 

    return socket_fd;

}


void startServer(int socket_fd) {

	struct sockaddr_in server_address, client_address; 

	// fill the block of memory with value 0
	memset(&server_address, 0, sizeof(server_address)); 

	//Server details :

	// Internet protocol v4(AF_INET) 
	server_address.sin_family = AF_INET; 
	
	// Address port (16 bits)
	// htons() function translates a short integer from host byte order to network byte order
	server_address.sin_port = htons(PORT); 
	
	// Internet address (32 bits)
	// INADDR_ANY : bind a socket to all IPs of the machine
	server_address.sin_addr.s_addr = INADDR_ANY; 

	int flag = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
		perror("error in setsockopt");
		exit(1);
	}

	// Binding newly created socket to given IP and verification 
	if ((bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address))) != 0) { 
		printf("socket bind failed\n"); 
		exit(0); 
	} 

	// Now server is ready to listen and verification 
	if ((listen(socket_fd, 5)) != 0) { 
		printf("listening failed\n"); 
		exit(0); 
	} 

}


int main() 
{ 
	int socket_fd = createSocket();

	startServer(socket_fd);

	printf("SERVER LISTENING AT PORT 8888 \n"); 

	struct sockaddr_in client_address; 
	int len = sizeof(client_address); 

	// Accept connection from client
	int	connection_socket_fd = accept(socket_fd, (struct sockaddr *)&client_address, &len); 

	printf("CONNECTED TO A CLIENT \n");

	if (connection_socket_fd < 0) { 
		printf("server failed to acccept connection\n"); 
		exit(0); 
	} 

	// chat between server and client
	chat(connection_socket_fd); 

	// After chatting close the socket 
	close(socket_fd); 
} 
