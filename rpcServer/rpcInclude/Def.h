#pragma once

#include "SpRpc.h"

#define DLL_GETVER	_getVersion
#define DLL_INIT	_init
#define DLL_RPC		_rpc


#define DLL_EXPORT __declspec(dllexport)
#define DLL_RPC_INIT(ver) \
static Rpc g_rpc;	\
extern "C" void DLL_EXPORT DLL_GETVER(String& str){ str = ver; } \
extern "C" void DLL_EXPORT DLL_RPC(Serializer& ser){ g_rpc.origCall(ser); }	\
extern "C" void DLL_EXPORT DLL_INIT()



#define DLL_RPC_FUNC(func, info) \
g_rpc.bind(#func, func, String(info))

#define DLL_RPC_FUNC_CLASS(cls, func, obj, info) \
g_rpc.bind(String("%s.%s", #cls, #func), &cls::func, obj, String(info))

