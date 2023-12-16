#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream> // Changed from <istream> to <iostream>
#include <Windows.h>
#include<stdio.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

#define BUFF_SIZE 100
#define NAME_SIZE 20

void receiveMessage(void* arg) {
	int socket = *((int*)arg);
	int receiveLength = 0;
	char message[BUFF_SIZE];

	while (true) {
		receiveLength = recv(socket, message, strlen(message) - 1, 0);

		if (receiveLength == -1) {
			printf("socket closed");
			break;
		}
		printf("%.*s \n", receiveLength, message);
	}
	closesocket(socket);
}


int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed to initialize Winsock\n");
		return 0;
	}
	int sock;
	struct sockaddr_in serverAddress;
	//pthread_t snd_thread, rcv_thread;
	void* thread_result;

	//WSADATA wsaData;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("socket() error \n");
	}

	std::thread receiveThread(receiveMessage, &sock);

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	//serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

	serverAddress.sin_port = htons(8080);

	if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		printf("connect() error \n");
	}
	else {
		printf("connect success \n");
	}

	char id[100];
	char msg[100];
	char sendMessage[200];

	printf("input id : ");
	fgets(id, sizeof(id), stdin);
	id[strcspn(id, "\n")] = '\0';

	while (1) {
		fgets(msg, sizeof(msg), stdin);
		msg[strcspn(msg, "\n")] = '\0';

		sprintf_s(sendMessage, "[%s] : %s", id, msg);
		//sendMessage[strcspn(sendMessage, "\n") + 1] = '\0';

		printf("%s", sendMessage);
		send(sock, sendMessage, strlen(sendMessage), 0);
		printf("\n");
	}
	closesocket(sock);
	WSACleanup();

}