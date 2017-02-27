using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Data;

public class DBPushPeer : CAdoClientPeer
{
    public DBPushPeer()
    {
        OnAdonetLoaded += OnAdoNet;
    }

    [RequestAttr(CAdoSerializationHelper.idDmlTriggerMessage, true)]
    private void OnDmlTriggerMessage(int triggerEvent, string fullDbObjectName, object param)
    {
        Console.WriteLine("FullDbTableName = {0}, event = {1}, param = {2}", fullDbObjectName, triggerEvent, param);
    }

    [RequestAttr(CAdoSerializationHelper.idRecordsetName, true)]
    private void OnQueryMessage(string recordsetName)
    {
        Console.WriteLine("Recordset = " + recordsetName);
    }

    [RequestAttr(CAdoSerializationHelper.idDbEventTriggerMessage, true)]
    private void OnDbTriggerMessage(int triggerEvent, string instance, string eventData)
    {
        Console.WriteLine("DB Logon event = " + instance + ", eventData = " + eventData);
    }

    private void OnAdoNet(CAdoClientPeer peer, ushort reqId)
    {
        switch (reqId)
        {
            case CAdoSerializationHelper.idDataReaderRecordsArrive:
                if (AdoSerializer.CurrentDataTable.Rows.Count > 10 * 1024)
                {
                    AdoSerializer.CurrentDataTable.Clear();
                }
                break;
            default:
                break;
        }

        if (reqId == CAdoSerializationHelper.idEndDataReader)
        {
            if (AdoSerializer.CurrentDataTable.Rows.Count > 100)
            {
                Console.WriteLine("Table rowset size = " + AdoSerializer.CurrentDataTable.Rows.Count);
            }
            else
            {
                foreach (DataRow dr in AdoSerializer.CurrentDataTable.Rows)
                {
                    int n = 0;
                    foreach (object obj in dr.ItemArray)
                    {
                        if (n > 0)
                            Console.Write(",\t");
                        Console.Write(obj.ToString());
                        ++n;
                    }
                    Console.WriteLine();
                }
            }
        }
    }
}