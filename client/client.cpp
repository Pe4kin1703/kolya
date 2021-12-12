#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <fstream>
//#include <sstream>
#include <vector>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

bool needStop = false;
int clientSockFD;

void programInfo() {
    std::cout
            << "Hi user!\n\t"
            << "You can type message using next format <ID0000 Example message>\n\t"
            << "If you want to end pritn 'exit'\n";

}

void connectionConfiguration(sockaddr_in &infoAboutServer) {
    infoAboutServer.sin_family = AF_INET;
    infoAboutServer.sin_port = htons(1030);
    infoAboutServer.sin_addr.s_addr = inet_addr("127.0.0.1");
}

bool connection(int &clientSockFD, const sockaddr_in &infoAboutServer) {
    clientSockFD = socket(AF_INET, SOCK_STREAM, 0);
    int Connect = connect(clientSockFD, (sockaddr *) &infoAboutServer, sizeof(infoAboutServer));
    if (Connect < 0) {
        std::cout << "Failed connection, return value: " << Connect << ", errno: " << strerror(errno) << "\n";
        return 0;
    } else {
        std::cout << "Client connected\n";
    }
    return 1;
}

void clientPart() {
    sockaddr_in infoAboutServer;

    connectionConfiguration(infoAboutServer);

    if (!connection(clientSockFD, infoAboutServer)) {
        needStop = true;
        return;
    }

    while (!needStop) {
        std::string request;
        getline (std::cin, request);
        request += "\n";
        send(clientSockFD, request.c_str(), request.size(), 0);

        if (request == "exit\n"){
            needStop = true;
            break;
        }

    }
}

void receivingThread(){
    while (!needStop){
        int sz = 512;
        char buf[sz]{0};
        int res = recv(clientSockFD, buf, sz, 0);
        std::cout<<buf<<"\n";
    }

}


int main() {
    std::cout << "Client started\n";

    programInfo();

    std::thread sendThread(clientPart);
    std::thread recieveTread(receivingThread);
    sendThread.join();
    recieveTread.join();



    std::cout << "Client end\n";

    return 0;
}
