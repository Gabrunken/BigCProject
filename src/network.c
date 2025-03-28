#include <network.h>
#include <bigc.h>
#include <pthread.h>

uint8_t bigc_network_InitializeModule()
{
	WSADATA wsa; //Info sull'implementazione di winsock, tipo un log di inizializzazione

	//winsock va inizializzato
	//il primo argomento è la versione richiesta, in questo caso 2.2
	//il secondo è un puntatore alla struct WSADATA, è puramente un feedback a noi
	//se il valore di ritorno è diverso da zero, qualcosa è andato storto
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("Something went wrong when initializing winsock.\n");
		#endif
		return BIGC_FALSE;
	}

	#ifdef DEBUG
	BIGC_LOG_NOTE("Network module initialized");
	#endif

	return BIGC_TRUE;
}

struct Connection
{
	SOCKET serverSocket;
	struct sockaddr_in server;
	char ip[16];
	uint16_t port;
	bigc_NetworkProtocol protocol;
	void* (*userLogicCallback)(void*);
};

void* ConnectToServerThread(void* args)
{
	struct Connection* connectionDataPtr = (struct Connection*)args;

	int connection = connect(connectionDataPtr->serverSocket,
							 (struct sockaddr*)&(connectionDataPtr->server),
							 sizeof(connectionDataPtr->server));

	if(connection == SOCKET_ERROR)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to connect to server:");
		printf("\t%s, %d, %s\n",
				connectionDataPtr->ip,
				connectionDataPtr->port,
				connectionDataPtr->protocol == BIGC_TCP ? "TCP" : "UDP");
		#endif
		closesocket(connectionDataPtr->serverSocket);
	
		free(connectionDataPtr);

		pthread_exit(NULL);
		return NULL;
	}

	SOCKET* serverSocket = (SOCKET*)malloc(sizeof(SOCKET));
	*serverSocket = connectionDataPtr->serverSocket;

	pthread_t userLogicThread;
	pthread_create(&userLogicThread, NULL, connectionDataPtr->userLogicCallback, (void*)serverSocket);
	
	free(connectionDataPtr);

	pthread_exit(NULL);
	return NULL;
}

void bigc_network_TryConnectToServer(void* (*callback)(void*), char ip[20], uint16_t port, bigc_NetworkProtocol protocol)
{
	SOCKET serverSocket;
	if(protocol == BIGC_TCP)
		serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//controllo se il socket è stato creato correttamente
	if(serverSocket == INVALID_SOCKET)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("socket creation failed:");
		printf("\t%s, %d, %s\n", ip, port, protocol == BIGC_TCP ? "TCP" : "UDP");
		#endif
		return;
	}

	struct sockaddr_in server; //the thing we want to connect to (tcp port and ip, for the AF_INET family)
	server.sin_family = AF_INET; //must always be AF_INET

	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);

	struct Connection* connectionData = (struct Connection*)calloc(1, sizeof(struct Connection));
	connectionData->serverSocket = serverSocket;
	connectionData->server = server;
	strcpy(connectionData->ip, ip);
	connectionData->port = port;
	connectionData->protocol = protocol;
	connectionData->userLogicCallback = callback;

	pthread_t connectThread;
	pthread_create(&connectThread, NULL, ConnectToServerThread, (void*)connectionData);
}

uint8_t bigc_network_CheckConnectionRequests(SOCKET serverSocket)
{
	fd_set fdRead;
	struct timeval timeout = {0, 1}; //set to non-blocking
	
	FD_ZERO(&fdRead);
	FD_SET(serverSocket, &fdRead);
	select(serverSocket + 1, &fdRead, NULL, NULL, &timeout);
	
	if(FD_ISSET(serverSocket, &fdRead)) //if there is 1 or more clients waiting to be accepted
	{
		return BIGC_TRUE;
	}

	return BIGC_FALSE;
}

SOCKET bigc_network_AcceptFirstConnectionRequest(SOCKET serverSocket)
{
	struct sockaddr_in clientIP;
	int clientSize = sizeof(struct sockaddr_in);
	SOCKET newClient = accept(serverSocket, (struct sockaddr *)&clientIP, &clientSize);
	
	if(newClient == INVALID_SOCKET)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to accept a client's request to connect");
		#endif
		closesocket(newClient);

		return INVALID_SOCKET;
	}

	return newClient;
}

uint8_t bigc_network_StartLocalServer(void* (*callback)(void*), uint16_t port, bigc_NetworkProtocol protocol)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET; //must be always AF_INET
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	SOCKET *serverSocket = (SOCKET*)malloc(sizeof(SOCKET));
	if(protocol == BIGC_TCP)
		*serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		*serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//controllo se il socket è stato creato correttamente
	if(*serverSocket == INVALID_SOCKET)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("socket creation failed:");
		printf("\tlocalhost, %d, %s\n", port, protocol == BIGC_TCP ? "TCP" : "UDP");
		#endif
		return BIGC_FALSE;
	}

	int result = bind(*serverSocket, (struct sockaddr *)&server, sizeof(server));
	if(result == SOCKET_ERROR)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("server socket binding failed");
		printf("\tlocalhost, %d, %s\n", port, protocol == BIGC_TCP ? "TCP" : "UDP");
		#endif
		return BIGC_FALSE;
	}

	listen(*serverSocket, 100);

	pthread_t manageClientsThread;
	pthread_create(&manageClientsThread, NULL, callback, (void*)serverSocket);

	return BIGC_TRUE;
}

uint8_t bigc_network_CheckIncomingData(SOCKET senderSocket)
{
	fd_set fdRead;
	struct timeval timeout = {0, 1}; //set to non-blocking
	
	FD_ZERO(&fdRead);
	FD_SET(senderSocket, &fdRead);
	select(senderSocket + 1, &fdRead, NULL, NULL, &timeout);
	
	if(FD_ISSET(senderSocket, &fdRead)) //if there is 1 or more clients waiting to be accepted
	{
		return BIGC_TRUE;
	}

	return BIGC_FALSE;
}