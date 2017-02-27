
package uqueue_demo;

public class Program {
    public static void main(String[] args) {
        CMyStruct ms = CMyStruct.MakeOne();
        SPA.CUQueue q = new SPA.CUQueue();
        ms.SaveTo(q);
        CMyStruct res = q.Load(CMyStruct.class);
        boolean equal = res.equals(ms);
        System.out.println("equal = " + equal);
    }
}
