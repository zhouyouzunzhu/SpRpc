#pragma once
#include "mstr.h"
#include "TupleHelper.h"

#define SERIALIZER_INIT_SIZE 32
#define SERIALIZER_HEAD_SIZE 32

#define SerInputFunc serInput
#define SerOutputFunc serOutput

/*
	ע�⣺
		�����д���String��ָ������ �����б���Ҫ�д���������,��ʽ����:
			void SerInputFunc(Serializer& ser) const; �˴������const
			void SerOutputFunc(Serializer& ser);
		�����������˳���෴,��Ϊ�ڲ���ջ�ṹ
			ser << a << b << c;
			ser >> c >> b >> a;
		���ú��������汾ֱ�ӷ���˳����ͬ,�ڲ�˳����������
			ser.inputs(a, b, c);
			ser.outputs(a, b, c);
		ser.at������Χ���,����ǰҪȷ���������ھ����м���Ԫ��
			at������<����>��Ĭ��ΪString
*/
class Serializer
{
private:
	// ת����tuple�ĸ�����
	class ToTuple
	{
		Serializer* mSer;
		size_t mIndex;
	public:
		ToTuple(Serializer* ser) : mSer(ser), mIndex(0){}
		template<typename Arg>
		void operator()(Arg& arg)
		{
			arg = mSer->at<Arg>(mIndex++);
		}
	};

	// ����Ƿ��д������ĸ����ṹ��(ֻ�������ж����Բ���Ҫ����)
	template<typename Type>
	struct hasSerInput
	{
	private:
		template<typename TType>
		static auto check(int) 
			-> decltype(std::declval<TType>().SerInputFunc(std::declval<Serializer&>()), std::true_type());
		template<typename TType>
		static std::false_type check(...);

	public:
		enum { value = std::is_same<decltype(check<Type>(0)), std::true_type>::value };
	};
	template<typename Type>
	struct hasSerOutput
	{
	private:
		template<typename TType>
		static auto check(int)
			-> decltype(std::declval<TType>().SerOutputFunc(std::declval<Serializer&>()), std::true_type());
		template<typename TType>
		static std::false_type check(...);

	public:
		enum { value = std::is_same<decltype(check<Type>(0)), std::true_type>::value };
	};

	// ������С���
	void checkExpend(size_t len){
		while (mCurrent + len >= mMaxSize)
		{
			mMaxSize *= 2;
			char* tBuf = new char[mMaxSize];
			memcpy(tBuf, mBuffer, mCurrent);
			delete[] mBuffer;
			mBuffer = tBuf;
		}
	}
	void checkMinify(){
		if (mMaxSize > SERIALIZER_INIT_SIZE && mCurrent * 3 < mMaxSize)
		{
			mMaxSize /= 2;
			char* tBuf = new char[mMaxSize];
			memcpy(tBuf, mBuffer, mCurrent);
			delete[] mBuffer;
			mBuffer = tBuf;
		}
	}

	std::vector<size_t> mIndexList;
	size_t mCurrent;
	size_t mMaxSize;
	char* mBuffer;

public:
	enum Sign{
		Omit	// �������
	};

public:
	Serializer() :mCurrent(0), mMaxSize(SERIALIZER_INIT_SIZE), mBuffer(new char[SERIALIZER_INIT_SIZE]){}
	Serializer(const Serializer& ser){
		mIndexList = ser.mIndexList;
		mCurrent = ser.mCurrent;
		mMaxSize = ser.mMaxSize;
		mBuffer = new char[mMaxSize];
		memcpy(mBuffer, ser.mBuffer, mMaxSize);
	}
	Serializer& operator=(const Serializer& ser){
		mIndexList = ser.mIndexList;
		mCurrent = ser.mCurrent;
		mMaxSize = ser.mMaxSize;
		delete[] mBuffer;
		mBuffer = new char[mMaxSize];
		memcpy(mBuffer, ser.mBuffer, mMaxSize);
		return *this;
	}
	Serializer(const char* buf, size_t size){ mBuffer = nullptr; decode(buf, size); }
	~Serializer(){ delete[] mBuffer; }

