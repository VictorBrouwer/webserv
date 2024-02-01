#include"HTTPserver.hpp"

int main()
{
	HTTPServer HTTPServer("0.0.0.0", 8080);
	HTTPServer.startListen();
	HTTPServer.startPolling();

	return 0;
}