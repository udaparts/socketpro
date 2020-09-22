using SocketProAdapter;

class Program
{
    static void Main(string[] args)
    {
        using (CScopeUQueue sb = new CScopeUQueue())
        {
            CMyStruct msOrig = CMyStruct.MakeOne();
            sb.Save(msOrig);
            CMyStruct ms = sb.Load<CMyStruct>();
            System.Diagnostics.Debug.Assert(sb.UQueue.GetSize() == 0);

            //check if both msOriginal and ms are equal in value.
        }
    }
}
