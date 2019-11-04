#include "Socket.h"

#pragma comment(lib, "ws2_32")

int g_socketNum = 0;
bool g_isInit = false;
void Start()
{
	++g_socketNum;
	if (g_isInit == false && g_socketNum > 0)
	{
		WSADATA data;
		WSAStartup(MAKEWORD(2, 2), &data);
		g_isInit = true;
	}
}
void End()
{
	--g_socketNum;
	if (g_isInit == true && g_socketNum == 0)
	{
		WSACleanup();
		g_isInit = false;
	}
}

Socket::Socket() :mSocket(0), mSockIn({ 0 })
{
	Start();
	mRef = new int(1);
	mIsVaild = new bool(false);
}
Socket::Socket(const Socket& s)
{
	*s.mRef += 1;
	Start();
	mSocket = s.mSocket;
	mSockIn = s.mSockIn;
	mIsVaild = s.mIsVaild;
	mRef = s.mRef;
}
Socket& Socket::operator=(const Socket& s)
{
	*s.mRef += 1;
	Start();
	mSocket = s.mSocket;
	mSockIn = s.mSockIn;
	mIsVaild = s.mIsVaild;
	mRef = s.mRef;
	return *this;
}
Socket::~Socket()
{
	--(*mRef);
	if (*mRef == 0)
	{
		delete mRef;
		delete mIsVaild;
		closesocket(mSocket);
	}
	End();
}
void Socket::close()const
{
	closesocket(mSocket);
	*mIsVaild = false;
}

int Socket::recv(char* buffer, int len)const 
{ 
	if (*mIsVaild == false) return 0;
	return ::recv(mSocket, buffer, len, 0); 
}
int Socket::send(const char* buffer, int len)const 
{
	if (*mIsVaild == false) return 0;
	return ::send(mSocket, buffer, len, 0); 
}

void Socket::recv(String& str)const
{ 
	if (*mIsVaild == false) return;

	str = "";
	int res = 0;
	char buffer[MAXBYTE] = { 0 };
	do
	{
		res = ::recv(mSocket, buffer, MAXBYTE, 0);
		str += buffer;
		memset(buffer, 0, MAXBYTE);
	} while (res == MAXBYTE);
}
void Socket::send(const String& str)const
{
	if (*mIsVaild == false) return;
	send(str.data(), str.length());
}

void Socket::recv(Serializer& ser)const
{
	if (*mIsVaild == false) return;
	size_t totalSize = MAXBYTE, offset = 0;
	char* tBuffer = new char[totalSize];

	char buffer[MAXBYTE] = { 0 };
	int res = 0;
	do
	{
		res = recv(buffer, MAXBYTE);
		memcpy(tBuffer + offset, buffer, res);
		offset += res;
		if (offset >= totalSize)
		{
			totalSize *= 2;
			char* t = new char[totalSize];
			memcpy(t, tBuffer, offset);
			delete[] tBuffer;
			tBuffer = t;
		}
	}while (res == MAXBYTE);
	ser.decode(tBuffer, offset);
	delete[] tBuffer;
}
void Socket::send(Serializer& ser)const
{
	if (*mIsVaild == false) return;
	char* buffer = nullptr;
	size_t len = 0;
	ser.encode(buffer, len);
	send(buffer, len);
	delete[] buffer;
}



String Socket::getAddr()const
{
	if (*mIsVaild == false) return "";
	return inet_ntoa(mSockIn.sin_addr);
}

int Socket::getPort()const
{
	if (*mIsVaild == false) return 0;
	return ntohs(mSockIn.sin_port);
}

bool Socket::isVaild()const
{
	return *mIsVaild;
}

ServerSocket::ServerSocket()
	//:mThreadPool(SOMAXCONN)
{
}

ServerSocket::ServerSocket(int port, int conNum)
	//: mThreadPool(conNum)
{
	listen(port, conNum);
}

void ServerSocket::listen(int port, int conNum)
{
	if (mSocket != 0) return;
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	mSockIn.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	mSockIn.sin_family = AF_INET;
	mSockIn.sin_port = htons(port);

	::bind(mSocket, (sockaddr*)&mSockIn, sizeof(SOCKADDR));
	::listen(mSocket, conNum);

	*mIsVaild = true;
}

Socket ServerSocket::accept()
{
	Socket client;
	int size = sizeof(SOCKADDR_IN);
	client.mSocket = ::accept(mSocket, (SOCKADDR*)(&client.mSockIn), &size);
	*client.mIsVaild = true;
	return client;
}


void ServerSocket::accept(ProcFunc&& func)
{
	while (true)
	{
		Socket sock = accept();
		// 多线程不知道为何行会崩溃
		//mThreadPool.run(std::forward<ProcFunc>(func), sock);
		//std::thread th(std::forward<ProcFunc>(func), sock);
		//th.detach();
		func(sock);
	}
}

ClientSocket::ClientSocket()
{
}

ClientSocket::ClientSocket(const String& addr, int port)
{
	connect(addr, port);
}

bool ClientSocket::connect(const String& addr, int port)
{
	if (mSocket != 0) return false;
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	mSockIn.sin_addr.S_un.S_addr = inet_addr(addr.data());
	mSockIn.sin_family = AF_INET;
	mSockIn.sin_port = htons(port);

	if (::connect(mSocket, (SOCKADDR*)&mSockIn, sizeof(SOCKADDR)) == 0)
	{
		*mIsVaild = true;
		return true;
	}
	return false;
}