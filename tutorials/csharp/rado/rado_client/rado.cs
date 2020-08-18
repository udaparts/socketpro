using SocketProAdapter.ClientSide;
using System.Data;

public class RAdo : CAsyncAdohandler
{
    public RAdo()
        : base(radoConst.sidRAdo)
    {
    }

    public DataSet CurrentDataSet {
        get {
            return AdoSerializer.CurrentDataSet;
        }
    }

    public DataTable CurrentDataTable {
        get {
            return AdoSerializer.CurrentDataTable;
        }
    }

    public DataSet GetDataSet(string sql0, string sql1)
    {
        Async(radoConst.idGetDataSetRAdo, sql0, sql1).Wait();
        return AdoSerializer.CurrentDataSet;
    }

    public DataTable GetDataTable(string sql)
    {
        Async(radoConst.idGetDataTableRAdo, sql).Wait();
        return AdoSerializer.CurrentDataTable;
    }
}
