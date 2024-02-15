#ifndef CLIENT_STATE
#define CLIENT_STATE

enum class ClientState
{
	READY_TO_SEND,
	READING_DONE,
	LOADING,
	SENDING_DONE,
};

#endif