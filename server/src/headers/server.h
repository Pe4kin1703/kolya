//
// Created by danylo on 10.12.2021.
//

#ifndef SIMPLE_SERVER_SERVER_H
#define SIMPLE_SERVER_SERVER_H

#include <iostream>
#include <unistd.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <chrono>
#include <ctime>
#include <map>
#include <functional>
#include <mutex>
#include <queue>
#include <wchar.h>
#include <wctype.h>
#include <stdio.h>
#include <cwchar>





// std::ofstream logFile;

 //std::map<std::string, std::function<bool(int x, int y, const std::string &text, const int clientServerSocFD,
                                      //   const int &clientID)>> commands;


void answerBack (const std::string& ID);

void serverConfiguration(int &sockfd, sockaddr_in &serv_addr, int &bindSocFD);
std::string requestParsingID(std::string request);
std::string requestParsingMsg(std::string request);
std::string getIP(sockaddr_in &client);
void conversationWithClientThread(const int& clientServerSocFD, const std::string& ID);
void AskID(int sockFd);
std::string GetID (int sockFd);
int parseSockFdFromMap(const std::string& ID);
std::queue<std::string>& parseMsgQueueFromMap(const std::string& ID);
void sendMsgsForNewcomerId (const std::string& ID);
int acceptServerThread();










#endif //SIMPLE_SERVER_SERVER_H
