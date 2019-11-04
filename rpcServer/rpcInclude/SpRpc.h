#pragma once

#include <functional>
#include <unordered_map>
#include "Socket.h"

#define RPC_FUNC_HELP "?"
#define RPC_FUNC_LIST "*"

class Rpc
{
private:
	struct RpcCell
	{
		std::function<void(Serializer&)> mFunc;		// ִ����
		String mInfo;								// �������
		String mDef;								// ����������
		std::vector<size_t> mArgsType;				// �������͹�ϣ
		size_t mRtnType;							// ��������
	};

	// ����������ͳһ�ṹ
	template<typename T> struct TypeSign{ typedef T type; };
	template<> struct TypeSign < void > { typedef int type; };

	// ����Դ�void�ͷ�void����
	template<typename Rtn, typename Func, typename Tuple>
	typename std::enable_if<std::is_same<Rtn, void>::value, typename TypeSign<Rtn>::type>::type
		callHelper(Func f, Tuple tuple){
		TupleHelper->runFunc(tuple, f);
		return 0;
	}
	template<typename Rtn, typename Func, typename Tuple>
	typename std::enable_if<!std::is_same<Rtn, void>::value, typename TypeSign<Rtn>::type>::type
		callHelper(Func f, Tuple tuple){
		return TupleHelper->runFunc(tuple, f);
	}

public:
	enum RpcError
	{
		NoError,		// û�д���
		SerError,		// ���л�������
		FuncNotFound,	// δ�ҵ�����
		TypeMismatch,	// �������Ͳ�ƥ��
		TooFewParam,	// ��������
		OtherError		// ��������
	};

	template<typename Rtn>
	struct RpcResult
	{
		typename TypeSign<Rtn>::type res;
		size_t rtnType;
		RpcError errCode;
	};

private:
	// ģ��򵥻�
	template<typename Func>
	void proxyFunc(Func func, Serializer& ser)
	{
		callProxy(func, ser);
	}
	template<typename CFunc, typename Obj>
	void proxyFunc(CFunc func, Obj* obj, Serializer& ser)
	{
		callProxy(func, obj, ser);
	}

	// ���ֺ����Ĵ�����÷�ʽ
	template<typename Rtn, typename... Args>
	void callProxy(Rtn(*func)(Args...), Serializer& ser)
	{
		// ����tuple����
		using ArgsType = std::tuple<typename std::decay<Args>::type...>;

		// ����tuple��ֵ
		ArgsType args = ser.toTuple<ArgsType>();

		// ��������
		TypeSign<Rtn>::type rtn = callHelper<Rtn>(func, args);
		ser.clear();
		ser << rtn;
	}
	template<typename Rtn, typename... Args>
	void callProxy(std::function<Rtn(Args...)> func, Serializer& ser)
	{
		// ����tuple����
		using ArgsType = std::tuple<typename std::decay<Args>::type...>;

		// ����tuple��ֵ
		ArgsType args = ser.toTuple<ArgsType>();

		// ��������
		TypeSign<Rtn>::type rtn = callHelper<Rtn>(func, args);
		ser.clear();
		ser << rtn;
	}
	template<typename Rtn, typename Cls, typename Obj, typename... Args>
	void callProxy(Rtn(Cls::*func)(Args...), Obj* obj, Serializer& ser)
	{
		// ����tuple����
		using ArgsType = std::tuple<typename std::decay<Args>::type...>;

		// ����tuple��ֵ
		ArgsType args = ser.toTuple<ArgsType>();

		// ��������
		TypeSign<Rtn>::type rtn = callHelper<Rtn>([=](Args... args)->Rtn{ return (obj->*func)(args...); }, args);
		ser.clear();
		ser << rtn;
	}

