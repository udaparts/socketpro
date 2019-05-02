

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class Pi : CAsyncServiceHandler
{
	public Pi() : base(piConst.sidPi)
	{
	}
}
