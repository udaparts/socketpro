package uqueue_demo;

public class Program {

    public static void main(String[] args) {
        CMyStruct ms = CMyStruct.MakeOne();
        try (SPA.CScopeUQueue sb = new SPA.CScopeUQueue()) {
            sb.Save(ms);
            System.out.println("Before loading size: " + sb.getUQueue().GetSize());
            CMyStruct res = sb.Load(CMyStruct.class);
            System.out.println("After loading size: " + sb.getUQueue().GetSize());
            boolean equal = res.equals(ms);
            System.out.println("equal: " + equal);
        }
    }
}