	// ��ϣ��ȡ
	template<typename Type>
	size_t getTypeHash(){
		size_t hash = 0;
		if (std::is_same<std::decay<Type>::type, const char*>::value) hash = typeid(String).hash_code();
		else hash = typeid(Type).hash_code();
		return hash;
	}
	template<typename Type>
	void getArgsName(std::vector<size_t>& argsHashList)
	{
		argsHashList.emplace_back(getTypeHash<Type>());
	}
	// ��ϣ��ȡǰ�Բ����������жϴ���
	template<typename... Args>
	typename std::enable_if < std::tuple_size<std::tuple<Args...>>::value == 0 >::type
		getArgsCheck(std::vector<size_t>&){}
	template<typename... Args>
	typename std::enable_if < std::tuple_size<std::tuple<Args...>>::value != 0 >::type
		getArgsCheck(std::vector<size_t>& list){
		std::initializer_list < int > {(getArgsName<typename std::decay<Args>::type>(list), 0)...};
	}
	// �ֺ������ͻ�ȡ
	template<typename Rtn, typename... Args>
	void getFuncInfo(RpcCell& cell, Rtn(*func)(Args...))
	{
		cell.mRtnType = getTypeHash<Rtn>();
		getArgsCheck<Args...>(cell.mArgsType);
	}
	template<typename Rtn, typename... Args>
	void getFuncInfo(RpcCell& cell, std::function<Rtn(Args...)> func)
	{
		cell.mRtnType = getTypeHash<Rtn>();
		getArgsCheck<Args...>(cell.mArgsType);
	}
	template<typename Rtn, typename Cls, typename... Args>
	void getFuncInfo(RpcCell& cell, Rtn(Cls::*func)(Args...))
	{
		cell.mRtnType = getTypeHash<Rtn>();
		getArgsCheck<Args...>(cell.mArgsType);
	}
	// �������
	template<typename Arg>
	void getFuncArgsType(Serializer& ser, std::vector<size_t>& typeList, const Arg& arg)
	{
		typeList.emplace_back(getTypeHash<Arg>());
		ser << arg;
	}

	RpcCell* findFunc(const String& func){
		auto it = mBinded.find(func);
		return (it == mBinded.end() ? nullptr : &it->second);
	}

	// �ͻ��˷���
	template<typename Rtn = void>
	typename RpcResult<Rtn> cliSend(const String& addr, int port, Serializer& ser)
	{
		typename RpcResult<Rtn> rtn;
		memset(&rtn, 0, sizeof(typename RpcResult<Rtn>));
		ClientSocket cli(addr, port);
		cli.send(ser);
		ser.clear();
		cli.recv(ser);
		ser >> rtn.rtnType >> rtn.errCode >> rtn.res;
		ser.clear();
		return rtn;
	}

	// �ڲ����ܰ�
	void insideBind()
	{
		bind(RPC_FUNC_HELP, &Rpc::getInfo, this, String("��ȡRPC�󶨵ĺ�������"));
		bind(RPC_FUNC_LIST, &Rpc::getList, this, String("��ȡRPC�󶨵ĺ����б�"));
	}
	
public:
	Rpc(){
		insideBind();
	}

	// addrPos��ʽΪ: 127.0.0.1:47856
	void client(const String& addrPos)
	{
		auto sl = addrPos.split(":");
		mClientAddr = sl[0];
		mClientPort = sl[1].toInt();
	}
	void server(int port, int conn = SOMAXCONN){
		if (!mServer.isVaild())
		{
			mServer.listen(port, conn);
			mServer.accept([this](Socket sock){
				Serializer ser;
				sock.recv(ser);
				origCall(ser);
				sock.send(ser);
			});
		}
	}

	// ������
	template<typename Func>
	void bind(const String& funcStr, Func func, const String& info = "")
	{
		RpcCell cell;
		cell.mFunc = std::bind(&Rpc::proxyFunc<Func>, this, func, std::placeholders::_1);
		cell.mDef = typeid(std::decay<Func>::type).name();
		getFuncInfo(cell, func);
		cell.mInfo = info;
		mBinded[funcStr] = cell;
	}
	template<typename CFunc, typename Obj>
	void bind(const String& funcStr, CFunc func, Obj* obj, const String& info = "")
	{
		RpcCell cell;
		cell.mFunc = std::bind(&Rpc::proxyFunc<CFunc, Obj>, this, func, obj, std::placeholders::_1);
		cell.mDef = typeid(std::decay<CFunc>::type).name();
		getFuncInfo(cell, func);
		cell.mInfo = info;
		mBinded[funcStr] = cell;
	}

