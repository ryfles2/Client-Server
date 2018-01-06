/*****************************************************************************
 *	APPLICATION:	exe402a_Polanin_Rafał
 *	MODULE:			main
 *	PURPOSE:
 *	AUTHOR(S):		Ryfles
 *
 *	6/8/2017 9:28:39 PM	Created.
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


 //**********************Global Type ******************
#pragma pack(1)
typedef struct msg1_t {
	short bqty;
	char info;
}MSG1_T;
#pragma pack()
enum { BUFFER_LEN = 256, INFO_LEN = 160 };
//********************Prototype Function*****************************
extern int							msg1_to_buffer(const char* info, int blen, char* buffer);
extern int							msgeSend(SOCKET ioSocket, const size_t mlen, const char* commbuff);
extern int							msgeRecv(SOCKET ioSocket, const size_t mlen, const char* commbuff);
extern char*						makeLine(const char c, const int len);
extern void							wsaErrorMsge(const char* oprname, int wsaErrCode);
struct addrinfo*					getAddrsInfo(char* nodeName, char* srvcName);
void client(const char*				nameServer, const char* nameService);
void msg(struct addrinfo*			iplist);
SOCKET Connect(struct addrinfo*		iplist);
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
		client("Ryfles-Komputer", SRVCNAME);
		WSACleanup();
	}

	return 0;
}

void client(const char* nameServer, const char* nameService)
{
	struct addrinfo*		addri;
	struct sockaddr_in* sAddr;

	addri = getAddrsInfo(nameServer, nameService);
	sAddr = (struct sockaddr_in*)addri->ai_addr;
	printf("%s\n%u.%u.%u.%u\n", addri->ai_canonname, sAddr->sin_addr.S_un.S_un_b.s_b1, sAddr->sin_addr.S_un.S_un_b.s_b2, sAddr->sin_addr.S_un.S_un_b.s_b3, sAddr->sin_addr.S_un.S_un_b.s_b4);
	msg(addri);

	freeaddrinfo(addri);

}
void msg(struct addrinfo* iplist)
{
	char buffer[BUFFER_LEN];
	char info[160];
	int mlen;
	SOCKET hSocket;


	puts("Wczytaj wiadomosc do wyslania.\n");

	for (;;) {
		printf("klient> ");
		gets(info);
		if (!strlen(info)) break;
		mlen = msg1_to_buffer(info, BUFFER_LEN, buffer);
		printf("message: ");
		for (int i = 0; i < mlen; i++)
			printf("%02X", buffer[i]);
		printf("\n");
		hSocket = Connect(iplist);
		if (hSocket == INVALID_SOCKET) {
			wsaErrorMsge("hSocket", "Cannot connect to the server");
			return -1;
		}
		if (msgeSend(hSocket, mlen, buffer) < 0) return;
		mlen = msgeRecv(hSocket, BUFFER_LEN, buffer);
		if (mlen < 0) return;

	}
}

SOCKET Connect(struct addrinfo* iplist)
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

		iResult = connect(hSocket, iplist->ai_addr, (int)iplist->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(hSocket);
			hSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	return hSocket;
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
int msg1_to_buffer(const char* info, int blen, char* buffer)
{
#pragma pack(1)
	struct msg_t {
		short id;
		MSG1_T body;
	} *msge = (struct msg_t*)buffer;
#pragma pack()
	msge->id = 1;
	msge->body.bqty = (short)strlen(info);
	memcpy(&msge->body.info, info, msge->body.bqty);

	return sizeof(struct msg_t) - sizeof(msge->body.info) + msge->body.bqty;
}