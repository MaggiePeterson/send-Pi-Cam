#include "opencv2/core.hpp"
include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "OpenVideo.hpp"
#include "OpenFilter.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <thread>

#define PORT 8888
#define BCAST_PORT 9999
#define BUFFER_SIZE 1*800*600
using namespace cv;
using namespace std;

void bCastThread(void)
{
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
    /*    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &broadcastPermission, sizeof(broadcastPermission)) < 0)
     {
     perror("Could not reuse UDP port");
     exit(EXIT_FAILURE);
     }*/
    
    
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;/* Broadcast IP address */
    broadcastAddr.sin_port = htons(BCAST_PORT);         /* Broadcast port */
    cout << "Starting beacon" << endl;
    
    while(1)
    {
        sendto(sock, "Host", 4, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
        
        sleep(2);
        
    }
}


int main()
{
    //tcp settings
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *hello = "Hello from server";
    
    
    thread threadHolder (bCastThread);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    bind(server_fd, (struct sockaddr *)&address,sizeof(address));
    
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    Mat image(600,800,CV_8UC3);
    Mat edges(600,800,CV_8UC3);
    int datalen = 0,
    packetSize = 0,
    currPos =0,
    currPacket=0,
    radius1,
    radius2;
    Point2f center1, center2;
    Rect rect1, rect2;
    int angle;
    int dist;
    int key =  waitKey(100);
    
    Size imageSize;
    Filter brita;
    Metrics myMetrics(1280,69);
    const string filename = "HSV.txt";                      //need to get this to save
    const string filename2 = "Metrics.txt";  
    
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    OpenVideo myVideo(0);
    cout << "Capture is opened" << endl;
    
    if (!brita.readHSV(filename))            //if HSV file is not created
    {
        image = myVideo.getImage();
        imageSize = image.size();
        datalen = imageSize.width * imageSize.height * 3;
        packetSize = imageSize.width;
        
        
        while(currPos < datalen)            //send one image to config values on client side
        {
            if(currPos + packetSize > datalen)
                currPacket = datalen - currPos;
            else
                currPacket = packetSize;
            
            send(new_socket, image.data + currPos, currPacket, 0);
            currPos += currPacket;
        }
        
        read(new_socket, &brita.h_min, sizeof(int));    //get HSV values from trackbar on client side
        read(new_socket, &brita.h_max, sizeof(int));
        read(new_socket, &brita.s_min, sizeof(int));
        read(new_socket, &brita.s_max, sizeof(int));
        read(new_socket, &brita.v_min, sizeof(int));
        read(new_socket, &brita.v_max, sizeof(int));
        
        cout<<"H MIN  "<< brita.h_min <<endl;
        cout<<"H Max  "<< brita.h_max <<endl;
        cout<<"S MIN  "<< brita.s_min <<endl;
        cout<<"S MAX  "<< brita.s_max <<endl;
        cout<<"V MIN  "<< brita.v_min <<endl;
        cout<<"V MAX  "<< brita.v_max <<endl;
        
        brita.writeHSV(filename);
        
    }
cout<<"here"<<endl;
    bool readM = myMetrics.readMetrics(filename2);
	cout<<"sending readM"<<send(new_socket, &readM, sizeof(bool),0)<<endl;;
    if(!readM){
        image = myVideo.getImage();
        edges = brita.edgeDetect(&image);
        imageSize = edges.size();
        datalen = imageSize.width * imageSize.height * 3;
        packetSize = imageSize.width;
	cout<<"cannot read readm"<<endl;
        
        do{
           cout<<"in Metrics while loop"<<endl; 
            while(currPos < datalen)            //send one image to config values on client side
            {
                if(currPos + packetSize > datalen)
                    currPacket = datalen - currPos;
                else
                    currPacket = packetSize;
                
                send(new_socket, edges.data + currPos, currPacket, 0);
                currPos += currPacket;
            }
        
            read(new_socket, &key, sizeof(int));
            if(key == 'z')
                myMetrics.calibrateZero(&edges, dist);
            if (key == 99)
                myMetrics.configValues(&edges, dist);
            
        }while(key != 'q');
            
        myMetrics.writeMetrics(filename2);
        
    }
    
    while(waitKey(100) != 'q'){
	cout<<"senidng metrics"<<endl;        
        image = myVideo.getImage();
        edges = brita.edgeDetect(&image);
        
        myMetrics.drawBoundingBox(&edges);
        angle = myMetrics.angle();
        dist = myMetrics.distance();
        send(new_socket, &angle, sizeof(int),0);
        send(new_socket, &dist, sizeof(int),0); 
    }
    return 0;
    
}