	// ԭʼ���� ������Ϣ�ͷ�����Ϣ�ᱻ������ser��
	void origCall(Serializer& ser)
	{
		if (ser.getCount() < 2)
		{
			ser.clear();
			ser << 0 << RpcError::SerError;
			return;
		}
		String funcName = "";
		size_t ArgsCnt = 0;
		ser >> funcName >> ArgsCnt;

		RpcCell* func = findFunc(funcName);
		if (func == nullptr)
		{
			ser.clear();
			ser << 0 << RpcError::FuncNotFound;
			return;
		}
		if (ArgsCnt < func->mArgsType.size())
		{
			ser.clear();
			ser << 0 << RpcError::TooFewParam;
			return;
		}

		size_t argHash = 0;
		for (auto it = func->mArgsType.begin(); it != func->mArgsType.end(); ++it)
		{
			if (ArgsCnt == 0)
			{
				ser.clear();
				ser << 0 << RpcError::TooFewParam;
				return;
			}
			ser >> argHash;
			--ArgsCnt;

			if (*it != argHash)
			{
				ser.clear();
				ser << 0 << RpcError::TypeMismatch;
				return;
			}
		}
		while (ArgsCnt--)
			ser.pop();

		func->mFunc(ser);
		ser << RpcError::NoError << func->mRtnType;
	}

	// ����ֱ�ӽ��к�������(״̬�ͷ���ֵ�ᱻ����)
	template<typename Rtn = void, typename ...Args>
	typename RpcResult<Rtn> localCall(const String& func, Args...arg)
	{
		typename RpcResult<Rtn> rtn;
		Serializer ser = toSer(func, std::forward<Args>(arg)...);
		origCall(ser);
		ser >> rtn.rtnType >> rtn.errCode >> rtn.res;
		ser.clear();
		return rtn;
	}
	template<typename Rtn = void>
	typename RpcResult<Rtn> localCall(Serializer& ser)
	{
		typename RpcResult<Rtn> rtn;
		origCall(ser);
		ser >> rtn.rtnType >> rtn.errCode >> rtn.res;
		ser.clear();
		return rtn;
	}

	// �������(״̬�ͷ���ֵ�ᱻ����)
	template<typename Rtn = void, typename ...Args>
	typename RpcResult<Rtn> call(const String& func, Args...arg)
	{
		Serializer ser = toSer(func, std::forward<Args>(arg)...);
		return cliSend<Rtn>(mClientAddr, mClientPort, ser);
	}
	template<typename Rtn = void>
	typename RpcResult<Rtn> call(Serializer& ser)
	{
		return cliSend(mClientAddr, mClientPort, ser);
	}

	// �����Ĳ������л�
	template<typename ...Args>
	Serializer toSer(const String& func, Args...arg)
	{
		Serializer ser;
		std::vector<size_t> argsList;
		std::initializer_list<int>{(getFuncArgsType(ser, argsList, arg), 0)...};
		for (auto it = argsList.rbegin(); it != argsList.rend(); ++it)
			ser << *it;
		ser << argsList.size() << func;
		return ser;
	}

	String getInfo(const String& func){
		auto f = findFunc(func);
		if (f == nullptr) return "�޴˺���";
		String info = String("Func: {0}\n Def: {1}\n")
						.arg(func).arg(f->mDef);
		String args;
		for (auto& it : f->mArgsType)
			args += String("%u, ", it);
		args.substr(0, args.rfind(",")).trim();
		return info + String("Args: {0}\n Rtn: %u\nInfo: {1}\n", f->mRtnType).arg(args).arg(f->mInfo);
	}
	std::vector<String> getList(){
		std::vector<String> list;
		for (auto& it : mBinded)
			list.emplace_back(it.first);
		return list;
	}
	
private:
	ServerSocket mServer;
	String mClientAddr;
	int mClientPort;

	std::unordered_map<String, RpcCell> mBinded;
};

