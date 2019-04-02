package SPA.ClientSide;

public class CConnectionContext {

    public CConnectionContext() {

    }

    public CConnectionContext(String host, int port, String userId, String password) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em, boolean zip) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
        Zip = zip;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em, boolean zip, boolean v6) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
        Zip = zip;
        V6 = v6;
    }

    void Normalize() throws Exception {
        if (Host == null) {
            throw new Exception("Host string cannot be null");
        }
        Host = Host.trim();
        if (Host.length() == 0) {
            throw new Exception("Host string cannot be empty");
        }
        if (Port <= 0) {
            throw new Exception("Port number must be a positive number");
        }
    }

    boolean IsSame(CConnectionContext cc) {
        if (cc == null) {
            return false;
        }
        return (Host.equalsIgnoreCase(cc.Host) && Port == cc.Port);
    }

    public String Host;
    public int Port;
    public String UserId;
    public String Password;
    public SPA.tagEncryptionMethod EncrytionMethod = SPA.tagEncryptionMethod.NoEncryption;
    public boolean V6 = false;
    public boolean Zip = false;
    public Object AnyData = null;
}
