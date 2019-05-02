using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Net;

namespace iCOS
{
    /// <summary>
    /// The sealed class supports the following features:
    /// a. process ip lookups in batch
    /// b. one single instance Lookup connected a number of SocketPro servers
    /// c. ip lookups are completed asynchronously
    /// d. auto failure recovery to anyone error of network, socketpro server application or server down
    /// e. auto reconnection to expected socketpro server applications
    /// f. ssl/tlsv1.1 is supported through openssl
    /// g. thread-safe for all of methods and properties
    /// h. both ip4 and ipv6 supported
    /// i. better extensibility and maintenance by use of method version
    /// </summary>
    public sealed class Lookup
    {
        private const ushort LOOKUP_VERSION = 0;
        private uint m_timeout;
        private uint m_timer;
        private uint m_batch;
        private System.Threading.Thread m_thread = null;
        private static Lookup m_lookup = null;
        private CSocketPool<CGeoIpAsyncHandler> m_spGi = null;
        private System.Threading.ManualResetEvent m_mre = new System.Threading.ManualResetEvent(false);

        private static object m_csLookup = new object(); //used to protected m_lookup

        private object m_cs = new object(); //used to protected m_id and m_dicIdTask
        private ulong m_id = 0;
        private Dictionary<ulong, MyStruct> m_dicIdTask;

        private Lookup()
        {
            m_thread = new System.Threading.Thread(new ThreadStart(WorkerThreadFunc));
        }

        public static CRCode ToCRCode(uint codes)
        {
            byte[] crCodes = BitConverter.GetBytes(codes);
            char[] countryCode = { (char)crCodes[0], (char)crCodes[1] };
            char[] regionCode = { (char)crCodes[2], (char)crCodes[3] };
            CRCode cr;
            cr.CountryCode = new string(countryCode);
            cr.RegionCode = new string(regionCode);
            return cr;
        }

        private void SendLookups()
        {
            using (CScopeUQueue su = new CScopeUQueue())
            {
                uint m = 0;
                su.UQueue.Save(LOOKUP_VERSION);
                foreach (ulong id in m_dicIdTask.Keys)
                {
                    if (m >= m_batch)
                        break;
                    //must match the deserialization of GEOIP server of id and ip address
                    su.UQueue.Save(id).Save(m_dicIdTask[id].Ip);
                    ++m;
                }
                CGeoIpAsyncHandler gi = m_spGi.Seek();
                if (gi == null)
                    return;

                //this is called from a non-socketpool thread
                lock (gi.m_dicIdTaskProcessing)
                {
                    if (gi.SendRequest(IpLookupConst.idLookupGeoIp, su.UQueue, (ar) =>
                    {
                        //this callback is called from one of socketpool threads
                        lock (gi.m_dicIdTaskProcessing)
                        {
                            while (ar.UQueue.GetSize() > 0)
                            {
                                ulong index;
                                uint code; //mapping to country and region codes -- 4 bytes

                                //must match the serialization of GEOIP server of id and code
                                ar.UQueue.Load(out index).Load(out code);

                                MyStruct ms = gi.m_dicIdTaskProcessing[index];
                                gi.m_dicIdTaskProcessing.Remove(index);

                                //assuming all ips are looked up successfully
                                //System.Diagnostics.Debug.Assert(found);

                                ms.Tcs.SetResult(code);
                            }
                        }
                    }))
                    {
                        ulong[] ids = new ulong[m];
                        foreach (ulong id in m_dicIdTask.Keys)
                        {
                            if (m == 0)
                                break;
                            gi.m_dicIdTaskProcessing.Add(id, m_dicIdTask[id]);
                            --m;
                            ids[m] = id;
                        }
                        if (ids.Length >= m_dicIdTask.Count)
                            m_dicIdTask.Clear();
                        else
                        {
                            foreach (ulong id in ids)
                            {
                                m_dicIdTask.Remove(id);
                            }
                        }
                    }
                    else //failed
                    {
                        //it is ok to leave them until the next call or timer
                    }
                }
            }
        }

        private void WorkerThreadFunc()
        {
            bool stop = m_mre.WaitOne((int)m_timer);
            while (!stop)
            {
                lock (m_cs)
                {
                    if (m_dicIdTask.Count > 0)
                    {
                        SendLookups();
                    }
                }
                stop = m_mre.WaitOne((int)m_timer);
            }
        }

