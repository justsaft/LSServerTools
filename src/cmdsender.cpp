// INCLUDES
 // Primitives
#include <iostream>		// for std::cout
#include <cstring>
#include <stdint.h>		// for e.g. uint8_t
#include <stdio.h>
#include <sstream>		// for std::getline()
// #include <stdlib.h>

#define WINDOWS

#ifdef WINDOWS
// #include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef SINGLE_INSTANCE
#include <tchar.h>
//#include <conio.h> // used for _getch()
//#include <synchapi.h> // For Single instance
#endif
#pragma comment(lib, "Ws2_32.lib")

//#elif defined(LINUX)
//#include <netinet/in.h>
//#include <unistd.h>
#endif

enum ReturnCodes
{
	SUCCESSFULLY_SENT,
	USER_ERROR,
	MY_SOCKET_ERROR,
	CONNECTION_ERROR,
#ifdef SINGLE_INSTANCE
	PIPE_ERROR,
	SENT_TO_PIPE
#endif
};

#ifdef SINGLE_INSTANCE
class PipeFunctionality {
public:
	bool createPipe(void)
	{
		bool fSuccess = false; // Return value

		while (true)
		{
			hPipe = CreateFile(
				pipeName,						// pipe name 
				GENERIC_READ | GENERIC_WRITE,	// read and write access 
				0,								// no sharing 
				NULL,							// default security attributes
				OPEN_EXISTING,					// opens existing pipe 
				0,								// default attributes 
				NULL);							// no template file 

			// Break if the pipe handle is valid. 
			if (hPipe != INVALID_HANDLE_VALUE)
			{
				/*std::cerr << "Invalid handle\n" << std::endl;
				exit(PIPE_ERROR);*/
				break; // We are done with creaing the pipe
			}

			// Exit if an error other than ERROR_PIPE_BUSY occurs. 
			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				std::cerr << "Could not open pipe\n" << std::endl;
				exit(PIPE_ERROR);
			}

			// All pipe instances are busy, so wait for 20 seconds.
			// I dunno if we can wait for 20s
			if (!WaitNamedPipe(pipeName, 20000))
			{
				std::cerr << "Could not open pipe\n" << std::endl;
				exit(PIPE_ERROR);
			}
		}

		//hPipe = CreateNamedPipeA(pipeName,
		//							PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
		//							PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
		//							PIPE_UNLIMITED_INSTANCES,
		//							BUFSIZE, BUFSIZE, 20000, NULL);


		// The pipe connected; change to message-read mode. 
		dwMode = PIPE_READMODE_MESSAGE;

		fSuccess = SetNamedPipeHandleState(
			hPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 

		if (!fSuccess)
		{
			std::cerr << "SetNamedPipeHandleState failed.\n" << std::endl;
			exit(PIPE_ERROR);
		}

		// _getch(); // Reads a single byte from the console without echoing it back
		// CloseHandle(hPipe);
		return fSuccess;
	}

	const LPTSTR& getName(void)
	{
		return pipeName;
	}

	bool writeToServer(const std::string& message)
	{
		bool fSuccess = 0; // Used for return value
		TCHAR chReadBuf[BUFSIZE];

		// Send a message to the pipe server and read the response. 
		fSuccess = TransactNamedPipe(
			hPipe,										// pipe handle 
			(void*)message.c_str(),						// message to server
			(lstrlen(message.c_str()) + 1) * sizeof(TCHAR),		// message length 
			chReadBuf,									// buffer to receive reply
			BUFSIZE * sizeof(TCHAR),					// size of read buffer
			&cbRead,									// bytes read
			NULL);										// not overlapped 

		if (!fSuccess && (GetLastError() != ERROR_MORE_DATA))
		{
			std::cerr << "TransactNamedPipe failed.\n" << std::endl;
			exit(PIPE_ERROR);
		}

		return fSuccess;
	}

	void readFromPipe(std::string& buffer)
	{
		bool fSuccess = 0;
		TCHAR chReadBuf[BUFSIZE];

		// Reading from pipe code
		while (true)
		{
			_tprintf(TEXT("%s\n"), chReadBuf);

			// Break if TransactNamedPipe or ReadFile is successful
			if (fSuccess)
				break;

			// Read from the pipe if there is more data in the message.
			fSuccess = ReadFile(
				hPipe,      // pipe handle 
				chReadBuf,  // buffer to receive reply 
				BUFSIZE * sizeof(TCHAR),  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			// Exit if an error other than ERROR_MORE_DATA occurs.
			if (!fSuccess && (GetLastError() != ERROR_MORE_DATA))
			{
				std::cout << "Some other error with the pipe occured" << std::endl;
				exit(PIPE_ERROR);
			}
			/*else
			{
				_tprintf(TEXT("%s\n"), chReadBuf);
			}*/
		}

		buffer.assign(static_cast<const char*>(chReadBuf)); // Problem: Not Null terminated
		// Transfer into our buffer that
		// we can give to the socket
	}

public:
	PipeFunctionality()
	{
		pipeName = TEXT("\\\\.\\pipe\\LSCmdSender");
		createPipe();
	}

	~PipeFunctionality()
	{
		CloseHandle(hPipe);
	}

private:
	static const int BUFSIZE = 512;
	LPTSTR pipeName;
	HANDLE hPipe;
	DWORD cbRead{}, dwMode{};
};
#endif

struct ClientSocket {
public:
	void sendViaSocket(const std::string& message) const
	{
		send(sock, message.c_str(), (int)message.length(), 0);
	}

