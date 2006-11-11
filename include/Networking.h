/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Common networking routines to help us
// Created 18/12/02
// Jason Boettcher


#ifndef __NETWORKING_H__
#define __NETWORKING_H__


#define		LX_SVRLIST		"/svr_list.php"
#define		LX_SVRREG		"/svr_register.php"
#define		LX_SVRDEREG		"/svr_deregister.php"


/* Shplorbs host, using freesql
#define		LX_SITE			"shplorb.ods.org"
#define		LX_SVRLIST		"/~jackal/svr_list.php"
#define		LX_SVRREG		"/~jackal/svr_register.php"
#define		LX_SVRDEREG		"/~jackal/svr_deregister.php"
*/


/* Old host
#define		LX_SITE			"host.deluxnetwork.com"
#define		LX_SVRLIST		"/~lierox/gamefiles/svr_list.php"
#define		LX_SVRREG		"/~lierox/gamefiles/svr_register.php"
#define		LX_SVRDEREG		"/~lierox/gamefiles/svr_deregister.php"
*/

#define		LX_SVTIMEOUT	35
#define		LX_CLTIMEOUT	30

#define     HTTP_TIMEOUT    90


// Functions
float	GetFixedRandomNum(int index);


// HTTP Request
bool	http_InitializeRequest(char *host, char *url);
void	http_ConvertUrl(char *dest, char *url);
int		http_ProcessRequest(char *szError);
bool	http_SendRequest(void);
void	http_RemoveHeader(void);
char	*http_GetContent(void);
void    http_CreateHostUrl(char *host, char *url);
void	http_Quit(void);

// socket type; the structure/type itself will be given around
#define NetworkSocket NLsocket
#define InvalidNetworkState NL_INVALID

// generell networking
bool	InitNetworkSystem();
bool	QuitNetworkSystem();
NetworkSocket	OpenReliableSocket(unsigned short port);
NetworkSocket	OpenUnreliableSocket(unsigned short port);
NetworkSocket	OpenBroadcastSocket(unsigned short port);
bool	ListenSocket(NetworkSocket sock);
bool	CloseSocket(NetworkSocket sock);


#endif  //  __NETWORKING_H__
