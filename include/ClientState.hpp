#pragma once

enum class ClientState
{
	LOADING,
	READING_DONE,
	READY_TO_SEND,
	SENDING,
	KEEP_ALIVE,
	REMOVE_CONNECTION,
	ERROR,
};

enum class HTTPMethod
{
	GET,
	POST,
	DELETE,
	UNDEFINED,
};