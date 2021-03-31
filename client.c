#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/time.h>

#define PORT 8888
#define MSG_SIZE 500
#define MB_TO_BYTES 1048576
#define DOWNLOADED_FILENAME "download.txt"
#define TRANSMISSION_RATES_FILENAME "transmission_rates.txt"


double value (struct timeval a) {

    double val = a.tv_sec + a.tv_usec*1e-6;
    return val;

}

void write_file(int socket_fd, struct timeval begin) {

    FILE *fp1;
    char *filename1 = DOWNLOADED_FILENAME;

    FILE *fp2;
    char *filename2 = TRANSMISSION_RATES_FILENAME;

    char buffer[MSG_SIZE];

    fp1 = fopen(filename1, "w");
    fp2 = fopen(filename2, "w");

    struct timeval interval_end, interval_start;

    //initialize interval_start;
    interval_start.tv_sec = 0;
    interval_start.tv_usec = 0;

    double time_taken, data_received, transmission_rate, tick = 0.1, total_data = 0;

    double n, interval_end_val;

    while(1) {

        n = recv(socket_fd, buffer, MSG_SIZE, 0);
        total_data += n;

        gettimeofday(&interval_end, NULL);

        interval_end.tv_sec = interval_end.tv_sec - begin.tv_sec;
        interval_end.tv_usec = interval_end.tv_usec - begin.tv_usec;

        long seconds = interval_end.tv_sec - interval_start.tv_sec;
        long microseconds = interval_end.tv_usec - interval_start.tv_usec;
        time_taken = seconds + microseconds*1e-6;

        interval_end_val = value (interval_end);

        while (tick <= interval_end_val) {

            transmission_rate = (total_data/MB_TO_BYTES) / (time_taken);

            fprintf(fp2, "%f %f\n", tick, transmission_rate);
            
            tick += 0.1;

            if(tick + 0.1 > interval_end_val) {

                interval_start.tv_sec = interval_end.tv_sec;
                interval_start.tv_usec = interval_end.tv_usec;

                total_data = 0;
            }
            
        }
    
        if(n < 0) {
            perror("read error");
            close(socket_fd);
            exit(1);
        }
        else if (n == 0) { 
            printf("server closed connection\n");
            close(socket_fd);
            exit(1);
        }

        // end of file reached
        if ((strcmp(buffer, "END")) == 0) { 
            break; 
        }

        fprintf(fp1, "%s", buffer);
        bzero(buffer, MSG_SIZE);

    }

    fclose(fp1);
    fclose(fp2);
    
    return;

}


void chat(int socket_fd) { 

	char client_message[100];
	char server_message[100];
	
	// Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
	
	int n; 
	
	while(1) { 
		
		//get message to be sent to server as input
		printf("Enter message : ");
        scanf("%[^\n]",client_message);
        getchar();

        if ((strcmp( client_message, "GivemeyourVideo")) == 0) {

            send(socket_fd, client_message, sizeof(client_message), 0);

            struct timeval begin;
            gettimeofday(&begin, NULL);

            printf("file downloading....\n");
            write_file(socket_fd, begin);
            printf("file downloaded\n");
            
        } else {

            send(socket_fd, client_message, sizeof(client_message), 0);

            if ((strcmp(client_message, "Bye")) == 0) { 
                close(socket_fd);
                printf("CONNECTION WITH SERVER CLOSED\n"); 
                break; 
            } 

            recv(socket_fd, server_message, sizeof(server_message), 0); 
            printf("Server : %s\n", server_message); 

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

// server address and port info set up
struct sockaddr_in  addServerDetails() {

    struct sockaddr_in server_address;

	// fill the block of memory with value 0
	memset(&server_address, 0, sizeof(server_address));   


	// Server details :

	// internet protocol 4 (AF_INET) 
	server_address.sin_family = AF_INET; 
	
	// address port (16 bits)
	// htons() function translates a short integer from host byte order to network byte order
	server_address.sin_port = htons(PORT); 
	
	// ip address (32 bits)
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    return server_address;

}

int main() { 

    int socket_fd =  createSocket();

    struct sockaddr_in server_address = addServerDetails(); 

    // connect the client socket to server socket 
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) { 
        printf("connection to the server failed\n"); 
        exit(0); 
    } 
    
    printf("CONNECTED TO THE SERVER...\n"); 

    // chat between server and client
    chat(socket_fd); 

	// close the socket 
	close(socket_fd); 
} 
