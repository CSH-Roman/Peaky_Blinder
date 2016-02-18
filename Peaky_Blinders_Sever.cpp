// Peaky_Blinders_Sever.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "WS2_32.lib")

class Socks {
protected:
	WSADATA wsaData;
	SOCKET ListenSocket;
	SOCKET ClientSocket;

public:
	Socks();
	~Socks();
	void resolve(addrinfo **info);
	void createNbind(addrinfo **info);
	void startlistening();
	void endclient();
};

Socks::Socks() {
	int iResult = 0; //initialization result
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf_s("Socket Initialization: %d\n", iResult);
		//system("pause");
		WSACleanup();
		exit(1);
	}
}

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
		exit(2);
	}
}

//create a SOCKET for server and bind it
void Socks::createNbind(addrinfo **info) {
	ListenSocket = INVALID_SOCKET;
	addrinfo *result = *info;
	ListenSocket = socket(result->ai_family,
		result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(3);
	}

	int res = 0;
	res = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (res == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(4);
	}
}

//listen on server
void Socks::startlistening() {
	int result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(5);
	}
}

//close socket
void Socks::endclient() {
	closesocket(ClientSocket);
	WSACleanup();
}

int main()
{
	//variable declarations
	Socks serv_sock;
	addrinfo *info_ptr = NULL;

	//connection management
	serv_sock.resolve(&info_ptr);
	serv_sock.createNbind(&info_ptr);
	freeaddrinfo(info_ptr);

    return 0;
}

