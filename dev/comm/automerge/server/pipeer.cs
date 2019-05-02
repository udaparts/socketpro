
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

class CPiPeer : CClientPeer
{
    [RequestAttr(piConst.idComputePi, true)]
    private double Compute(double dStart, double dStep, int nNum)
    {
        double dX = dStart + dStep / 2;
        double dd = dStep * 4.0;
        double d = 0.0;
        for (int n = 0; n < nNum; n++)
        {
            dX += dStep;
            d += dd / (1 + dX * dX);
        }
        return d;
    }
}

