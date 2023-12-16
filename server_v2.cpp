#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream> 
#include <cstdio>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
#define CLIENT_MAX 10
#define BUFF_SIZE 200

std::mutex clientCountMutex;

int globalClientSockets[CLIENT_MAX];
int globalClientCount = 0;

void sendMessageToClinet(char* msg, int messegeLength, int sendSocket) {
	clientCountMutex.lock();
	for (int i = 0; i < globalClientCount; ++i) {
		if (globalClientSockets[i] == sendSocket) continue;
		send(globalClientSockets[i], msg, messegeLength, 0);
	}
	clientCountMutex.unlock();
}

void clientConnection(void* arg) {
	int clientSocket = *((int*)arg);
	int receiveLength = 0;
	char message[BUFF_SIZE];

	while (true) {
		receiveLength = recv(clientSocket, message, strlen(message) - 1, 0);

		if (receiveLength == -1) {
			printf("client socket id : %d, disconnected \n", clientSocket);
			break;
		}
		printf("%.*s \n", receiveLength, message);
		sendMessageToClinet(message, receiveLength, clientSocket);
	}

	clientCountMutex.lock();

	for (int i = 0; i < globalClientCount; ++i) {
		if (clientSocket != globalClientSockets[i]) continue;

		for (int j = i + 1; j < globalClientCount; --j) {
			globalClientSockets[j - 1] = i;
		}
		break;
	}

	globalClientCount--;
	clientCountMutex.unlock();

	closesocket(clientSocket);
	printf("client count : %d\n", globalClientCount);
}

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to initialize Winsock\n");
		return 0;
	}

	int serverSocket;
	int clientSocket;
	int receiveLength;

	//IPv4 , TCP 통신, TCP 프로토콜 사용
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (serverSocket == -1) {
		printf("%d\n", serverSocket);
		printf("server socket error \n");
		return 0;
	}
	else {
		printf("socket success \n");
	}

	// Set up the server address structure
	struct sockaddr_in serverAddress;
	//IPv4
	serverAddress.sin_family = AF_INET;
	//host order를 big-enddian으로 변환
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(8080);

	struct sockaddr_in clientAddress;
	int clientAddressSize;

	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		printf("bind error \n");
	}
	else {
		printf("bind seccess \n");
	}

	//몇명 대기상태 listen할지 설정
	if (listen(serverSocket, 5) == -1) {
		printf("listen error \n");
	}
	else {
		printf("listen success \n");
	}

	char buff[200];
	while (true) {
		clientAddressSize = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
		/*{
			std::lock_guard<std::mutex> lock(clientCountMutex);
			globalClientSockets[globalClientCount++] = clientSocket;
		}*/
		clientCountMutex.lock();
		globalClientSockets[globalClientCount++] = clientSocket;
		printf("client count : %d\n", globalClientCount);

		clientCountMutex.unlock();

		std::thread clientThread(clientConnection, &clientSocket);
		clientThread.detach();
	}

	WSACleanup();
}
