//Maggie Peterson 2019

#include "OpenVideo.hpp"
#include "OpenFilter.hpp"
#include "Metrics.hpp"
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <thread>
#include <stdio.h>
#define PORT 58010
#define BCAST_PORT 58012
#define BUFFER_SIZE 1*800*600
using namespace cv;
using namespace std;

int main()
{
	cout <<__LINE__ << endl;
    Mat* raw_img = new Mat;
    Mat* target_img = new Mat;

    Filter myFilter;
    Metrics myMetrics;
            
    const string HSV_file = "HSV.txt";
    const string Metrics_file = "Metrics.txt";
    String data;
    
    //set up UDP
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    char *broadcastIP;                /* IP broadcast address */
    char *sendString;                 /* String to broadcast */
    int broadcastPermission = 1;          /* Socket opt to set permission to broadcast */
    unsigned int sendStringLen;       /* Length of string to broadcast */
    
    cout << "bcast thread start" << endl;
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("UDP socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0)
    {
        perror("Could not set UDP permissions");
        exit(EXIT_FAILURE);
    }
    
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;/* Broadcast IP address */
    broadcastAddr.sin_port = htons(BCAST_PORT);         /* Broadcast port */
    cout << "Starting beacon" << endl;
    
    OpenVideo stream(0); //opens camera stream
    cout << "Capture is opened" << endl;
    myFilter.readHSV(HSV_file);
	myMetrics.readMetrics(Metrics_file);
        cout <<__LINE__ << endl;

   while(1){     //sends distance and angle

       *raw_img = stream.getImage();
	cout<<"image height: "<<raw_img->cols<<" width: "<<raw_img->rows<<endl;
	cout<<"random pixel "<<(int)raw_img->data[67]<<endl;
       *target_img = myFilter.edgeDetect(raw_img);
       myMetrics.TargetInit(target_img);
	imwrite("image.jpg", *target_img);
       data = myMetrics.getAngleAndDistance();
       
       char cData[255]; 
       strcpy(cData, data.c_str());
       sendto(sock, &cData, sizeof(cData), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));

      }

    return 0;

}
    
