package SPA.ClientSide;

public class ErrInfo {

    public ErrInfo(int res, String errMsg) {
        ec = res;
        em = errMsg;
    }
    public int ec = 0;
    public String em = "";

    @Override
    public String toString() {
        String s = "ec: " + String.valueOf(ec) + ", em: " + em;
        return s;
    }
};
