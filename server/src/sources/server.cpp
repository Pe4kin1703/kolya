//#include <server.h>
#include "../headers/server.h"

std::mutex tableMutex;
std::mutex fileMutex;

int port = 1030;
int MAX_PORT = 10000;

int clientsCount = 0;
std::map<std::string, std::pair<int, std::queue<std::string>> > messagesForId;


void serverConfiguration(int &sockfd, sockaddr_in &serv_addr, int &bindSocFD) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cout << "ERROR opening socket \n";
        throw (0);
    }
    while (port < MAX_PORT) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_family = AF_INET;
        bindSocFD = bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr));
        if (bindSocFD < 0) {
            port++;
            // std::cout << "Port: " << port << " close\n";
            std::cout << bindSocFD << "\n";
        } else {
            std::cout << "Port: " << port << " bind\n";
            break;
        }
    }

    if (port > MAX_PORT) {
        throw (-1);
    }
}



std::string requestParsingID(std::string request) {
    std::string ID;
    for (int i = 0; i < 6; ++i) {
        ID += request[i];
    }
    return ID;
}

std::string requestParsingMsg(std::string request) {
    std::vector<std::string> _argv;
    return request.erase(0,6);
}

std::string getIP(sockaddr_in &client) {
    std::lock_guard<std::mutex> fileLK(tableMutex);
    uint32_t ip = client.sin_addr.s_addr;

    return std::to_string(reinterpret_cast<uint8_t *>(&ip)[0]) + "." +
           std::to_string(reinterpret_cast<uint8_t *>(&ip)[1]) + "." +
           std::to_string(reinterpret_cast<uint8_t *>(&ip)[2]) + "." +
           std::to_string(reinterpret_cast<uint8_t*>(&ip)[3]);
}

void conversationWithClientThread(const int& clientServerSocFD, const std::string& clientID) {
    int res;
    while (1) {

        std::string requestString;
        int sz = 512;
        char buf[sz]{0};
        res = recv(clientServerSocFD, buf, sz, 0);
        requestString += buf;

        std::cout<<"[conversationWithClientThread]: requestString: "<<requestString<<"\n";

        if (requestString == "exit\n"){
                char msg[] = "Bye";
                send (clientServerSocFD ,msg, strlen(msg), 0);
                break;
        }

        std::string destID = requestParsingID(requestString);
        std::string msg = requestParsingMsg(requestString);
        msg.insert(0, clientID+": ");
        std::cout<<"Message for ID:["<<destID<<"] "<<msg<<"\n";

        bool pushStatus = 0;
        while (pushStatus != 1){
            pushStatus = 1;
            tableMutex.lock();
            messagesForId[destID].second.push(msg);
            std::cout<<"[conversationWithClientThread]: msg for ID ["<<destID<<"] "<<messagesForId[destID].second.front()<<"\n";
            std::cout<<"[conversationWithClientThread]: sock ID "<<" "<<messagesForId[destID].first<<"\n";
            tableMutex.unlock();
        }



    }
    close(clientServerSocFD);

}

void AskID(int sockFd){
    char idRequest[] = "Please write your ID (for example <ID1111>)\n -> ";
    std::cout<< idRequest << "\n";
    send(sockFd,idRequest,strlen(idRequest), 0);
}

std::string GetID (int sockFd){
    char buf[32]{0};
    std::string ID;
    std::string req;
    bool status = 0;
    while (status != 1){
        int res = recv(sockFd, buf, 32,0);
        req +=buf;
/*    if (res!=6){
        AskID(sockFd);
    }*/
        std::cout<<"[GetID()]: res = ["<<res<<"]\n";
        if(res == 7){
            ID += buf;
            status = 1;
        }
        else{
            char msg[] = "Incorect input\n";
            send(sockFd, msg, strlen(msg), 0);
            AskID(sockFd);
        }
        std::cout<<"[GetID()]: ID = ["<<ID<<"]\n";
    }
    char acceptMsg[] = "Welcome to server. You can send message using only 512 symbols\n";
    send(sockFd, acceptMsg, strlen(acceptMsg), 0);

    return ID;
}

int parseSockFdFromMap(const std::string& ID){
    std::lock_guard<std::mutex> lc (tableMutex);
    int sockFd = messagesForId[ID].first;
    return sockFd;
}

void answerBack (const std::string& ID){
    //std::lock_guard<std::mutex> lc (tableMutex);
    if (messagesForId.find(ID) != messagesForId.end()){
        int sockFd = parseSockFdFromMap(ID);
        while (!messagesForId[ID].second.empty()){
            const char *msg = messagesForId[ID].second.front().c_str();
            messagesForId[ID].second.pop();
            send(sockFd, msg, strlen(msg), 0);
        }
    }
}

void sendMsgsForNewcomerId (const std::string& ID){
    //std::lock_guard<std::mutex> lc (tableMutex);
    if (messagesForId.find(ID) != messagesForId.end()){
        int sockFd = parseSockFdFromMap(ID);
        std::cout<<"[sendMsgsForNewcomerId]: Items in queue: "<<messagesForId[ID].second.size()<<"\n";
        while (!messagesForId[ID].second.empty()){
            const char *msg =messagesForId[ID].second.front().c_str();
            messagesForId[ID].second.pop();
            send(sockFd, msg, strlen(msg), 0);
        }
        std::cout<<"[sendMsgsForNewcomerId]: !msgQueue.empty(): "<<!messagesForId[ID].second.empty()<<"\n";
    }
}

void sendMsgsBack (const std::string& ID){
    while (1){
        answerBack(ID);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void initSockFdForNewcomer(int sockFd, const std::string& ID){
    //std::lock_guard<std::mutex> lc (tableMutex);
    messagesForId[ID].first = sockFd;
    std::cout<<"[initSockFdForNewcomer]: ID:["<<ID<<"] sockFd: "<< messagesForId[ID].first <<"\n";
}

void acceptMessage (sockaddr_in client_addres ,const std::string& ID){
    std::time_t time = std::time(0);
    std::tm *time_now = std::localtime(&time);
    std::mktime(time_now);

    std::cout << std::asctime(time_now) << "Connection accept for client with IP ID: [" << ID << "]\n\n";
}

int acceptServerThread() {
    int sockfd;
    sockaddr_in serv_addr;
    sockaddr_in client_addres;

    int bindSocFD;

    try {
        serverConfiguration(sockfd, serv_addr, bindSocFD);
    }
    catch (int) {
        return 0;
    }

    if (listen(sockfd, 10) < 0) {
        return 0;
    }

    socklen_t clilen = sizeof(client_addres);

    while (1) {
        int newSocketFD = accept(sockfd, (sockaddr *) &client_addres, &clilen);
        if (newSocketFD < 0) {
            std::cout<< "Accept error\n" << std::flush;
            // close(sockfd);
            return -2;
        }

        // Send messages to this ID if they exist
        AskID(newSocketFD);
        std::string clientID = GetID(newSocketFD);
        if (clientID.size() > 1){
            clientID.erase(clientID.end() - 1);
        }

        initSockFdForNewcomer(newSocketFD, clientID);

        acceptMessage(client_addres, clientID);

        sendMsgsForNewcomerId(clientID);

        std::thread serverCommunication(conversationWithClientThread, newSocketFD, clientID);
        std::thread serverSendBack(sendMsgsBack, clientID);
        serverCommunication.detach();
        serverSendBack.detach();

    }
}
