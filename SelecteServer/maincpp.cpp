#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <conio.h>

#pragma comment(lib, "ws2_32")

#pragma pack(push, 1)
struct Data
{
	int X;
	int Y;

};
#pragma pack(pop)

int main()
{
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 0), &wsaData);

	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = PF_INET;
	inet_pton(AF_INET,
		"192.168.3.67",
		(PVOID) & (ServerSockAddr.sin_addr.s_addr)
	);
	ServerSockAddr.sin_port = htons(3000);

	bind(ListenSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	listen(ListenSocket, 0);

	fd_set ReadSockets;
	fd_set CopyReadSockets;
	FD_ZERO(&ReadSockets);
	FD_ZERO(&CopyReadSockets);

	FD_SET(ListenSocket, &ReadSockets);

	timeval TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 10;

	int PlayerX = 10;
	int PlayerY = 10;

	while (true)
	{
		CopyReadSockets = ReadSockets;

		int ChangedSocketCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);

		if (ChangedSocketCount == 0)
		{
			//다른작업
			//printf("processing\n");
			continue;
		}

		//서버한테 자료가 오면 소켓 네트워크 프로그래밍
		if (ChangedSocketCount < 0)
		{
			printf("error %d", GetLastError());
			exit(-1);
		}

		for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
		{
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					//accept
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClinetSocketLength = sizeof(ClientSockAddr);
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClinetSocketLength);
					FD_SET(ClientSocket, &ReadSockets);
					printf("connect client %s\n", inet_ntoa(ClientSockAddr.sin_addr));
				}
				else
				{
					char Buffer[1024] = { 0, };
					int RecvBytes = recv(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);

					if (RecvBytes <= 0)
					{
						SOCKET DisconnectedSocket = ReadSockets.fd_array[i];
						FD_CLR(DisconnectedSocket, &ReadSockets);
						closesocket(DisconnectedSocket);
					}
					else
					{

						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							if (ReadSockets.fd_array[j] != ListenSocket)
							{

								switch (Buffer[0])
								{
								case 'w':
									PlayerY--;
									break;
								case 's':
									PlayerY++;
									break;
								case 'a':
									PlayerX--;
									break;
								case 'd':
									PlayerX++;
									break;
								}

								system("cls");
								COORD Cur;
								Cur.X = PlayerX;
								Cur.Y = PlayerY;
								SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
								printf("*");

								Data Packet;

								Packet.X = htonl(PlayerX);
								Packet.Y = htonl(PlayerY);

								int SendBytes = send(ReadSockets.fd_array[j], (char*)&Packet, sizeof(Packet), 0);

							}
						}

					}
				}
			}
		}

	}



	WSACleanup();


	return 0;
}