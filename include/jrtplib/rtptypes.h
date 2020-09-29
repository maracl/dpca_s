#include "rtpconfig.h"

#ifdef RTP_SOCKETTYPE_WINSOCK
	#include <winsock2.h>	
	#include <ws2tcpip.h>

#endif // RTP_SOCKETTYPE_WINSOCK

#include <stdint.h>
#include <sys/types.h>
