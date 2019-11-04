#include "Def.h"

int mul(int a, int b)
{
	return a * b;
}

struct TestClass
{
	int v;

	int get(int a)
	{
		return v + a;
	}
};

TestClass t;

// 初始化 被server端加载此段代码会被执行
// 括号内的为当前dll版本号 server端只会加载高版本
DLL_RPC_INIT("v1.0.0")
{
	// 内部一些初始化
	t.v = 5;

	// 普通函数绑定 后面为绑定函数的帮助信息
	DLL_RPC_FUNC(mul, "乘法测试");

	// 类成员函数绑定
	DLL_RPC_FUNC_CLASS(TestClass, get, &t, "TestClass下的get函数");
}