        /// <summary>
        /// Initialize single instance of GEOIP. You must first call the method before calling any other methods or properties
        /// </summary>
        /// <param name="ccs">A two-dimension array. The length of its first dimension represents the number of thread. The length of the second dimension is the number of sockets per thread.</param>
        /// <param name="batch">The size of tasks in one batch processing</param>
        /// <param name="timer">The value for timer interval in milliseconds</param>
        /// <param name="SocketPoolEvent">A callback to track the behind socket pool events</param>
        /// <param name="SslAuthentication">A callback to do ssl server authentication by checking its certificate</param>
        /// <param name="timeout">A value for task processing timeout in milliseconds</param>
        /// <returns>True if there is at least one socket connection to remote SocketPro server; false if there is no socket connection.</returns>
        /// <remarks>You can call the method before you start anyone of expected SocketPro servers. When SocketPro servers are available, socket connections will be automatically established.</remarks>
        public static bool Initialize(CConnectionContext[,] ccs, uint batch = 1024, uint timer = 1, CSocketPool<CGeoIpAsyncHandler>.DOnSocketPoolEvent SocketPoolEvent = null, CSocketPool<CGeoIpAsyncHandler>.DDoSslServerAuthentication SslAuthentication = null, uint timeout = 30000)
        {
            bool ok = true;
            if (timer == 0)
                timer = 100;
            if (batch == 0)
                batch = 1024;
            if (timeout == 0)
                timeout = 30000;
            lock (m_csLookup)
            {
                if (m_lookup != null)
                    return ok;

                m_lookup = new Lookup();
                m_lookup.m_dicIdTask = new Dictionary<ulong, MyStruct>((int)batch);
                m_lookup.m_spGi = new CSocketPool<CGeoIpAsyncHandler>(true, timeout);

                m_lookup.m_spGi.SocketPoolEvent += (sender, spe, handler) =>
                {
                    switch (spe)
                    {
                        case tagSocketPoolEvent.speSocketClosed:
                            lock (m_lookup.m_cs)
                            {
                                lock (handler.m_dicIdTaskProcessing)
                                {
                                    //put back all processing tasks back into m_lookup.m_dicIdTask to be processed when socket is closed
                                    foreach (ulong id in handler.m_dicIdTaskProcessing.Keys)
                                    {
                                        m_lookup.m_dicIdTask[id] = handler.m_dicIdTaskProcessing[id];
                                    }
                                    handler.m_dicIdTaskProcessing.Clear();
                                }
                            }
                            break;
                        default:
                            break;
                    }
                };

                if (SocketPoolEvent != null)
                    m_lookup.m_spGi.SocketPoolEvent += SocketPoolEvent;
                if (SslAuthentication != null)
                    m_lookup.m_spGi.DoSslServerAuthentication += SslAuthentication;
                ok = m_lookup.m_spGi.StartSocketPool(ccs);
                m_lookup.m_timer = timer;
                m_lookup.m_batch = batch;
                m_lookup.m_timeout = timeout;
                m_lookup.m_thread.Start();
            }
            return ok;
        }

        public static bool Working
        {
            get
            {
                lock (m_csLookup)
                {
                    if (m_lookup != null)
                    {
                        return (m_lookup.m_spGi.ConnectedSockets > 0);
                    }
                    return false;
                }
            }
        }

        public static void Shutdown()
        {
            lock (m_csLookup)
            {
                if (m_lookup != null)
                {
                    m_lookup.m_spGi.Dispose();
                    m_lookup.m_mre.Set();
                    m_lookup.m_thread.Join();
                    m_lookup.m_thread = null;
                    m_lookup = null;
                }
            }
        }

        public static Lookup GeoIp
        {
            get
            {
                lock (m_csLookup)
                {
                    return m_lookup;
                }
            }
        }

        public static uint QueuedIps
        {
            get
            {
                lock (m_csLookup)
                {
                    if (m_lookup != null)
                    {
                        lock (m_lookup.m_cs)
                        {
                            return (uint)m_lookup.m_dicIdTask.Count;
                        }
                    }
                    return 0;
                }
            }
        }

        public Task<uint> DoLookup(string ip)
        {
            MyStruct ms;
            ms.Tcs = new TaskCompletionSource<uint>();

            //do we convert ip address in little-endian?

            //usually convert ip address into uint in big-endian as implemented in the below
            IPAddress ipAddress = IPAddress.Parse(ip);
            byte[] ipBytes = ipAddress.GetAddressBytes();
            uint ipint = (uint)ipBytes[3] << 24;
            ipint += (uint)ipBytes[2] << 16;
            ipint += (uint)ipBytes[1] << 8;
            ipint += (uint)ipBytes[0];

            ms.Ip = ipint;
            lock (m_cs)
            {
                //always increase the m_id
                ++m_id;
                m_dicIdTask.Add(m_id, ms);
                if (m_dicIdTask.Count >= (int)m_batch)
                    SendLookups();
            }
            return ms.Tcs.Task;
        }

        /// <summary>
        /// A property to the underlying socketpool object.
        /// </summary>
        /// <remarks>You can use the property to diagnose socket connection situations, but you should never shutdown the pool directly.</remarks>
        public CSocketPool<CGeoIpAsyncHandler> SocketPool
        {
            get
            {
                return m_spGi;
            }
        }
    }
}
