#include <iostream>
#include "ServerManager.h"
#include <fstream>

using namespace std;


void sendTest(Rpc& rpc, const String& filePath, const String& serPath)
{
	Serializer ser;
	std::ifstream file;
	file.open(filePath.data(), std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		size_t len = (size_t)file.tellg();
		file.seekg(file.beg);

		char* buffer = new char[len];
		memset(buffer, 0, len);
		file.read(buffer, len);
		ser.input(buffer, len);
		auto res = rpc.call("SendFile", ser, serPath);
		if (res.errCode == Rpc::NoError && res.res)
			cout << "发送成功" << endl;
		else
		{
			cout << String("发送失败 rtn:{0} err:{1}").arg(res.res).arg(res.errCode) << endl;
		}
		delete[] buffer;
		file.close();
	}
}

template<typename Type>
void putRtn(const Type& obj)
{
	cout << "rtn: \n\t" << obj << endl;
}
template<typename Type>
void putRtn(const std::vector<Type>& obj)
{
	puts("rtn: ");
	for (auto& it : obj)
		cout << "\t" << it << endl;
}
template<typename Rtn>
void putsRtn(Rpc::RpcResult<Rtn> rtn)
{
	cout << "err: " << rtn.errCode << endl;
	cout << "rtnType: " << rtn.rtnType << endl;
	putRtn(rtn.res);
}
void client()
{
	Rpc rpc;
	rpc.client("127.0.0.1:47856");
	puts("\n客户端启动...");

	puts("\n获取全局rpc");
	putsRtn(rpc.call<std::vector<String>>("*"));

	puts("\n复制testDll模块到服务器");
	sendTest(rpc, "testDll.dll", "modules/testDll.dll");

	puts("\n重新加载服务器的模块");
	putsRtn(rpc.call<int>("reload", "", "", false));

	puts("\n调用testDll模块内的mul函数");
	putsRtn(rpc.call<int>("testDll.mul", 5, 3));

	puts("\n获取testDll模块内的TestClass.get函数的帮助信息");
	putsRtn(rpc.call<String>("testDll.?", "TestClass.get"));

	puts("\n调用testDll模块内的TestClass.get函数");
	putsRtn(rpc.call<int>("testDll.TestClass.get", 37));
}

int main()
{
	client();

	system("pause");
	return 0;
}
