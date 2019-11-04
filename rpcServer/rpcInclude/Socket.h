#pragma once

#include "Serializer.h"
//#include "ThreadPool.h"
#include <Windows.h>
#include <functional>

class Socket
{
public:
	Socket(const Socket&);
	Socket& operator=(const Socket&);
	virtual ~Socket();

	void close()const;

	int recv(char* buffer, int len)const;
	int send(const char* buffer, int len)const;

	void recv(String& str)const;
	void send(const String& str)const;

	void recv(Serializer& ser)const;
	void send(Serializer& ser)const;

	String getAddr()const;
	int getPort()const;

	bool isVaild()const;

protected:
	friend class ServerSocket;
	Socket();

	SOCKET mSocket;
	SOCKADDR_IN mSockIn;
	int* mRef;
	bool* mIsVaild;
};


class ServerSocket : public Socket
{
public:
	typedef std::function<void(Socket)> ProcFunc;

	ServerSocket();
	ServerSocket(int port, int conNum = SOMAXCONN);

	void listen(int port, int conNum = SOMAXCONN);
	Socket accept();

	void accept(ProcFunc&& func);

//private:
//	ThreadPool mThreadPool;
};


class ClientSocket : public Socket
{
public:
	ClientSocket();
	ClientSocket(const String& addr, int port);

	bool connect(const String& addr, int port);

};




