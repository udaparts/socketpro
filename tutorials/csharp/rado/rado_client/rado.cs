using SocketProAdapter.ClientSide;
using System.Data;

public class RAdo : CAsyncAdohandler
{
    public RAdo()
        : base(radoConst.sidRAdo)
    {
    }

    public DataSet CurrentDataSet
    {
        get
        {
            return AdoSerializer.CurrentDataSet;
        }
    }

    public DataTable CurrentDataTable
    {
        get
        {
            return AdoSerializer.CurrentDataTable;
        }
    }

    public DataSet GetDataSet(string sql0, string sql1)
    {
        if (ProcessR0(radoConst.idGetDataSetRAdo, sql0, sql1))
            return AdoSerializer.CurrentDataSet;
        return new DataSet();
    }

    public DataTable GetDataTable(string sql)
    {
        if (ProcessR0(radoConst.idGetDataTableRAdo, sql))
            return AdoSerializer.CurrentDataTable;
        return new DataTable();
    }
}
