namespace iCOS
{
    static class IpLookupConst
    {
        //defines for service GeoIp
        public const uint sidGeoIp = (SocketProAdapter.BaseServiceID.sidReserved + 2345);
        public const ushort idLookupGeoIp = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
        public const ushort idLookupGeoIpEx = idLookupGeoIp + 1;
    }
}