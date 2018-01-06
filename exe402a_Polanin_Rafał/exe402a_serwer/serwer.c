/*****************************************************************************
 *	APPLICATION:	exe402a_serwer
 *	MODULE:			main
 *	PURPOSE:
 *	AUTHOR(S):		Ryfles
 *
 *	6/8/2017 9:34:27 PM	Created.
 *****************************************************************************/
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")
#define SRVCNAME "50000"


#pragma pack(1)
typedef struct msg1_t {
	short bqty;
	char info;
}MSG1_T;
#pragma pack()
enum { BUFFER_LEN = 256, INFO_LEN = 160 };
//********************Prototype Function*****************************
extern int							msgeSend(SOCKET ioSocket, const size_t mlen, const char* commbuff);
extern int							msgeRecv(SOCKET ioSocket, const size_t mlen, const char* commbuff);
extern char*						makeLine(const char c, const int len);
extern void							wsaErrorMsge(const char* oprname, int wsaErrCode);
struct addrinfo*					getAddrsInfo(char* nodeName, char* srvcName);
static void							server(const char* srvcPort);
static SOCKET						bindToIp(struct addrinfo* iplist);
static DWORD						WINAPI CServc(void* param);
/*****************************************************************************
*	THE MAIN ENTRY POINT TO THE PROGRAM.
*
*	input:	argc	The number of arguments of the programme.
*			argv	The vector of arguments of the programme.
*	return:	The code of the reason the process was terminated for.
*			The value defaults to zero.
*****************************************************************************/
int main(int argc, char* argv[])
{
	int     iResult;
	WSADATA	wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult) {
		printf("(WSA)ERROR: Function 'WSAStartup' failed with error %d.\n",
			iResult);
		return	iResult;
	}
	else {
		//printf("%s \n", SRVCNAME);
		server(SRVCNAME);

		WSACleanup();
	}

	return 0;
}

void server(const char* srvcPort)
{
	struct addrinfo*		addri;
	struct addrinfo			adrhints;
	struct sockaddr_in*		sAddr;
	SOCKET					sListen;
	SOCKET					hSocket;
	int						iResult;

	ZeroMemory(&adrhints, sizeof(adrhints));

	adrhints.ai_family = AF_INET;
	adrhints.ai_socktype = SOCK_STREAM;
	adrhints.ai_protocol = IPPROTO_TCP;
	adrhints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, srvcPort, &adrhints, &addri);

	/*addri = getAddrsInfo(NULL, &srvcPort);*/
	sAddr = (struct sockaddr_in*)addri->ai_addr;
	printf("%s\n%u.%u.%u.%u\n", addri->ai_canonname, sAddr->sin_addr.S_un.S_un_b.s_b1, sAddr->sin_addr.S_un.S_un_b.s_b2, sAddr->sin_addr.S_un.S_un_b.s_b3, sAddr->sin_addr.S_un.S_un_b.s_b4);

	sListen = bindToIp(addri);
	freeaddrinfo(addri);

	if (sListen == INVALID_SOCKET) {
		printf("The socket is invalid.\n");
		return;
	}
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) {
		printf(WSAGetLastError());
		closesocket(sListen);
		return;
	}

	for (;;) {
		HANDLE hThread;
		hSocket = accept(sListen, NULL, NULL);

		if (hSocket == INVALID_SOCKET) {
			printf(WSAGetLastError());
			closesocket(hSocket);
			break;
		}
		hThread = CreateThread(NULL, 0, CServc, &hSocket, 0, NULL);
		if (!hThread) {
			printf("Thread creation has failed with error #%d", GetLastError());
		}
		CloseHandle(hThread);
	}

}

SOCKET bindToIp(struct addrinfo* iplist)
{
	int						iResult;
	SOCKET					hSocket = INVALID_SOCKET;
	struct sockaddr_in*		ipServ;

	for (; iplist; iplist = iplist->ai_next) {
		hSocket = socket(iplist->ai_family, iplist->ai_socktype, iplist->ai_protocol);
		if (hSocket == INVALID_SOCKET) continue;
		ipServ = (struct sockaddr_in*)iplist->ai_addr;
		printf("Server IP:   %u.%u.%u.%u\n", ipServ->sin_addr.S_un.S_un_b.s_b1, ipServ->sin_addr.S_un.S_un_b.s_b2, ipServ->sin_addr.S_un.S_un_b.s_b3, ipServ->sin_addr.S_un.S_un_b.s_b4);
		printf("Port:   %d\n", ipServ->sin_port);
		iResult = bind(hSocket, iplist->ai_addr, (int)iplist->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(hSocket);
			hSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	return hSocket;
}
DWORD WINAPI CServc(void* param)
{
	char buffer[BUFFER_LEN];
	int iResult;
	int mlen;
	SOCKET hSocket = *(SOCKET*)param;

	iResult = msgeRecv(hSocket, BUFFER_LEN, buffer);
	if (iResult > 0) {
		printf("message: ");
		for (int i = 0; i < iResult; i++)
			printf("%02X", buffer[i]);
		printf("\n");

		for (int i = 4; i < iResult; i++)
			printf("%c", buffer[i]);
		printf("\n");
	}
	closesocket(hSocket);
	return 0;
}
struct addrinfo* getAddrsInfo(char* nodeName, char* srvcName)
{
	struct addrinfo*		addri;
	struct addrinfo			addrHints;

	ZeroMemory(&addrHints, sizeof(addrHints));
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;
	addrHints.ai_protocol = IPPROTO_TCP;
	addrHints.ai_flags = AI_CANONNAME;

	getaddrinfo(nodeName, srvcName, &addrHints, &addri);

	return addri;
}

void wsaErrorMsge(const char* oprname, int wsaErrCode)
{
	printf("(WSA)ERROR: Function '%s' failed with error %d.\n", oprname,
		wsaErrCode);
}
char* makeLine(const char c, const int len)
{
	const int	size = len + 1;		/* The size of the container		  */
	return (char*)memset(memset(malloc(size), 0, size), c, len);
}
int msgeSend(SOCKET ioSocket, const size_t mlen, const char* commbuff)
{
	int iResult = send(ioSocket, commbuff, mlen, 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ioSocket);
		return -1;
	}
	return iResult;
}
int msgeRecv(SOCKET ioSocket, const size_t cblen, const char* commbuff)
{
	int iResult = recv(ioSocket, commbuff, cblen, 0);
	if (iResult == SOCKET_ERROR) {
		return -1;
	}
	//if (iResult == 0) printf("The connection has been lost");
	return iResult;
}