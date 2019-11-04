#pragma once

#include "Def.h"
#include "Socket.h"
#include <unordered_map>

#include <iostream>
#include <fstream>
using namespace std;

class ServerModule
{
	friend class ServerManager;
private:
	HMODULE mModule;
	bool mIsLoaded;
	bool mIsInited;

	typedef void(*DllGetVersion)(String&);
	typedef void(*DllInit)();
	typedef void(*DllRpc)(Serializer&);

	DllRpc mDllRpc;

	ServerModule(const ServerModule&);
	ServerModule& operator=(const ServerModule& m) = default;

public:
	ServerModule() :mModule(nullptr), mIsLoaded(false), mIsInited(false), mDllRpc(nullptr){}

	bool load(const String& path){
		if (mIsLoaded) return false;

		mModule = LoadLibrary(path.data());
		if (mModule != nullptr)
			mIsLoaded = true;
		return mIsLoaded;
	}
	void unload(){
		if (!mIsLoaded) return;
		FreeLibrary(mModule);
		mModule = nullptr;
	}

	String getVersion(){
		if (!mIsLoaded) return "";
		DllGetVersion dllGetVersion = (DllGetVersion)(GetProcAddress(mModule, "_getVersion"));
		if (dllGetVersion == nullptr) return "";
		String str;
		dllGetVersion(str);
		return str;
	}
	bool initModule(){
		if (!mIsLoaded || mIsInited) return false;
		DllInit dllInit = (DllInit)(GetProcAddress(mModule, "_init"));
		if (dllInit == nullptr) return false;
		dllInit();
		mDllRpc = (DllRpc)(GetProcAddress(mModule, "_rpc"));
		mIsInited = true;
		return true;
	}

	bool rpc(Serializer& ser)
	{
		if (mDllRpc == nullptr || ser.getCount() == 0) return false;
		mDllRpc(ser);
		return true;
	}
};




class ServerManager
{
public:
	ServerManager(){
		insideBind();
		scaning();
	}
	String scaning(String path = "", String exp = "", bool remove = false){
		if (path.isEmpty())
		{
			char buffer[MAXBYTE] = { 0 };
			GetCurrentDirectory(MAXBYTE, buffer);
			path = String(buffer) + "\\modules\\";
		}
		if (exp.isEmpty())
			exp = "dll";
		cout << String("开始扫描模块\n扫描路径:{0}\n扫描扩展名:{1}").arg(path).arg(exp) << endl;
		mFailNum = 0;
		mNewNum = 0;
		mUpdateNum = 0;
		mSkimNum = 0;
		mRemoveNum = 0;

		String name;
		WIN32_FIND_DATA data = { 0 };
		HANDLE sign = FindFirstFile((path + "*." + exp).data(), &data);
		std::vector<String> tList;
		while (sign)
		{
			name = data.cFileName;
			if (!name.isEmpty())
			{
				name = name.split(".")[0];
				tList.push_back(name);
				detect(name, path + name + "." + exp);
			}
			if (!FindNextFile(sign, &data))
				break;
		}

		if (remove)
		{
			for (auto it = mModules.begin(); it != mModules.end();)
			{
				auto iter = std::find(tList.begin(), tList.end(), it->first);
				if (iter == tList.end())
				{
					++mRemoveNum;
					it = mModules.erase(it);
				}
				else
					++it;
			}
		}
		String info = String("扫描完毕\n失败:%d\t新增:%d\t更新:%d\t移除:%d\t略过:%d\n", mFailNum, mNewNum, mUpdateNum, mRemoveNum, mSkimNum);
		cout << info << endl;
		return info;
	}
	void server(int port, int conn = SOMAXCONN){
		if (!mServer.isVaild())
		{
			mServer.listen(port, conn);
			cout << "rpc服务器启动...." << endl;
			mServer.accept([this](Socket sock){
				String info = String("{0}:{1} ").arg(sock.getAddr()).arg(sock.getPort());
				cout << (info + "链接成功") << endl;
				Serializer ser;
				sock.recv(ser);
				cout << info + "请求:" + ser.back() << endl;
				rpc(ser);
				sock.send(ser);
				cout << info + "状态:" << ser.at<Rpc::RpcError>(1) << endl;
				cout << info + "断开连接\n" << endl << endl;
			});
		}
	}

private:
	std::vector<String> getModuleNames(){
		std::vector<String> list;
		String str = "";
		for (auto& it : mModules)
			list.emplace_back(it.first + "\t" + it.second.getVersion());
		return list;
	}

private:

	void insideBind(){
		mGlobalRpc.bind("reload", &ServerManager::scaning, this, String("使服务器重新加载模块"));
		mGlobalRpc.bind("mods", &ServerManager::getModuleNames, this, String("获取服务器当前已经加载的模块名称以及版本信息"));
		mGlobalRpc.bind("SendFile", &ServerManager::sendFile, this, String("通过rpc和ser传递小型文件到服务器"));
	}

	void rpc(Serializer& ser){
		auto sign = ser.back().split(".");
		if (sign.size() > 1)
		{
			String modName = sign[0];
			sign.erase(sign.begin());
			ser.pop();
			String t = String::Join(sign, ".");
			ser << t;
			auto it = mModules.find(modName);
			if (it != mModules.end())
				it->second.rpc(ser);
		}
		else
		{
			ser.pop();
			String t = String::Join(sign, ".");
			ser << t;
			mGlobalRpc.origCall(ser);
		}
	}

	void detect(const String& name, const String& path){
		ServerModule tModule;
		if (tModule.load(path) == false)
		{
			++mFailNum;
			cout << String("加载失败:{0}").arg(path) << endl;
			return;
		}

		String tarVer = tModule.getVersion();
		auto it = mModules.find(name);
		if (it == mModules.end())
		{
			if (tModule.initModule())
			{
				++mNewNum;
				cout << String("装载模块:{0}\t版本:{1}").arg(name).arg(tarVer) << endl;
				mModules[name] = tModule;
				return;
			}
		}

		String srcVer = it->second.getVersion();
		if (srcVer < tarVer)
		{
			if (tModule.initModule())
			{
				++mUpdateNum;
				cout << String("更新模块:{0}\t版本:{1} -> {2}").arg(name).arg(srcVer).arg(tarVer) << endl;
				it->second.unload();
				mModules.erase(it);
				mModules[name] = tModule;
				return;
			}
		}
		++mSkimNum;
	}

	int sendFile(Serializer ser, String path)
	{
		int sign = 0;
		char* buf = nullptr;
		size_t len = 0;
		ser.outputEx(buf, len);
		std::ofstream file;
		file.open(path.data(), std::ios::out | std::ios::binary);
		if (file.is_open())
		{
			file.write(buf, len);
			file.close();
			sign = 1;
		}
		delete[] buf;
		return sign;
	}

private:
	std::unordered_map<String, ServerModule> mModules;
	Rpc mGlobalRpc;
	ServerSocket mServer;
	int mFailNum;
	int mNewNum;
	int mUpdateNum;
	int mSkimNum;
	int mRemoveNum;
};
