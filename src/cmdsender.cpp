// INCLUDES
 // Primitives
#include <iostream> // for std::cout
#include <cstring>
#include <stdint.h> // for e.g. uint8_t
#include <stdio.h>
#include <sstream> // for std::getline()

#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <winsock2.h>
//#include <io.h>
//#include <netioapi.h>

//#include <netinet/in.h>
//#include <unistd.h>

#pragma comment(lib, "Ws2_32.lib")


// DEFINES

// AF_INET -> IP
// AF = Address Family
// PF = Protocol Family
#define IP AF_INET


enum ReturnCodes
{
	SUCCESSFULLY_SENT,
	USER_ERROR,
	CONNECTION_PROBLEM
};



int main(int argc, char** argv)
{
	// strings we need
	/* char* pause = "pausegametime";
	char* resume = "unpausegametime"; */

	// Getting the argument
	std::string argument;
	//const char* argument = "noarg";

	if (argv[1])
	{
		argument = argv[1];
	}
	else
	{
		// Hardcoded override for debugging
		constexpr bool getParamenterFromCin = true;
		if (getParamenterFromCin)
		{
			std::string input;
			std::getline(std::cin, input);
			argument = input;
		}
		else
		{
			std::cerr << "No string to send was provided" << std::endl;
			return USER_ERROR; // Return because we can't do anything without what we need to write
		}
	}

	// creating socket
	// Protocol 0 selects it automatically.
	// Since IP only uses TCP, it selects TCP lol
	SOCKET clientSocket = socket(IP, SOCK_STREAM, 0);

	// clientSocket must be examined
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "Invalid Socket" << std::endl;
		return CONNECTION_PROBLEM;
	}

	// Creating the server address struct
	sockaddr_in serverAddress{};
	in_addr addr_buf{};
	char addr_str[INET_ADDRSTRLEN] = "127.0.0.1";
	serverAddress.sin_family = IP;
	serverAddress.sin_port = htons(16834);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(IP, addr_str, &addr_buf) != 1)
	{
		std::cerr << "Invalid address" << std::endl;
		return CONNECTION_PROBLEM;
	}

	serverAddress.sin_addr = addr_buf;

	// send connection request
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)))
	{
		// sending data
		send(clientSocket, argument.c_str(), argument.length(), 0);

		// closing socket
		closesocket(clientSocket);

		return SUCCESSFULLY_SENT;
	}

	std::cerr << "Couldn't connect" << std::endl;
	return CONNECTION_PROBLEM; // A connection hasn't been made
}