	// ����Ϊ���ݿ� encode��buf��Ҫdelete[]
	void encode(char*& buf, size_t& size)
	{
		size = SERIALIZER_HEAD_SIZE + mIndexList.size() * sizeof(size_t)+mMaxSize;
		buf = new char[size];
		char* offset = buf;

		char headBuf[SERIALIZER_HEAD_SIZE] = { 0 };
		const char* headStr = "cnt:%u cur:%u max:%u ";
		sprintf_s(headBuf, headStr, mIndexList.size(), mCurrent, mMaxSize);
		memcpy(offset, headBuf, SERIALIZER_HEAD_SIZE);
		offset += SERIALIZER_HEAD_SIZE;

		if (!mIndexList.empty())
		{
			size_t listLen = mIndexList.size() * sizeof(size_t);
			memcpy(offset, (char*)(&mIndexList[0]), listLen);
			offset += listLen;
		}

		memcpy(offset, mBuffer, mMaxSize);
	}
	void decode(const char* buf, size_t size)
	{
		size_t listCnt = 0;
		const char* headStr = "cnt:%u cur:%u max:%u ";
		sscanf_s(buf, headStr, &listCnt, &mCurrent, &mMaxSize);
		buf += SERIALIZER_HEAD_SIZE;

		if (listCnt != 0)
		{
			mIndexList.resize(listCnt);
			memcpy((char*)(&mIndexList[0]), buf, listCnt * sizeof(size_t));
			buf += listCnt * sizeof(size_t);
		}
		if (mBuffer != nullptr)
			delete[] mBuffer;
		mBuffer = new char[mMaxSize];
		memcpy(mBuffer, buf, mMaxSize);
	}

	size_t getCount()const{ return mIndexList.size(); }
	size_t getSize()const{ return mMaxSize; }

	template<typename Type = String>
	Type back(){ return this->at<Type>(mIndexList.size() - 1); }
	void pop(){
		if (mIndexList.empty()) return;
		size_t lastPos = mIndexList.back();
		size_t len = mCurrent - lastPos;
		mIndexList.pop_back();
		mCurrent = lastPos;
		checkMinify();
	}
	void clear(){
		mIndexList.clear();
		mCurrent = 0;
		mMaxSize = SERIALIZER_INIT_SIZE;
		delete[]mBuffer;
		mBuffer = new char[SERIALIZER_INIT_SIZE];
	}

	// �˺���������Χ���ͼ��
	template<typename Type>
	Type at(size_t index){
		size_t pos = mIndexList[index];
		Type& obj = (*reinterpret_cast<Type*>(mBuffer + pos));
		return obj;
	}
	template<> String at(size_t index){
		size_t pos = mIndexList[index];
		size_t last = mCurrent;
		if (index + 1 < mIndexList.size())
			last = mIndexList[index + 1];
		size_t len = last - pos;
		char* buf = new char[len + 1];
		memcpy(buf, mBuffer + pos, len);
		buf[len] = '\0';
		String str = buf;
		delete[] buf;
		return std::forward<String>(str);
	}
	template<> Serializer at(size_t index){
		Serializer ser;
		if (mIndexList.empty()) return ser;
		size_t pos = mIndexList[index];
		size_t last = mCurrent;
		if (index + 1 < mIndexList.size())
			last = mIndexList[index + 1];
		size_t len = last - pos;
		char* bgn = mBuffer + pos;
		ser.decode(bgn, len);
		return ser;
	}
	String at(size_t index){
		size_t pos = mIndexList[index];
		size_t last = mCurrent;
		if (index + 1 < mIndexList.size())
			last = mIndexList[index + 1];
		size_t len = last - pos;
		char* buf = new char[len + 1];
		memcpy(buf, mBuffer + pos, len);
		buf[len] = '\0';
		String str = buf;
		delete[] buf;
		return std::forward<String>(str);
	}

	// �ֱ�����������޴���������
	template<typename Type>
	Serializer& input(const Type& obj, typename std::enable_if<hasSerInput<Type>::value, Type>::type* = 0){
		obj.SerInputFunc(*this);
		return *this;
	}
	template<typename Type>
	Serializer& input(const Type& obj, typename std::enable_if<!hasSerInput<Type>::value, Type>::type* = 0){
		size_t size = sizeof(Type);
		char* buf = (char*)&obj;
		checkExpend(size);
		memcpy(mBuffer + mCurrent, buf, size);
		mIndexList.push_back(mCurrent);
		mCurrent += size;
		return *this;
	}
	template<typename Type>
	Serializer& output(Type& obj, typename std::enable_if<hasSerOutput<Type>::value, Type>::type* = 0){
		obj.SerOutputFunc(*this);
		return *this;
	}
	template<typename Type>
	Serializer& output(Type& obj, typename std::enable_if<!hasSerOutput<Type>::value, Type>::type* = 0){
		char* buf = (char*)&obj;
		size_t size = sizeof(Type);
		if (mIndexList.empty()) return *this;
		size_t lastPos = mIndexList.back();
		size_t len = mCurrent - lastPos;
		if (size != len) return *this;
		memcpy(buf, mBuffer + lastPos, len);
		mIndexList.pop_back();
		mCurrent = lastPos;
		checkMinify();
		return *this;
	}

	// ���������������
	Serializer& input(const Serializer& obj){
		char* buf = nullptr;
		size_t size = 0;
		Serializer& tSer = const_cast<Serializer&>(obj);
		tSer.encode(buf, size);
		input(buf, size);
		delete[] buf;
		return *this;
	}
	Serializer& output(Serializer& obj){
		char* buf = nullptr;
		size_t size = 0;
		outputEx(buf, size);
		obj.decode(buf, size);
		delete[] buf;
		return *this;
	}

