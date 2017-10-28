using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;

class CYourServer : CSocketProServer
{
    public static CSqlMasterPool<CMysql, CDataSet> Master = new CSqlMasterPool<CMysql, CDataSet>("sakila", true);
    public static CSqlMasterPool<CMysql, CDataSet>.CSlavePool Slave = new CSqlMasterPool<CMysql, CDataSet>.CSlavePool("sakila");

    private CSocketProService<CYourPeerOne> m_SSPeer = new CSocketProService<CYourPeerOne>();
}
