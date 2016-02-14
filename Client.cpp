// Peaky_Blinders_v2.cpp : Defines the entry point for the console application.
//

//libraries
#include "stdafx.h"
#include "pcap.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <IPHlpApi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

class Socks {
protected:
	WSADATA wsaData;
	SOCKET ConnectSocket;
	
public:
	Socks();
	~Socks();
	void resolve(addrinfo **info);
	int createclientsocket(addrinfo *info);
	int sendData();
	int recvData();
	void endclient();
};

//Constructor
//Initializes socket
Socks::Socks() {
	int iResult = 0; //initialization result
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf_s("Socket Initialization: %d\n", iResult);
		//system("pause");
		WSACleanup();
		exit(10);
	}

	//Create a socket
	/*mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mySocket == INVALID_SOCKET)
	{
		printf("Socket Initialization: Error creating socket");
		system("pause");
		WSACleanup();
		exit(11);
	}

	myBackup = mySocket;*/
}

//Deconstructor
Socks::~Socks() {
	WSACleanup();
}

//resolves address and port
void Socks::resolve(addrinfo **info) {
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int result = getaddrinfo("localhost", "80", &hints, info);
	if (result != 0) {
		printf("getaddrinfo failed with error: %d\n", result);
		WSACleanup();
		exit(1);
	}
}

//creates client socket
int Socks::createclientsocket(addrinfo *info) {
	ConnectSocket = socket(info->ai_family, info->ai_socktype,
		info->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		//printf("broken");
		return 1;
	}
	else
	{
		///////DOESN'T WORK WITHOUT SERVER//////////////
		//connect to server
		int result = connect(ConnectSocket, info->ai_addr,
			(int)info->ai_addrlen);
		if (result == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			printf("broken");
			return 2;
		}
		////////////////////////////////////////////////
	}
	return 0;
}

//recieve data
int Socks::sendData() {
	return 0;
}

//send data
int Socks::recvData() {
	return 0;
}

//close socket
void Socks::endclient() {
	closesocket(ConnectSocket);
	WSACleanup();
}

int main()
{
	Socks socket;
	struct addrinfo *info = NULL, *ptr = NULL;
	int result = 0;
	socket.resolve(&info);
	ptr = info;
	result = socket.createclientsocket(ptr);
	if (result > 0) {
		printf("broken");
		return 1;
	}
	freeaddrinfo(info);
	/*for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		std::cout << "family: " << ptr->ai_family << std::endl;
	}*/
	socket.endclient();
    return 0;
}

