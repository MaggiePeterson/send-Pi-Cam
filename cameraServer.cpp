#include "opencv2/core.hpp"
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

#define PORT 8888
#define BUFFER_SIZE 1*800*600
using namespace cv;
using namespace std;

int main()
{
    //tcp settings
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *hello = "Hello from server";
    
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
    
    Mat currImg;
    int datalen = 0;
    int packetSize = 0;
    int currPos =0;
    int currPacket=0;
    Size imageSize;
    int h_min,
        h_max,
        s_min,
        s_max,
        v_min,
        v_max;

    OpenVideo myVideo(0);
    cout << "Capture is opened" << endl;
    
    
    
    while(waitKey(10) != 'q')
    {
         currImg = myVideo.getImage();
         imageSize = currImg.size();
         datalen = imageSize.width * imageSize.height * 3;
         packetSize = imageSize.width;
       
       
        while(currPos < datalen)
        {
            if(currPos + packetSize > datalen)
                currPacket = datalen - currPos;
            else
                currPacket = packetSize;
            
            send(new_socket, currImg.data + currPos, currPacket, 0);
            currPos += currPacket;
        }
        
        read(new_socket, &h_min, sizeof(int));
        read(new_socket, &h_max, sizeof(int));
        read(new_socket, &s_min, sizeof(int));
        read(new_socket, &s_max, sizeof(int));
        read(new_socket, &v_min, sizeof(int));
        read(new_socket, &v_max, sizeof(int));
        
        cout<<"HMIN  "<< h_min <<endl;
        cout<<"HMax  "<< h_max <<endl;
        cout<<"SMIN  "<< s_min <<endl;
        cout<<"SMAX  "<< s_max <<endl;
        cout<<"VMIN  "<< v_min <<endl;
        cout<<"VMAX  "<< v_max <<endl;
        
    }
        
    return 0;
    
}




