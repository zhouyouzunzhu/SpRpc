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

// ��ʼ�� ��server�˼��ش˶δ���ᱻִ��
// �����ڵ�Ϊ��ǰdll�汾�� server��ֻ����ظ߰汾
DLL_RPC_INIT("v1.0.0")
{
	// �ڲ�һЩ��ʼ��
	t.v = 5;

	// ��ͨ������ ����Ϊ�󶨺����İ�����Ϣ
	DLL_RPC_FUNC(mul, "�˷�����");

	// ���Ա������
	DLL_RPC_FUNC_CLASS(TestClass, get, &t, "TestClass�µ�get����");
}







