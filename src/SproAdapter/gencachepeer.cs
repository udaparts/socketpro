﻿
using System;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public abstract class CCacheBasePeer : CClientPeer
        {
            [RequestAttr(UDB.DB_CONSTS.idGetCachedTables, true)]
            protected abstract string GetCachedTables(string defaultDb, uint flags, ulong index, out int dbManagementSystem, out int res);

            public bool SendMeta(UDB.CDBColumnInfoArray meta, ulong index)
            {
                //A client expects a rowset meta data and call index
                uint ret = SendResult(UDB.DB_CONSTS.idRowsetHeader, meta, index);
                return (ret != REQUEST_CANCELED && ret != SOCKET_NOT_FOUND);
            }

            public bool SendRows(UDB.CDBVariantArray vData)
            {
                uint len;
                if (vData == null)
                    vData = new UDB.CDBVariantArray();
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    foreach (object vt in vData)
                    {
                        if (vt is string)
                        {
                            string s = (string)vt;
                            if (s.Length > UDB.DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                            {
                                if (sb.UQueue.GetSize() > 0 && !SendRows(sb, true))
                                    return false;
                                sb.UQueue.Save(vt);
                                //6 == sizeof(ushort) + sizeof(uint)
                                if (!SendBlob((ushort)tagVariantDataType.sdVT_BSTR, sb.UQueue.IntenalBuffer, sb.UQueue.GetSize() - 6, 6))
                                    return false;
                                sb.UQueue.SetSize(0);
                            }
                            else
                            {
                                sb.UQueue.Save(vt);
                            }
                        }
                        else if (vt is byte[])
                        {
                            byte[] bytes = (byte[])vt;
                            if (bytes.LongLength > 2 * UDB.DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                            {
                                if (sb.UQueue.GetSize() > 0 && !SendRows(sb, true))
                                    return false;
                                if (!SendBlob((ushort)(tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY), bytes, (uint)bytes.LongLength, 0))
                                    return false;
                            }
                            else
                            {
                                sb.UQueue.Save(vt);
                            }
                        }
                        else if (vt is sbyte)
                        {
                            sbyte[] bytes = (sbyte[])vt;
                            if (bytes.LongLength > 2 * UDB.DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                            {
                                if (sb.UQueue.GetSize() > 0 && !SendRows(sb, true))
                                    return false;
                                sb.UQueue.Save(vt);
                                //6 == sizeof(ushort) + sizeof(uint)
                                if (!SendBlob((ushort)(tagVariantDataType.sdVT_I1 | tagVariantDataType.sdVT_ARRAY), sb.UQueue.IntenalBuffer, sb.UQueue.GetSize() - 6, 6))
                                    return false;
                                sb.UQueue.SetSize(0);
                            }
                            else
                            {
                                sb.UQueue.Save(vt);
                            }
                        }
                        else
                        {
                            sb.UQueue.Save(vt);
                        }
                    }
                    len = SendResult(UDB.DB_CONSTS.idEndRows, sb);
                    return (len != REQUEST_CANCELED && len != SOCKET_NOT_FOUND);
                }
            }

            protected bool SendBlob(ushort data_type, byte[] buffer, uint bytes, uint offset)
            {
                uint ret = SendResult(UDB.DB_CONSTS.idStartBLOB,
                    //extra 4 bytes for string null termination
                        (uint)(bytes + sizeof(ushort) + sizeof(uint) + sizeof(uint)),
                        data_type, bytes);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND)
                {
                    return false;
                }
                while (bytes > UDB.DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
                {
                    ret = SendResult(UDB.DB_CONSTS.idChunk, buffer, UDB.DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE, offset);
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND)
                    {
                        return false;
                    }
                    offset += ret;
                    bytes -= ret;
                }
                ret = SendResult(UDB.DB_CONSTS.idEndBLOB, buffer, bytes, offset);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND)
                {
                    return false;
                }
                return true;
            }

            protected bool SendRows(CScopeUQueue sb, bool transferring)
            {
                bool batching = (BytesBatched >= UDB.DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE);
                if (batching)
                {
                    CommitBatching();
                }
                uint ret = SendResult(transferring ? UDB.DB_CONSTS.idTransferring : UDB.DB_CONSTS.idEndRows, sb.UQueue.IntenalBuffer, sb.UQueue.GetSize());
                sb.UQueue.SetSize(0);
                if (batching)
                {
                    StartBatching();
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND)
                {
                    return false;
                }
                return true;
            }
        }
    }
}