	// ����String�൥������
	Serializer& input(const String& str)
	{
		size_t len = str.length();
		checkExpend(len);
		memcpy(mBuffer + mCurrent, str.data(), len);
		mIndexList.push_back(mCurrent);
		mCurrent += len;
		return *this;
	}
	Serializer& output(String& str){
		if (mIndexList.empty()) return *this;
		size_t lastPos = mIndexList.back();
		size_t len = mCurrent - lastPos;
		char* tBuf = new char[len + 1];
		memcpy(tBuf, mBuffer + lastPos, len);
		tBuf[len] = '\0';
		str = tBuf;
		delete[] tBuf;
		mIndexList.pop_back();
		mCurrent = lastPos;
		checkMinify();
		return *this;
	}

	// ʵ�ֶ��(�����汾,ע�⣺�ַ�������ΪString����,����ᱻ����const char* 4�ֽ�ָ�����ʹ���)
	template<typename Type, typename ...Args>
	Serializer& inputs(const Type& obj, Args&&...args){ return this->input(obj).inputs(std::forward<Args>(args)...); }
	template<typename Type>
	Serializer& inputs(const Type& obj){ return this->input(obj); }
	template<typename Type, typename ...Args>
	Serializer& outputs(Type& obj, Args&&...args){ return this->outputs(std::forward<Args>(args)...).output(obj); }
	template<typename Type>
	Serializer& outputs(Type& obj){ return this->output(obj); }

	// ��ǵ������������
	Serializer& input(Sign sign){
		return *this;
	}
	Serializer& output(Sign sign){
		if (sign == Omit)
			pop();
		return *this;
	}

	// ��������
	Serializer& input(const char* buf, size_t size){
		checkExpend(size);
		memcpy(mBuffer + mCurrent, buf, size);
		mIndexList.push_back(mCurrent);
		mCurrent += size;
		return *this;
	}
	// �������,��output�汾���ݲ����������,buf��Ԥ�ȷ����ڴ�,sizeҲ�����ջ��Ԫ�ش�Сһ��
	Serializer& output(char* buf, size_t size){
		if (mIndexList.empty()) return *this;
		size_t lastPos = mIndexList.back();
		size_t len = mCurrent - lastPos;
		if (size != len) return *this;
		memcpy(buf, mBuffer + lastPos, len);
		mIndexList.pop_back();
		mCurrent = lastPos;
		checkMinify();
		return *this;
	}
	// �������,��output�汾����ջ��Ԫ���������,buf��ҪΪnuller,size���ܳߴ� buf��Ҫ�ͷ�
	Serializer& outputEx(char*& buf, size_t& size){
		if (mIndexList.empty()) return *this;
		size_t lastPos = mIndexList.back();
		size = mCurrent - lastPos;
		buf = new char[size];
		memset(buf, 0, size);
		memcpy(buf, mBuffer + lastPos, size);
		mIndexList.pop_back();
		mCurrent = lastPos;
		checkMinify();
		return *this;
	}

	// ��������ķ�������
	template<typename Type>
	Serializer& operator<<(const Type& obj){ return input(obj); }
	Serializer& operator<<(const char* obj){ return input(String(obj)); }
	template<typename Type>
	Serializer& operator>>(Type& obj){ return output(obj); }
	Serializer& operator<<(Sign sign){ return input(sign); }
	Serializer& operator>>(Sign sign){ return output(sign); }
	
	// ���ն��ģ��ת��Ϊtuple���� ����Դ�tuple���Ƿ������� ��ת��ʱ�Ƿ���ڲ���
	template<typename Tuple>
	typename std::enable_if<!std::is_same<Tuple, std::tuple<>>::value, Tuple>::type
	toTuple(){
		Tuple tuple;
		ToTuple toTuple(this);
		TupleHelper->foreach(tuple, toTuple);
		return tuple;
	}
	template<typename Tuple>
	typename std::enable_if<std::is_same<Tuple, std::tuple<>>::value, std::tuple<>>::type
	toTuple(){
		return std::tuple<>();
	}

};


// ��vector֧�� ֻ֧�ַ��Ű汾 ��֧�ָ����汾
template<typename Type>
Serializer& operator<<(Serializer& ser, const std::vector<Type>& list)
{
	Serializer tSer;
	for (auto& it : list)
		tSer << it;
	ser << tSer;
	return ser;
}
template<typename Type>
Serializer& operator>>(Serializer& ser, std::vector<Type>& list)
{
	Serializer tSer;
	ser >> tSer;
	for (size_t i = 0; i < tSer.getCount(); ++i)
		list.emplace_back(tSer.at<Type>(i));
	return ser;
}


