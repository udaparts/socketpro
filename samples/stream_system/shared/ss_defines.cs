
using System;
using SocketProAdapter;
using System.Collections.Generic;

namespace ss
{
    public static class Consts
    {
        public const uint sidStreamSystem = (SocketProAdapter.BaseServiceID.sidReserved + 1210);

        public const ushort idQueryMaxMinAvgs = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
        public const ushort idGetMasterSlaveConnectedSessions = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 2);
        public const ushort idUploadEmployees = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 3);
        public const ushort idGetRentalDateTimes = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 4);
    }

    public class CInt64Array : List<long>, IUSerializer
    {
        public void LoadFrom(CUQueue UQueue)
        {
            int size;
            Clear();
            UQueue.Load(out size);
            while (size > 0)
            {
                long n;
                UQueue.Load(out n);
                Add(n);
                --size;
            }
        }

        public void SaveTo(CUQueue UQueue)
        {
            UQueue.Save(Count);
            foreach (long n in this)
            {
                UQueue.Save(n);
            }
        }
    }

    public class CMaxMinAvg : IUSerializer
    {
        public double Max = 0;
        public double Min = 0;
        public double Avg = 0;

        public void LoadFrom(CUQueue UQueue)
        {
            UQueue.Load(out Max).Load(out Min).Load(out Avg);
        }

        public void SaveTo(CUQueue UQueue)
        {
            UQueue.Save(Max).Save(Min).Save(Avg);
        }
    }

    class CRentalDateTimes : IUSerializer
    {
        public long rental_id = 0;
        public CRentalDateTimes()
        {
        }
        public CRentalDateTimes(long rentaLId)
        {
            rental_id = rentaLId;
        }
        public DateTime Rental;
        public DateTime Return;
        public DateTime LastUpdate;

        private const ulong ZERO_TICK = 0;
        public void LoadFrom(CUQueue UQueue)
        {
            UQueue.Load(out rental_id).Load(out Rental).Load(out Return).Load(out LastUpdate);
        }

        public void SaveTo(CUQueue UQueue)
        {
            UQueue.Save(rental_id);
            if (Rental == null || Rental.Ticks == 0)
                UQueue.Save(ZERO_TICK);
            else
                UQueue.Save(Rental);
            if (Return == null || Return.Ticks == 0)
                UQueue.Save(ZERO_TICK);
            else
                UQueue.Save(Return);
            if (LastUpdate == null || LastUpdate.Ticks == 0)
                UQueue.Save(ZERO_TICK);
            else
                UQueue.Save(LastUpdate);
        }
    };
}