	void closeSocket() const
	{
		closesocket(sock);
	}

	void makeSocket()
	{
		established = connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
		if (!established)
		{
			std::cerr << "Couldn't connect" << std::endl;
			exit(MY_SOCKET_ERROR);
		}
	}

	bool isEstablished() const
	{
		return established;
	}

public:
	ClientSocket()
	{
		sock = socket(IP, SOCK_STREAM, IP_TCP);

		// clientSocket must be examined
		if (sock == INVALID_SOCKET)
		{
			std::cout << "Invalid Socket" << std::endl;
			exit(MY_SOCKET_ERROR);
		}

		// Creating the server address data struct
		in_addr addr_buf{};
		if (inet_pton(IP, SOCKET_ADDR, &addr_buf) != 1)
		{
			std::cerr << "Invalid address" << std::endl;
			exit(CONNECTION_ERROR);
		}

		serverAddress.sin_family = IP;
		serverAddress.sin_port = htons(SOCKET_PORT);
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		serverAddress.sin_addr = addr_buf;

	}

	~ClientSocket()
	{
		closeSocket();
	}

private:
	// Socket information
	// AF_INET -> IP
	// AF = Address Family
	// PF = Protocol Family
	const static int IP = AF_INET; // Macro to make this more readable
	const static int IP_TCP = IPPROTO_TCP; // Macro to make this more readable
	const uint16_t SOCKET_PORT = 16834;
	const char SOCKET_ADDR[INET_ADDRSTRLEN] = "127.0.0.1";
	bool established = false;
	SOCKET sock;
	sockaddr_in serverAddress;
};


int main(int argc, char** argv)
{ // Entry Point

	// Getting the argument from argv[1]
	std::string argument;
	argument.reserve(32); // Reserving 32 bytes so we have some space

	if (argv[1])
	{
		argument = argv[1];
	}
	else
	{
		constexpr bool getArgumentFromCin = true; // For debugging
		if (getArgumentFromCin)
		{
			std::getline(std::cin, argument);
			if (argument.empty())
			{
				exit(USER_ERROR);
			}
		}
		else
		{
			std::cerr << "No string to send was provided" << std::endl;
			exit(USER_ERROR); // Return because we can't do anything without what we need to write
		}
	}

#ifdef SINGLE_INSTANCE
	//HANDLE hMutexHandle = CreateMutex(NULL, TRUE, L"org.HorizonSpeedrun.LSCmdSenderInstance");
	//if (GetLastError() == ERROR_ALREADY_EXISTS)
	//{ // Use the existing Socket
	//	return 0;
	//}

	/*SECURITY_ATTRIBUTES secAttr{};
	secAttr.nLength = sizeof(secAttr);
	secAttr.lpSecurityDescriptor = NULL;
	secAttr.bInheritHandle = TRUE;

	HANDLE eventHandle = CreateEventA((LPSECURITY_ATTRIBUTES)&secAttr, FALSE, FALSE, L"");*/

	PipeFunctionality pipe;

	// 1. detect that this is the first instance
	bool isServer = !GetNamedPipeInfo(pipe.getName(), NULL, NULL, NULL, NULL);
	if (!isServer)
	{ // If this is not the server instance, send the argument to it
		pipe.writeToServer(argument);
		exit(SENT_TO_PIPE); // We are done and the first instance will take over from here.
	}
	/*else
	{
		pipe.createPipe();
	}*/
#endif

	// Creating the socket
	ClientSocket sock;
	sock.makeSocket();
	sock.sendViaSocket(argument);

#ifdef SINGLE_INSTANCE
	// We keep the socket open in this case
	while (true)
	{ // Check the pipe for data
		argument.clear(); // Hopefully this keeps the reserve
		// we made in the beginning so we avoid reallocations

// 1. Write the data read to the buffer
		pipe.readFromPipe(argument);

		// 2. send the data to the socket
		sock.sendViaSocket(argument);
	}

	pipe.~PipeFunctionality();
#endif

	sock.~ClientSocket();
	return SUCCESSFULLY_SENT; // A connection hasn't been made

	//#ifdef defined(SINGLE_INSTANCE)
	//	ReleaseMutex(hMutexHandle);
	//	CloseHandle(hMutexHandle);
	//#endif
}