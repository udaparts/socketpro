#pragma once
namespace SocketProAdapter
{
#ifndef _WIN32_WCE
namespace ServerSide
{

ref class CBaseService;

protected delegate void DM_I0_R0();

generic<typename R0>
protected delegate void DM_I0_R1([Out]R0 %r0);
generic<typename R0, typename R1>
protected delegate void DM_I0_R2([Out]R0 %r0, [Out]R1 %r1);
generic<typename R0, typename R1, typename R2>
protected delegate void DM_I0_R3([Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I0_R4([Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I0_R5([Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0>
protected delegate void DM_I1_R0(T0 t0);
generic<typename T0, typename R0>
protected delegate void DM_I1_R1(T0 t0, [Out]R0 %r0);
generic<typename T0, typename R0, typename R1>
protected delegate void DM_I1_R2(T0 t0, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename R0, typename R1, typename R2>
protected delegate void DM_I1_R3(T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I1_R4(T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I1_R5(T0 t0, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1>
protected delegate void DM_I2_R0(T0 t0, T1 t1);
generic<typename T0, typename T1, typename R0>
protected delegate void DM_I2_R1(T0 t0, T1 t1, [Out]R0 %r0);
generic<typename T0, typename T1, typename R0, typename R1>
protected delegate void DM_I2_R2(T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename R0, typename R1, typename R2>
protected delegate void DM_I2_R3(T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I2_R4(T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I2_R5(T0 t0, T1 t1, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2>
protected delegate void DM_I3_R0(T0 t0, T1 t1, T2 t2);
generic<typename T0, typename T1, typename T2, typename R0>
protected delegate void DM_I3_R1(T0 t0, T1 t1, T2 t2, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename R0, typename R1>
protected delegate void DM_I3_R2(T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2>
protected delegate void DM_I3_R3(T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I3_R4(T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I3_R5(T0 t0, T1 t1, T2 t2, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3>
protected delegate void DM_I4_R0(T0 t0, T1 t1, T2 t2, T3 t3);
generic<typename T0, typename T1, typename T2, typename T3, typename R0>
protected delegate void DM_I4_R1(T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1>
protected delegate void DM_I4_R2(T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2>
protected delegate void DM_I4_R3(T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I4_R4(T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I4_R5(T0 t0, T1 t1, T2 t2, T3 t3, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3, typename T4>
protected delegate void DM_I5_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0>
protected delegate void DM_I5_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1>
protected delegate void DM_I5_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2>
protected delegate void DM_I5_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I5_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I5_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
protected delegate void DM_I6_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0>
protected delegate void DM_I6_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1>
protected delegate void DM_I6_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2>
protected delegate void DM_I6_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I6_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I6_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);


generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
protected delegate void DM_I7_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0>
protected delegate void DM_I7_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1>
protected delegate void DM_I7_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2>
protected delegate void DM_I7_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I7_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I7_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
protected delegate void DM_I8_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0>
protected delegate void DM_I8_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1>
protected delegate void DM_I8_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2>
protected delegate void DM_I8_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I8_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I8_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
protected delegate void DM_I9_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0>
protected delegate void DM_I9_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1>
protected delegate void DM_I9_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2>
protected delegate void DM_I9_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I9_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I9_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
protected delegate void DM_I10_R0(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0>
protected delegate void DM_I10_R1(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1>
protected delegate void DM_I10_R2(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2>
protected delegate void DM_I10_R3(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3>
protected delegate void DM_I10_R4(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3);
generic<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3, typename R4>
protected delegate void DM_I10_R5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, [Out]R0 %r0, [Out]R1 %r1, [Out]R2 %r2, [Out]R3 %r3, [Out]R4 %r4);

[CLSCompliantAttribute(true)] 
public ref class CClientPeer abstract
{
private:
	ref class CUPushServerImpl : public IUPush
	{
	public:
		virtual bool Enter(array<int> ^Groups);
		virtual bool Broadcast(Object ^Message, array<int> ^Groups);
		virtual bool Broadcast(array<unsigned char> ^Message, array<int> ^Groups);
		virtual bool SendUserMessage(Object ^Message, String ^UserId);
		virtual bool SendUserMessage(String ^UserId, array<unsigned char> ^Message);
		virtual bool Exit();

	internal:
		CClientPeer ^m_cp;
	};

public:
	static const int SOCKET_NOT_FOUND = -1;
	static const int LEN_NOT_AVAILABLE = -1;
	static const int REQUEST_CANCELED = -2;
	static const int RETURN_DATA_INTERCEPTED = -3;

public:
	CClientPeer();

internal:
	virtual ~CClientPeer();

protected:
	int M_I0_R0(DM_I0_R0 ^f)
	{
		f();
		return SendResult(CurrentRequestID);
	}

protected:
	generic<typename R0>
	int M_I0_R1(DM_I0_R1<R0> ^f)
	{
		R0 r0;
		f(r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename R0, typename R1>
	int M_I0_R2(DM_I0_R2<R0, R1> ^f)
	{
		R0 r0;
		R1 r1;
		f(r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename R0, typename R1, typename R2>
	int M_I0_R3(DM_I0_R3<R0, R1, R2> ^f)
	{
		R0 r0;
		R1 r1;
		R2 r2;
		f(r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename R0, typename R1, typename R2, typename R3>
	int M_I0_R4(DM_I0_R4<R0, R1, R2, R3> ^f)
	{
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I0_R5(DM_I0_R5<R0, R1, R2, R3, R4> ^f)
	{
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0>
	int M_I1_R0(DM_I1_R0<A0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		f(a0);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename R0>
	int M_I1_R1(DM_I1_R1<A0, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		R0 r0;
		f(a0, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename R0, typename R1>
	int M_I1_R2(DM_I1_R2<A0, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		R0 r0;
		R1 r1;
		f(a0, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename R0, typename R1, typename R2>
	int M_I1_R3(DM_I1_R3<A0, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename R0, typename R1, typename R2, typename R3>
	int M_I1_R4(DM_I1_R4<A0, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I1_R5(DM_I1_R5<A0, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}
	
protected:
	generic<typename A0, typename A1>
	int M_I2_R0(DM_I2_R0<A0, A1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		f(a0, a1);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename R0>
	int M_I2_R1(DM_I2_R1<A0, A1, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		R0 r0;
		f(a0, a1, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename R0, typename R1>
	int M_I2_R2(DM_I2_R2<A0, A1, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		R0 r0;
		R1 r1;
		f(a0, a1, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename R0, typename R1, typename R2>
	int M_I2_R3(DM_I2_R3<A0, A1, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename R0, typename R1, typename R2, typename R3>
	int M_I2_R4(DM_I2_R4<A0, A1, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I2_R5(DM_I2_R5<A0, A1, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2>
	int M_I3_R0(DM_I3_R0<A0, A1, A2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		f(a0, a1, a2);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename R0>
	int M_I3_R1(DM_I3_R1<A0, A1, A2, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		R0 r0;
		f(a0, a1, a2, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename R0, typename R1>
	int M_I3_R2(DM_I3_R2<A0, A1, A2, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename R0, typename R1, typename R2>
	int M_I3_R3(DM_I3_R3<A0, A1, A2, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename R0, typename R1, typename R2, typename R3>
	int M_I3_R4(DM_I3_R4<A0, A1, A2, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I3_R5(DM_I3_R5<A0, A1, A2, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3>
	int M_I4_R0(DM_I4_R0<A0, A1, A2, A3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		f(a0, a1, a2, a3);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename R0>
	int M_I4_R1(DM_I4_R1<A0, A1, A2, A3, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		R0 r0;
		f(a0, a1, a2, a3, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename R0, typename R1>
	int M_I4_R2(DM_I4_R2<A0, A1, A2, A3, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename R0, typename R1, typename R2>
	int M_I4_R3(DM_I4_R3<A0, A1, A2, A3, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename R0, typename R1, typename R2, typename R3>
	int M_I4_R4(DM_I4_R4<A0, A1, A2, A3, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I4_R5(DM_I4_R5<A0, A1, A2, A3, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4>
	int M_I5_R0(DM_I5_R0<A0, A1, A2, A3, A4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		f(a0, a1, a2, a3, a4);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename R0>
	int M_I5_R1(DM_I5_R1<A0, A1, A2, A3, A4, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		R0 r0;
		f(a0, a1, a2, a3, a4, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename R0, typename R1>
	int M_I5_R2(DM_I5_R2<A0, A1, A2, A3, A4, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename R0, typename R1, typename R2>
	int M_I5_R3(DM_I5_R3<A0, A1, A2, A3, A4, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename R0, typename R1, typename R2, typename R3>
	int M_I5_R4(DM_I5_R4<A0, A1, A2, A3, A4, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I5_R5(DM_I5_R5<A0, A1, A2, A3, A4, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	int M_I6_R0(DM_I6_R0<A0, A1, A2, A3, A4, A5> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		f(a0, a1, a2, a3, a4, a5);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename R0>
	int M_I6_R1(DM_I6_R1<A0, A1, A2, A3, A4, A5, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		R0 r0;
		f(a0, a1, a2, a3, a4, a5, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename R0, typename R1>
	int M_I6_R2(DM_I6_R2<A0, A1, A2, A3, A4, A5, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, a5, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename R0, typename R1, typename R2>
	int M_I6_R3(DM_I6_R3<A0, A1, A2, A3, A4, A5, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, a5, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename R0, typename R1, typename R2, typename R3>
	int M_I6_R4(DM_I6_R4<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, a5, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I6_R5(DM_I6_R5<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, a5, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	int M_I7_R0(DM_I7_R0<A0, A1, A2, A3, A4, A5, A6> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		f(a0, a1, a2, a3, a4, a5, a6);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R0>
	int M_I7_R1(DM_I7_R1<A0, A1, A2, A3, A4, A5, A6, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		R0 r0;
		f(a0, a1, a2, a3, a4, a5, a6, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R0, typename R1>
	int M_I7_R2(DM_I7_R2<A0, A1, A2, A3, A4, A5, A6, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, a5, a6, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R0, typename R1, typename R2>
	int M_I7_R3(DM_I7_R3<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, a5, a6, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R0, typename R1, typename R2, typename R3>
	int M_I7_R4(DM_I7_R4<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, a5, a6, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I7_R5(DM_I7_R5<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, a5, a6, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	int M_I8_R0(DM_I8_R0<A0, A1, A2, A3, A4, A5, A6, A7> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		f(a0, a1, a2, a3, a4, a5, a6, a7);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename R0>
	int M_I8_R1(DM_I8_R1<A0, A1, A2, A3, A4, A5, A6, A7, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		R0 r0;
		f(a0, a1, a2, a3, a4, a5, a6, a7, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename R0, typename R1>
	int M_I8_R2(DM_I8_R2<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, a5, a6, a7, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename R0, typename R1, typename R2>
	int M_I8_R3(DM_I8_R3<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, a5, a6, a7, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename R0, typename R1, typename R2, typename R3>
	int M_I8_R4(DM_I8_R4<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, a5, a6, a7, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I8_R5(DM_I8_R5<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, a5, a6, a7, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	int M_I9_R0(DM_I9_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename R0>
	int M_I9_R1(DM_I9_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		R0 r0;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename R0, typename R1>
	int M_I9_R2(DM_I9_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename R0, typename R1, typename R2>
	int M_I9_R3(DM_I9_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename R0, typename R1, typename R2, typename R3>
	int M_I9_R4(DM_I9_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I9_R5(DM_I9_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

//
protected:
	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	int M_I10_R0(DM_I10_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		return SendResult(CurrentRequestID);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename R0>
	int M_I10_R1(DM_I10_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		R0 r0;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, r0);
		return SendResult(CurrentRequestID, r0);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename R0, typename R1>
	int M_I10_R2(DM_I10_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		R0 r0;
		R1 r1;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, r0, r1);
		return SendResult(CurrentRequestID, r0, r1);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename R0, typename R1, typename R2>
	int M_I10_R3(DM_I10_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		R0 r0;
		R1 r1;
		R2 r2;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, r0, r1, r2);
		return SendResult(CurrentRequestID, r0, r1, r2);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename R0, typename R1, typename R2, typename R3>
	int M_I10_R4(DM_I10_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, r0, r1, r2, r3);
		return SendResult(CurrentRequestID, r0, r1, r2, r3);
	}

	generic<typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename R0, typename R1, typename R2, typename R3, typename R4>
	int M_I10_R5(DM_I10_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4> ^f)
	{
		A0 a0;
		m_UQueue->Load(a0);
		A1 a1;
		m_UQueue->Load(a1);
		A2 a2;
		m_UQueue->Load(a2);
		A3 a3;
		m_UQueue->Load(a3);
		A4 a4;
		m_UQueue->Load(a4);
		A5 a5;
		m_UQueue->Load(a5);
		A6 a6;
		m_UQueue->Load(a6);
		A7 a7;
		m_UQueue->Load(a7);
		A8 a8;
		m_UQueue->Load(a8);
		A9 a9;
		m_UQueue->Load(a9);
		R0 r0;
		R1 r1;
		R2 r2;
		R3 r3;
		R4 r4;
		f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, r0, r1, r2, r3, r4);
		return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
	}

public:
	/// <summary>
	/// Discard all of previously batched returning results.
	/// </summary>
	bool AbortBatching();
	
	/// <summary>
	/// Send all of previously batched returning results in memory onto a remote client.
	/// </summary>
	bool CommitBatching();
/*
	/// <summary>
	/// Join one or more chat groups identified by a given number of bitwise or-ed nGroupIDs (1, 2, 4, 8, 16, or ......).
	/// Call this method from non-HTTP service.
	/// </summary>
	bool Enter(int nGroupIDs);
	
	/// <summary>
	/// leave from a chat group.
	/// Call this method from non-HTTP service.
	/// </summary>
	bool Exit();*/
	bool GetInterfaceAttributes(int %nMTU, int %nMaxSpeed, tagInterfaceType %Type, int %nMask);
	
	int GetMySpecificBytes(array<BYTE> ^Buffer);
	String^ GetPeerName(int %nPeerPort);
	
	/// <summary>
	/// Get an array of request ids in queue to be processed into an array of buffer you allocated.
	/// The method returns the actual number of request ids obtained. 
	/// </summary>
	/// <param name="psRequestID">The array buffer for receiving request ids.</param>
	int GetRequestIDsInQueue(array<short> ^psRequestID);

	/// <summary>
	/// Get an array of request ids in queue to be processed into an array of buffer you allocated.
	/// The method returns the actual number of request ids obtained. 
	/// </summary>
	/// <param name="psRequestID">The array buffer for receiving request ids.</param>
	/// <param name="nSize">The size of the array psRequestID.</param>
	int GetRequestIDsInQueue(array<short> ^psRequestID, int nSize);
	
	String^ GetSockAddr(int %nSocketPort);
	
	/// <summary>
	/// Post a message for close this socket connection with a given error code (sError)
	/// </summary>
	bool PostClose(short sError);
	
	int RetrieveBuffer(IntPtr pBuffer, int nLen, bool bPeek);
	int RetrieveBuffer(IntPtr pBuffer, int nLen);
//	int SendErrorMessage(short sRequestID, int nErrorCode, String^ strErrorMessage);
	
	/// <summary>
	/// Send a result to remote client. In general, this method is not preferred. Use the method SendResult instead for less code, better thread safety, and simplicity.
	/// </summary>
	int SendReturnData(short sRequestID, IntPtr pBuffer, int nLen);
	
	/// <summary>
	/// Send a result to remote client. In general, this method is not preferred. Use the method SendResult instead for less code, better thread safety, and simplicity.
	/// </summary>
	int SendReturnData(array<BYTE> ^pBuffer, short sRequestID);
	
	/// <summary>
	/// Send a result to remote client. In general, this method is not preferred. Use the method SendResult instead for less code, better thread safety, and simplicity.
	/// </summary>
	int SendReturnData(short sRequestID, array<BYTE> ^pBuffer, int nLen);
	
	/// <summary>
	/// Send a result to remote client. In general, this method is not preferred. Use the method SendResult instead for less code, better thread safety, and simplicity.
	/// </summary>
	int SendReturnData(short sRequestID, CUQueue ^UQueue);
	
	/// <summary>
	/// Send an empty result to remote client. In general, this method is not preferred. Use the method SendResult instead for less code, better thread safety, and simplicity.
	/// </summary>
	int SendReturnData(short sRequestID);
	
	/// <summary>
	/// Send an empty result to remote client. In general, this method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	int SendResult(short sRequestID);


	/// <summary>
	/// Send a result (UQueue) to remote client. 
	/// </summary>
	int SendResult(short sRequestID, CUQueue ^UQueue);

	/// <summary>
	/// Send a result (UQueue) to remote client.
	/// </summary>
	int SendResult(short sRequestID, CScopeUQueue ^UQueue);

	/// <summary>
	/// Send a result to remote client. In general, this generic method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	generic<typename T0>
	int SendResult(short sRequestID, T0 data0)
	{
		CScopeUQueue UQueue;
		if(TransferServerException)
			UQueue.m_UQueue->Push((long)0); //required by client
		UQueue.Save(data0);
		return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
	}
	
	/// <summary>
	/// Send a result to remote client. In general, this generic method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	generic<typename T0, typename T1>
	int SendResult(short sRequestID, T0 data0, T1 data1)
	{
		CScopeUQueue UQueue;
		if(TransferServerException)
			UQueue.m_UQueue->Push((long)0); //required by client
		UQueue.Save(data0);
		UQueue.Save(data1);
		return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
	}
	
	/// <summary>
	/// Send a result to remote client. In general, this generic method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	generic<typename T0, typename T1, typename T2>
	int SendResult(short sRequestID, T0 data0, T1 data1, T2 data2)
	{
		CScopeUQueue UQueue;
		if(TransferServerException)
			UQueue.m_UQueue->Push((long)0); //required by client
		UQueue.Save(data0);
		UQueue.Save(data1);
		UQueue.Save(data2);
		return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
	}
	
	/// <summary>
	/// Send a result to remote client. In general, this generic method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	generic<typename T0, typename T1, typename T2, typename T3>
	int SendResult(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3)
	{
		CScopeUQueue UQueue;
		if(TransferServerException)
			UQueue.m_UQueue->Push((long)0); //required by client
		UQueue.Save(data0);
		UQueue.Save(data1);
		UQueue.Save(data2);
		UQueue.Save(data3);
		return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
	}
	
	/// <summary>
	/// Send a result to remote client. In general, this generic method is preferred to the methods SendReturnData for less code, better thread safety, and simplicity.
	/// </summary>
	generic<typename T0, typename T1, typename T2, typename T3, typename T4>
	int SendResult(short sRequestID, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4)
	{
		CScopeUQueue UQueue;
		if(TransferServerException)
			UQueue.m_UQueue->Push((long)0); //required by client
		UQueue.Save(data0);
		UQueue.Save(data1);
		UQueue.Save(data2);
		UQueue.Save(data3);
		UQueue.Save(data4);
		return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
	}

	bool SetBlowFish(array<BYTE> ^pKey);
	
	/// <summary>
	/// Call this method to reduce allocated memory associated with this socket.
	/// </summary>
	void ShrinkMemory();

	CBaseService^ GetBaseService();
	
	bool StartBatching();

	void DropRequestResult(short sRequestId);

	void DropCurrentSlowRequest();
	bool IsClosing();
	
internal:
	CUQueue^ GetUQueue();

public:
	property int AssociatedThreadID
	{
		int get()
		{
			return g_SocketProLoader.GetAssociatedThreadID(m_hSocket);
		}
	}

	/// <summary>
	/// A property indicating the max message size allowed.
	/// Setting this property to a proper value reduces the chance of DoS attack.
	/// </summary>
	property int MaxMessageSize
	{
		int get()
		{
			if(g_SocketProLoader.GetHTTPMaxMessageSize == NULL)
				return -1;
			return g_SocketProLoader.GetHTTPMaxMessageSize(m_hSocket);
		}

		void set(int nMaxMessageSize)
		{
			if(g_SocketProLoader.SetHTTPMaxMessageSize != NULL)
			{
				g_SocketProLoader.SetHTTPMaxMessageSize(m_hSocket, nMaxMessageSize);
			}
		}
	}

	/// <summary>
	/// A property turning on/off buffering requests automatically. By default, it is true.
	/// If you set the property to false, you have to call RetrieveBuffer by yourself. Usually,
	/// you should NOT set the property to false.
	/// </summary>
	property bool AutoBuffer
	{
		bool get()
		{
			return m_bAutoBuffer;
		}
		void set(bool bAuto)
		{
			m_bAutoBuffer = bAuto;
		}
	}

	property IUPush^ Push
	{
		IUPush^ get()
		{
			return m_pPush;
		}
	}

	property int BytesBatched
	{
		int get()
		{
			return g_SocketProLoader.GetBytesBatched(m_hSocket);
		}
	}

	property __int64 BytesReceived
	{
		__int64 get()
		{
			__int64 llBytes = 0;
			ULONG ulHigh;
			ULONG ulLow = g_SocketProLoader.GetBytesIn(m_hSocket, &ulHigh);
			if(ulHigh > 0)
			{
				llBytes = ulHigh;
				llBytes = (llBytes << 32);
			}
			llBytes += ulLow;
			return llBytes;
		}
	}

	property __int64 BytesSent
	{
		__int64 get()
		{
			__int64 llBytes = 0;
			ULONG ulHigh;
			ULONG ulLow = g_SocketProLoader.GetBytesOut(m_hSocket, &ulHigh);
			if(ulHigh > 0)
			{
				llBytes = ulHigh;
				llBytes = (llBytes << 32);
			}
			llBytes += ulLow;
			return llBytes;
		}
	}
	
	/// <summary>
	/// A property indicating key information data from a client.
	/// You can use this property to check various versions of client USocket.dll and its underlying socket implementation.
	/// You can also use this property to check client implementation version for a sevice.
	/// </summary>
	property USOCKETLib::CSwitchInfo ClientInfo
	{
		USOCKETLib::CSwitchInfo get()
		{
			USOCKETLib::CSwitchInfo mSI;
			CSwitchInfo nSI;
			g_SocketProLoader.GetClientInfo(m_hSocket, &nSI);
			mSI.m_ulParam1 = nSI.m_ulParam1;
			mSI.m_ulParam2 = nSI.m_ulParam2;
			mSI.m_ulParam3 = nSI.m_ulParam3;
			mSI.m_ulParam4 = nSI.m_ulParam4;
			mSI.m_ulParam5 = nSI.m_ulParam5;
			mSI.m_ulSvsID = nSI.m_ulSvsID;
			mSI.m_usUSockMajor = nSI.m_usUSockMajor;
			mSI.m_usUSockMinor = nSI.m_usUSockMinor;
			mSI.m_usVerMajor = nSI.m_usVerMajor;
			mSI.m_usVerMinor = nSI.m_usVerMinor;
			return mSI;
		}
	}
	
	/// <summary>
	/// A property indicating key information data for this server.
	/// You set this property to tell usktpro.dll and underlying socket implementation versions.
	/// You can also use this property to tell server implementation for a service.
	/// </summary>
	property USOCKETLib::CSwitchInfo ServerInfo
	{
		USOCKETLib::CSwitchInfo get()
		{
			USOCKETLib::CSwitchInfo mSI;
			CSwitchInfo nSI;
			g_SocketProLoader.GetServerInfo(m_hSocket, &nSI);
			mSI.m_ulParam1 = nSI.m_ulParam1;
			mSI.m_ulParam2 = nSI.m_ulParam2;
			mSI.m_ulParam3 = nSI.m_ulParam3;
			mSI.m_ulParam4 = nSI.m_ulParam4;
			mSI.m_ulParam5 = nSI.m_ulParam5;
			mSI.m_ulSvsID = nSI.m_ulSvsID;
			mSI.m_usUSockMajor = nSI.m_usUSockMajor;
			mSI.m_usUSockMinor = nSI.m_usUSockMinor;
			mSI.m_usVerMajor = nSI.m_usVerMajor;
			mSI.m_usVerMinor = nSI.m_usVerMinor;
			return mSI;
		}

		void set(USOCKETLib::CSwitchInfo sw)
		{
			CSwitchInfo nSI;
			nSI.m_ulParam1 = sw.m_ulParam1;
			nSI.m_ulParam2 = sw.m_ulParam2;
			nSI.m_ulParam3 = sw.m_ulParam3;
			nSI.m_ulParam4 = sw.m_ulParam4;
			nSI.m_ulParam5 = sw.m_ulParam5;
			nSI.m_ulSvsID = sw.m_ulSvsID;
			nSI.m_usUSockMajor = sw.m_usUSockMajor;
			nSI.m_usUSockMinor = sw.m_usUSockMinor;
			nSI.m_usVerMajor = sw.m_usVerMajor;
			nSI.m_usVerMinor = sw.m_usVerMinor;
			g_SocketProLoader.SetServerInfo(m_hSocket, &nSI);
		}
	}

	property int ConsumedMemory
	{
		int get()
		{
			return g_SocketProLoader.GetTotalMemory(m_hSocket);
		}
	}
	
	property int CountOfMySpecificBytes
	{
		int get()
		{
			return g_SocketProLoader.GetCountOfMySpecificBytes(m_hSocket);
		}
	}
	
	property short CurrentRequestID
	{
		short get()
		{
			return m_sCurrentRequestId;
		}
	}

	property int CurrentRequestLen
	{
		int get()
		{
			return g_SocketProLoader.GetCurrentRequestLen(m_hSocket);
		}
	}
	
	/// <summary>
	/// A property for an array of group ids.
	/// </summary>
	property array<long>^ JoinedGroups
	{
		array<long>^ get()
		{
			unsigned long n;
			List<long> lst;
			if(g_SocketProLoader.GetJoinedGroupIds)
			{
				unsigned long ulGroup = g_SocketProLoader.GetCountOfChatGroups();
				CScopeUQueue su;
				unsigned long ulSize = ulGroup*sizeof(unsigned long);
				if(su.UQueue->GetInternalUQueue()->GetMaxSize() < ulSize)
					su.UQueue->GetInternalUQueue()->ReallocBuffer(ulSize + 10);
				unsigned long *pGroup = (unsigned long *)su.UQueue->GetInternalUQueue()->GetBuffer();
				ulGroup = g_SocketProLoader.GetJoinedGroupIds(m_hSocket, pGroup, ulGroup);
				for(n=0; n<ulGroup; n++)
					lst.Add((long)(pGroup[n]));
			}
			else
			{
				int n = 0;
				unsigned long ulGroup = 1;
				unsigned long ulGroups = g_SocketProLoader.GetJoinedGroup(m_hSocket);
				while(n<32)
				{
					if((ulGroup & ulGroups) == ulGroup)
						lst.Add(ulGroup);
					++n;
					ulGroup <<= 1;
				}
			}
			return lst.ToArray();
		}
	}

	property bool IsBatching
	{
		bool get()
		{
			return g_SocketProLoader.IsBatching(m_hSocket);
		}
	}

	property bool IsSameEndian
	{
		bool get()
		{
			return g_SocketProLoader.IsSameEndian(m_hSocket);
		}
	}

	property __int64 LastRcvTime
	{
		__int64 get()
		{
			return g_SocketProLoader.GetLastRcvTime(m_hSocket);
		}
	}

	property __int64 LastSndTime
	{
		__int64 get()
		{
			return g_SocketProLoader.GetLastSndTime(m_hSocket);
		}
	}

	property String^ Password
	{
		String^ get()
		{
			WCHAR strPassword[256] = {0};
			g_SocketProLoader.GetPassword(m_hSocket, strPassword, sizeof(strPassword)/sizeof(WCHAR));
			return gcnew String(strPassword);
		}
	}

	property int RcvBufferSize
	{
		int get()
		{
			return g_SocketProLoader.GetRcvBufferSize(m_hSocket);
		}
	}

	property int RcvBytesInQueue
	{
		int get()
		{
			return g_SocketProLoader.GetRcvBytesInQueue(m_hSocket);
		}
	}
	property int RequestsInQueue
	{
		int get()
		{
			return g_SocketProLoader.QueryRequestsInQueue(m_hSocket);
		}
	}
	property int SndBytesInQueue
	{
		int get()
		{
			return g_SocketProLoader.GetSndBytesInQueue(m_hSocket);
		}
	}
	property int SndBufferSize
	{
		int get()
		{
			return g_SocketProLoader.GetSndBufferSize(m_hSocket);
		}
	}
	property int Socket
	{
		int get()
		{
			return m_hSocket;
		}
	}
	
	property int SvsID
	{
		int get()
		{
			return g_SocketProLoader.GetSvsID(m_hSocket);
		}
	}
	property String^ UserID
	{
		String^ get()
		{
			WCHAR strUID[256] = {0};
			g_SocketProLoader.GetUID(m_hSocket, strUID, sizeof(strUID)/sizeof(WCHAR));
			return gcnew String(strUID);
		}

		void set(String ^strUserId)
		{
			if(strUserId == nullptr)
			{
				g_SocketProLoader.SetUserID(m_hSocket, L"");
			}
			else
			{
				pin_ptr<const wchar_t> wch = PtrToStringChars(strUserId);
				g_SocketProLoader.SetUserID(m_hSocket, wch);
			}
		}
	}
	property bool ZipEnbaled
	{
		bool get()
		{
			return g_SocketProLoader.GetZip(m_hSocket);
		}
		void set(bool bZip)
		{
			g_SocketProLoader.SetZip(m_hSocket, bZip);
		}
	}
	
	property USOCKETLib::tagZipLevel ZipLevel
	{
		USOCKETLib::tagZipLevel get()
		{
			if(g_SocketProLoader.GetZipLevel != NULL)
			{
				return (USOCKETLib::tagZipLevel)g_SocketProLoader.GetZipLevel(m_hSocket);
			}
			return USOCKETLib::tagZipLevel::zlDefault;
		}

		void set(USOCKETLib::tagZipLevel zl)
		{
			if(g_SocketProLoader.SetZipLevel != NULL)
			{
				g_SocketProLoader.SetZipLevel(m_hSocket, (tagZipLevel)zl);
			}
		}
	}
	
	property bool TransferServerException
	{
		bool get()
		{
			CSwitchInfo ServerInfo;
			memset(&ServerInfo, 0, sizeof(ServerInfo));
			g_SocketProLoader.GetServerInfo(m_hSocket, &ServerInfo);
			return ((ServerInfo.m_ulParam5 & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
		}
	}

	property USOCKETLib::tagEncryptionMethod EncryptionMethod
	{
		USOCKETLib::tagEncryptionMethod get()
		{
			return (USOCKETLib::tagEncryptionMethod)g_SocketProLoader.GetEncryptionMethod(m_hSocket);
		}
		void set(USOCKETLib::tagEncryptionMethod em)
		{
			g_SocketProLoader.SetEncryptionMethod(m_hSocket, (tagEncryptionMethod)em);
		}
	}

	static int SendReturnData(int hSocket, short sRequestID, CUQueue ^UQueue);
	static int SendReturnData(int hSocket, array<BYTE> ^pBuffer, short sRequestID);
	static int SendReturnData(int hSocket, short sRequestID, array<BYTE> ^pBuffer, int nLen);
	static int SendReturnData(int hSocket, short sRequestID, IntPtr pBuffer, int nLen);
	static int RetrieveBuffer(int hSocket, IntPtr pBuffer, int nLen, bool bPeek);
	static int RetrieveBuffer(int hSocket, IntPtr pBuffer, int nLen);
	

	/// <summary>
	/// A property indicating socket handle.
	/// Look at this property anywhere within a worker thread.
	/// </summary>
	static property int AssociatedSocket
	{
		int get()
		{
			return g_SocketProLoader.GetAssociatedSocket();
		}
	}
	
	/// <summary>
	/// A property indicating if the current thread is main thread.
	/// </summary>
	static property bool InMainThread
	{
		bool get()
		{
			return (g_SocketProLoader.GetMainThreadID() == GetCurrentThreadId());
		}
	}

	/// <summary>
	/// A property indicating if the server has received the request Cancel from a client.
	/// Look at this property anywhere and anytime within a worker thread.
	/// </summary>
	static property bool IsCanceled
	{
		bool get()
		{
			return g_SocketProLoader.IsCanceled();
		}
	}

	static property int LastSocketError
	{
		int get()
		{
			return g_SocketProLoader.GetLastSocketError();
		}
	}

protected:
	virtual void OnFastRequestArrive(short sRequestID, int nLen) abstract = 0;
	virtual int OnSlowRequestArrive(short sRequestID, int nLen) abstract = 0;
	
	/// <summary>
	/// The virtual function will be called when a socket is going to be either closed or switched for a new service.
	/// </summary>
	/// <param name="bClosing">If the parameter is true, the socket will be closed with error code nInfo. 
	/// Otherwise, the socket will be switched for new service nInfo.</param>
	/// <param name="nInfo">An interger value indicating either an error code or new service id.</param>
	virtual void OnReleaseResource(bool bClosing, int nInfo);
	
	/// <summary>
	/// The virtual function will be called when a socket is going to switch for this service.
	/// </summary>
	/// <param name="nSvsID">An interger value indicating a previous service id. 
	/// For the very first switch after a socket connection, it must be sidStartup.</param>
	virtual void OnSwitchFrom(int nSvsID);

	virtual void OnReceive(int nError);
	virtual void OnSend(int nError);
	virtual void OnBaseRequestCame(short sRequestID);
	virtual void OnDispatchingSlowRequest(short sRequestID);
	virtual void OnSlowRequestProcessed(short sRequestID);
	virtual void OnChatRequestComing(USOCKETLib::tagChatRequestID ChatRequestId, Object ^Param0, Object ^Param1);
	virtual void OnChatRequestCame(USOCKETLib::tagChatRequestID ChatRequestId);
	virtual bool OnSendReturnData(short sRequestID, int nLen, IntPtr pBuffer);

protected:
	CUQueue ^m_UQueue;

private:
	short m_sCurrentRequestId;

internal:
	//m_bAutoBuffer is true by default
	//if m_bAutoBuffer is true, all of fast and slow requests are automatically buffered into m_UQueue
	bool m_bAutoBuffer;
	CUPushServerImpl	^m_pPush;
	unsigned int	m_hSocket;
	unsigned long	m_ulTickCountReleased;
	void OnFRA(short sRequestID, int nLen);
	int OnSRA(short sRequestID, int nLen);
	void OnRR(bool bClosing, int nInfo);
	void OnSF(int nSvsID);
	void OnR(int nError);
	void OnS(int nError);
	void OnBRC(short sRequestID);
	void OnDSR(short sRequestID);
	void OnSRP(short sRequestID);
	void OnCRComing(USOCKETLib::tagChatRequestID sRequestID, Object ^Param0, Object ^Param1);
	void OnCRCame(USOCKETLib::tagChatRequestID sRequestID);
	bool OnSRData(short sRequestID, int nLen, IntPtr pBuffer);
	static void ConvertFromObjectToVariant(Object ^obj, VARIANT *pvt);
}; //class CClientPeer

/// <summary>
/// An interface for sending chat messages through HTTP push. 
/// Note that all of methods are thread-safe.
/// </summary>
[CLSCompliantAttribute(true)] 
public interface class IUHttpPush
{
	/// <summary>
	/// Join one or more chat groups (GroupIds) through HTTP service with a given lease time.
	/// GroupIds can be null or nothing. If so, you can use the chat id string to send a message to other clients.
	/// If successful, the method returns a unique HTTP server chat session ID, which will be used by other HTTP chat methods.
	/// The returned chat session is associated with this client IP address. 
	/// </summary>
	String^ Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime);
	
	/// <summary>
	/// Join one or more chat groups (GroupIds) through HTTP service with a given lease time.
	/// GroupIds can be null or nothing. If so, you can use the chat id string to send a message to other clients.
	/// If successful, the method returns a unique HTTP server chat session ID, which will be used by other HTTP chat methods.
	/// The returned chat session is associated with the ip address (strIpAddr) like '111.222.212.121'. 
	/// If the input strIpAddr is null, empty or invalid, SocketPro will use the current client IP address instead.
	/// </summary>
	String^ Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime, String^ strIpAddr);
	
	/// <summary>
	/// Subscribe chat messages through a given HTTP server push session.
	/// </summary>
	bool HTTPSubscribe(String^ strSessionId, int nTimeout, String ^strCrossSiteJSCallback);
	
	/// <summary>
	/// Explicitly exit the previously joined chat groups identified by a given HTTP server push session ID.
	/// </summary>
	bool Exit(String^ strSessionId);
	
	/// <summary>
	/// Send a message (msg) to a user (strUserID) through HTTP service.
	/// </summary>
	bool SendUserMessage(String^ strSessionId, String^ strUserID, Object ^msg);
	
	/// <summary>
	/// Send a message onto a number of chat groups (GroupIds) of clients through HTTP service.
	/// </summary>
	bool Speak(String^ strSessionId, Object ^msg, array<long>^ GroupIds);
	
	/// <summary>
	/// Get chat group ids from a given chat session string.
	/// </summary>
	array<long>^ GetHttpChatGroupIds(String^ strSessionId);
};

[CLSCompliantAttribute(true)] 
public ref class CHttpPeerBase abstract : public CClientPeer
{
private:
	ref class CHttpPushImpl : IUHttpPush
	{
	public:
		/// <summary>
		/// Join one or more chat groups (GroupIds) through HTTP service with a given lease time.
		/// GroupIds can be null or nothing. If so, you can use the chat id string to send a message to other clients.
		/// If successful, the method returns a unique HTTP server chat session ID, which will be used by other HTTP chat methods.
		/// The returned chat session is associated with this client IP address. 
		/// </summary>
		virtual String^ Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime);
		
		/// <summary>
		/// Join one or more chat groups (GroupIds) through HTTP service with a given lease time.
		/// GroupIds can be null or nothing. If so, you can use the chat id string to send a message to other clients.
		/// If successful, the method returns a unique HTTP server chat session ID, which will be used by other HTTP chat methods.
		/// The returned chat session is associated with the ip address (strIpAddr) like '111.222.212.121'. 
		/// If the input strIpAddr is null, empty or invalid, SocketPro will use the current client IP address instead.
		/// </summary>
		virtual String^ Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime, String^ strIpAddr);
		
		/// <summary>
		/// Subscribe chat messages through a given HTTP server push session.
		/// </summary>
		virtual bool HTTPSubscribe(String^ strSessionId, int nTimeout, String ^strCrossSiteJSCallback);
		
		/// <summary>
		/// Explicitly exit the previously joined chat groups identified by a given HTTP server push session ID.
		/// </summary>
		virtual bool Exit(String^ strSessionId);
		
		/// <summary>
		/// Send a message (msg) to a user (strUserID) through HTTP service.
		/// </summary>
		virtual bool SendUserMessage(String^ strSessionId, String^ strUserID, Object ^msg);
		
		/// <summary>
		/// Send a message onto a number of chat groups (GroupIds) of clients through HTTP service.
		/// </summary>
		virtual bool Speak(String^ strSessionId, Object ^msg, array<long>^ GroupIds);
		
		/// <summary>
		/// Get chat group ids from a given chat session string.
		/// </summary>
		virtual array<long>^ GetHttpChatGroupIds(String^ strSessionId);

	internal:
		CHttpPeerBase	^m_HttpPeer;
	};

public:
	CHttpPeerBase();

internal:
	virtual ~CHttpPeerBase();

public:
	bool SetResponseHeader(String ^strUTF8Header, String ^strUTF8Value);
	void SetResponseCode(int nHttpErrorCode);
	
	/// <summary>
	/// Send a text result in utf-8 format.
	/// </summary>
	int SendResult(short sRequestID, String ^strData) 
#if _MSC_VER < 1500
		new
#endif
	;

	property IUHttpPush^ HttpPush
	{
		IUHttpPush^ get()
		{
			return m_HttpPush;
		}
	}
	
	property tagHTTPRequest Request
	{
		tagHTTPRequest get()
		{
			return (tagHTTPRequest)m_HttpRequest;
		}
	}

	property String^ PathName
	{
		String^ get()
		{
			if(m_qQueue->GetSize() == 0)
				return gcnew String(L"");
			unsigned int nLen = 0;
			char *strQuery = (char*)m_qQueue->GetBuffer();
			const char *str = ::strstr(strQuery, "?");
			if(str)
				nLen = (unsigned int)(str - strQuery);
			else
				nLen = m_qQueue->GetSize() - 1;
			return gcnew String(strQuery, 0, nLen);
		}
	}

	property Dictionary<String^, String^>^ Params
	{
		Dictionary<String^, String^>^ get()
		{
			int n;
			if(m_qQueue->GetSize() == 0 || m_Params == nullptr)
				m_Params = gcnew Dictionary<String^, String^>();
			if(m_Params->Count != 0)
				return m_Params;
			char *strQuery = (char*)m_qQueue->GetBuffer();
			const char *str = ::strstr(strQuery, "?");
			if(!str)
				return m_Params;
			String ^strParams = gcnew String((const char*)(++str));
			array<String^> ^sep = {gcnew String(L"&")};
			array<String^> ^sepH = {gcnew String(L"=")};
			array<String^> ^aHeader = strParams->Split(sep, System::StringSplitOptions::RemoveEmptyEntries);
			for(n=0; n<aHeader->Length; n++)
			{
				array<String^> ^kv = aHeader[n]->Split(sepH, System::StringSplitOptions::RemoveEmptyEntries);
				if(kv != nullptr && kv->Length == 2 && kv[0] != nullptr && kv[0]->Length > 0)
					(*m_Params)[kv[0]] = kv[1];
			}
			return m_Params;
		}
	}

	property Dictionary<String^, String^> ^Headers
	{
		Dictionary<String^, String^>^ get()
		{
			int n;
			if(m_qHeader->GetSize() == 0)
				return nullptr;
			if(m_Headers != nullptr)
				return m_Headers;
			m_Headers = gcnew Dictionary<String^, String^>();
			String ^headers = gcnew String((const char*)m_qHeader->GetBuffer());
			array<String^> ^sep = {gcnew String(L"\r\n")};
			array<String^> ^sepH = {gcnew String(L": ")};
			array<String^> ^aHeader = headers->Split(sep, System::StringSplitOptions::RemoveEmptyEntries);
			for(n=0; n<aHeader->Length; n++)
			{
				array<String^> ^kv = aHeader[n]->Split(sepH, System::StringSplitOptions::RemoveEmptyEntries);
				if(kv != nullptr && kv->Length == 2 && kv[0] != nullptr && kv[0]->Length > 0)
					(*m_Headers)[kv[0]] = kv[1];
			}
			return m_Headers;
		}
	}

	property String^ Query
	{
		String^ get()
		{
			if(m_qQueue->GetSize() == 0)
				return gcnew String(L"");
			return gcnew String((const char*)m_qQueue->GetBuffer());
		}
	}

	property double HTTPClientVersion
	{
		double get()
		{
			return m_dVersion;
		}
	}
	
	/// <summary>
	/// A property indicating if SocketPro server should automatically partition a large request.
	/// Note that this property has a role on HTTP multipart request only.
	/// Setting this property to true may reduce server memory footprint and increase scalability for processing large HTTP request.
	/// </summary>
	property bool AutoPartition
	{
		bool get()
		{
			if(g_SocketProLoader.GetHTTPAutoPartition == NULL)
				return false;
			return g_SocketProLoader.GetHTTPAutoPartition(m_hSocket);
		}
		void set(bool b)
		{
			if(g_SocketProLoader.SetHTTPAutoPartition != NULL)
			{
				g_SocketProLoader.SetHTTPAutoPartition(m_hSocket, b);
			}
		}
	}
protected:
	virtual void OnFastRequestArrive(short sRequestID, int nLen) override;
	virtual void OnSwitchFrom(int ServiceID) override;

internal:
	static unsigned long UriDecode(const unsigned char *strIn, unsigned long nLenIn, unsigned char *strOut);
			
private:
	CInternalUQueue	*m_qQueue;	//store HTTP Query
	CInternalUQueue	*m_qHeader;	//store HTTP Headers
	enumHTTPRequest	m_HttpRequest;
	double	m_dVersion;
	Dictionary<String^, String^> ^m_Params;
	Dictionary<String^, String^> ^m_Headers;
	CHttpPushImpl	^m_HttpPush;
}; //CHttpPeerBase

};
#endif
};
