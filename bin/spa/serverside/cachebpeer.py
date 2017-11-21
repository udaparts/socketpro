
from abc import abstractmethod
from spa import CScopeUQueue, CUQueue, tagVariantDataType
from spa.serverside import CClientPeer
from spa.clientside import CAsyncDBHandler
from spa.serverside.scoreloader import SCoreLoader as scl
from ctypes import c_ubyte


class CCacheBasePeer(CClientPeer):

    @abstractmethod
    def GetCachedTables(self, defaultDb, flags, index):
        raise NotImplementedError("Please implement this method")

    def SendMeta(self, meta, index):
        q = CScopeUQueue.Lock()
        meta.SaveTo(q)
        ret = self.SendResult(q, CAsyncDBHandler.idRowsetHeader)
        CScopeUQueue.Unlock(q)
        return ret != CClientPeer.REQUEST_CANCELED and ret != CClientPeer.SOCKET_NOT_FOUND

    def SendRows(self, vData):
        if not vData:
            vData = []
        q = CScopeUQueue.Lock()
        for vt in vData:
            if isinstance(vt, str):
                if len(vt) <= CAsyncDBHandler.DEFAULT_BIG_FIELD_CHUNK_SIZE:
                    q.SaveObject(vt)
                else:
                    if q.Size > 0 and not self._SendRows_(q, True):
                        CScopeUQueue.Unlock(q)
                        return False
                    if not self._SendBlob_(vt):
                        CScopeUQueue.Unlock(q)
                        return False
            elif isinstance(vt, bytearray):
                if len(vt) <= 2*CAsyncDBHandler.DEFAULT_BIG_FIELD_CHUNK_SIZE:
                    q.SaveObject(vt)
                else:
                    if q.Size > 0 and not self._SendRows_(q, True):
                        CScopeUQueue.Unlock(q)
                        return False
                    if not self._SendBlob_(vt):
                        CScopeUQueue.Unlock(q)
                        return False
            else:
                q.SaveObject(vt)
        ret = self.SendResult(q, CAsyncDBHandler.idEndRows)
        return ret != CClientPeer.REQUEST_CANCELED and ret != CClientPeer.SOCKET_NOT_FOUND

    def _SendRows_(self, q, transferring):
        batching = self.BytesBatched >= CAsyncDBHandler.DEFAULT_RECORD_BATCH_SIZE
        if batching:
            self.CommitBatching()
        req_id = None
        if transferring:
            req_id = CAsyncDBHandler.idTransferring
        else:
            req_id = CAsyncDBHandler.idEndRows
        ret = self.SendResult(q, req_id)
        q.SetSize(0)
        if batching:
            self.StartBatching()
        return ret != CClientPeer.REQUEST_CANCELED and ret != CClientPeer.SOCKET_NOT_FOUND

    def _SendBlob_(self, blob):
        dt = tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI1
        if isinstance(blob, str):
            dt = tagVariantDataType.sdVT_BSTR
        q = CScopeUQueue.Lock()
        q.SaveObject(blob)
        q0 = CScopeUQueue.Lock()
        byte_len = q.Size + 4  #extra 4 bytes for string null termination
        q0.SaveUInt(byte_len).SaveUShort(dt)
        byte_len = q.Size - 6  #sizeof(ushort) + sizeof(uint)
        q.SaveUInt(byte_len)
        ret = self.SendResult(q0, CAsyncDBHandler.idStartBLOB)
        CScopeUQueue.Unlock(q0)
        if ret == CClientPeer.REQUEST_CANCELED or ret == CClientPeer.SOCKET_NOT_FOUND:
            CScopeUQueue.Unlock(q)
            return False
        q.Discard(6)
        while byte_len > CAsyncDBHandler.DEFAULT_BIG_FIELD_CHUNK_SIZE:
            buffer = (c_ubyte * CAsyncDBHandler.DEFAULT_BIG_FIELD_CHUNK_SIZE).from_buffer(q._m_bytes_, q._m_position_)
            ret = scl.SendReturnData(self.Handle, CAsyncDBHandler.idChunk, CAsyncDBHandler.DEFAULT_BIG_FIELD_CHUNK_SIZE, buffer)
            if ret == CClientPeer.REQUEST_CANCELED or ret == CClientPeer.SOCKET_NOT_FOUND:
                CScopeUQueue.Unlock(q)
                return False
            q.Discard(ret)
            byte_len -= ret
        ret = self.SendResult(q, CAsyncDBHandler.idEndBLOB)
        CScopeUQueue.Unlock(q)
        return ret != CClientPeer.REQUEST_CANCELED and ret != CClientPeer.SOCKET_NOT_FOUND
