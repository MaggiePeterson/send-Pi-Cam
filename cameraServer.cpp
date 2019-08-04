//Maggie Peterson 2019

#include "OpenVideo.hpp"
#include "OpenFilter.hpp"
#include "Metrics.hpp"
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <thread>

#define PORT 5801
#define BUFFER_SIZE 1*800*600
using namespace cv;
using namespace std;

int main()
{
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
    int broadcastPermission = 1;          /* Socket opt to set permission to broadcast */

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
    broadcastAddr.sin_port = htons(PORT);         /* Broadcast port */
    cout << "Starting beacon" << endl;
    
    OpenVideo stream(0); //opens camera stream
    cout << "Capture is opened" << endl;

	//read from files
    myFilter.readHSV(HSV_file);
    myMetrics.readMetrics(Metrics_file);    
    
   while(1){     //sends distance and angle

       *raw_img = stream.getImage();
       *target_img = myFilter.edgeDetect(raw_img);
       myMetrics.TargetInit(target_img);
       data = myMetrics.getAngleAndDistance();
	
	imwrite("image.jpg", *target_img);
     
       char cData[255]; 
       strcpy(cData, data.c_str());
       sendto(sock, &cData, sizeof(cData), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));	

      }

    return 0;

}
    
