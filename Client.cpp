// Peaky_Blinders_v2.cpp : Defines the entry point for the console application.
//

//libraries
#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <IPHlpApi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <VersionHelpers.h>
#include <sstream>
#include <fstream>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

class Socks {
protected:
	WSADATA wsaData;
	SOCKET ConnectSocket;
	std::string port;
	std::string ipaddr;

public:
	Socks();
	~Socks();
	void resolve(addrinfo **info);
	int createclientsocket(addrinfo *info);
	int sendData(std::string msg);
	int recvData();
	int operations(std::string msg);
	void endclient();
	std::string encryption(std::string msg);
	int sendfile(std::string directory);
	std::string recvfile();
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

	//get ip address
	std::cout << "IP Address: ";
	std::cin >> ipaddr;

	//get port number
	std::cout << "Port: ";
	std::cin >> port;

	//get address information
	int result = getaddrinfo(ipaddr.c_str(), port.c_str(), &hints, info);
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

//send data
int Socks::sendData(std::string msg) {
	int result = 0;
	
	//send client information
	if (msg == "information") {
		//gets operating system version
		if (!IsWindows8OrGreater()) {
			msg = "Less than Windows 8";
		}
		else
			msg = "Is Windows 8 or greater";

		//get username
		std::string username = "";
		std::cout << "Enter user name: ";
		std::cin >> username;

		msg = "CLIENT_INFO\nOS: " +  msg + "\nUser Name: " + username + "\nIP Address: " + ipaddr + "\nPort: " + port;
	}

	//send message length
	int msg_len = msg.length();
	std::stringstream out;
	out << msg_len;
	std::string msg_as_len = out.str();
	result = send(ConnectSocket, msg_as_len.c_str(), msg_as_len.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 7;
	}

	//send message
	msg = encryption(msg);
	result = send(ConnectSocket, msg.c_str(), msg.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 7;
	}
	return 0;
}

//recieve data
int Socks::recvData() {
	int result = 0;
	char recvbuf[20];
	result = recv(ConnectSocket, recvbuf, 20, 0);

	if (result > 0) {
		std::string msg = recvbuf;

		//parse actual data from message
		for (int i = 0; i < 20; i++) {
			if (recvbuf[i] > 0)
				msg = msg + recvbuf[i];
		}
		int msg_len = atoi(msg.c_str());

		int number_of_bytes = 0;
		msg = "";
		while (number_of_bytes < msg_len) {
			result = recv(ConnectSocket, recvbuf, 20, 0);
			if (result > 0) {
				for (int i = 0; i < 20; i++) {
					msg = msg + recvbuf[i];
				}
				number_of_bytes += 20;
			}
			else
				break;
		}
		if (result > 0) {
			std::string encrypted_str = "";
			for (int i = 0; i < msg_len; i++) {
				encrypted_str += msg[i];
			}
			encrypted_str = encryption(encrypted_str);
			printf("%s", encrypted_str.c_str());
		}
	}
	else if (result == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());
		
	return 0;
}

//receive file from client
std::string Socks::recvfile() {
	int result = 0;
	char recvbuf[20];
	result = recv(ConnectSocket, recvbuf, 20, 0);

	if (result > 0) {
		std::string msg = recvbuf;

		//parse actual data from message
		for (int i = 0; i < 20; i++) {
			if (recvbuf[i] > 0)
				msg = msg + recvbuf[i];
		}
		int msg_len = atoi(msg.c_str());

		int number_of_bytes = 0;
		msg = "";
		while (number_of_bytes < msg_len) {
			result = recv(ConnectSocket, recvbuf, 20, 0);
			if (result > 0) {
				for (int i = 0; i < 20; i++) {
					msg = msg + recvbuf[i];
				}
				number_of_bytes += 20;
			}
			else
				break;
		}
		if (result > 0) {
			std::string encrypted_str = "";
			for (int i = 0; i < msg_len; i++) {
				encrypted_str += msg[i];
			}
			//decrypt
			encrypted_str = encryption(encrypted_str);
			return encrypted_str;
		}
	}

	return "0";
}

//manages networking operations
int Socks::operations(std::string msg) {
	recvData();
	while (msg != "exit") {
		//get message for server
		std::cout << "Enter command: ";
		std::cin >> msg;
		if (msg == "upload") {
			sendfile(msg);
		}
		else if (msg == "download") {
			sendData("download");
			recvData();
			std::string fname = "";
			std::cout << "Enter File Path: ";
			std::cin >> fname;
			sendData(fname);
			//receive file
			msg = recvfile();
			if (msg != "0") {
				//open file
				std::ofstream writeData;//Output stream to store incoming file data
				int found = 0;			//Points to the first char found
				found = fname.find_last_of("\\");//Parse the directory to get the filename 
				std::string newFile = fname.substr(found + 1, fname.length()).c_str(); // store the filename not the full path
				writeData.open(newFile);//open up a local file to write to
				//write file
				if (writeData.is_open()) {
					printf("%s", msg.c_str());
					writeData << msg;
					writeData.close();
					sendData("Ok\n");
				}
			}
			else
				sendData("error\n");
		}
		else
			sendData(msg);
		recvData();
	}
	return 0;
}

//close socket
void Socks::endclient() {
	closesocket(ConnectSocket);
	WSACleanup();
}

//encrypts data
std::string Socks::encryption(std::string msg) {
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	int msg_len = msg.length();
	//encryption
	for (int i = 0, x = 0; i < msg_len; i++, x++) {
		if (x > peaky_len) {
			x = 0;
		}
		msg[i] = msg[i] ^ peaky[x];
	}

	return msg;
}

//sends file
int Socks::sendfile(std::string directory) {
	////////////////////////////////////
	// Send "upload" to start file transfer
	///////////////////////////////////
	sendData("upload");

	////////////////////////////////////
	// RECIEVE RESPONSE "OKAY" FROM CLIENT
	////////////////////////////////////
	recvData();

	////////////////////////////////////
	// Start sending file data to client
	////////////////////////////////////
	std::cout << "Enter File Path: ";
	std::cin >> directory;

	std::fstream myfile(directory.c_str(), std::fstream::in);
	myfile.seekg(0, myfile.end);
	unsigned int filesize = myfile.tellg();
	char *fileBuf = new char[filesize];
	myfile.seekg(0, std::fstream::beg);
	myfile.read(fileBuf, filesize);
	myfile.close();


	sendData(directory);
	recvData();
	sendData(fileBuf);
	
	return 0;
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
	socket.sendData("information");
	//socket.recvData();
	socket.operations("");
	int temp = 0;
	std::cin >> temp;
	/*for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
	std::cout << "family: " << ptr->ai_family << std::endl;
	}*/
	socket.endclient();
	return 0;
}
