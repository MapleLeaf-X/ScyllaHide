#ifndef IDASERVER_H
#define IDASERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

BOOL startWinsock();
void startListen();
void handleClient( SOCKET ClientSocket );

#endif // IDASERVER_H

