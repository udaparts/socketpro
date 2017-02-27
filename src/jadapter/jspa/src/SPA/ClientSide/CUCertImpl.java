package SPA.ClientSide;

class CUCertImpl extends IUcert {

    final CClientSocket m_cs;

    CUCertImpl(CClientSocket cs) {
        m_cs = cs;
        CertInfo ci = (CertInfo) ClientCoreLoader.GetUCert(cs.getHandle());
        Algorithm = ci.Algorithm;
        CertPem = ci.CertPem;
        Issuer = ci.Issuer;
        NotAfter = ci.NotAfter;
        NotBefore = ci.NotBefore;
        PublicKey = ci.PublicKey;
        SerialNumber = ci.SerialNumber;
        SessionInfo = ci.SessionInfo;
        SigAlg = ci.SigAlg;
        Subject = ci.Subject;
        Validity = ci.Validity;
    }

    @Override
    public String Verify(SPA.RefObject<Integer> errCode) {
        int[] ec = {0};
        String res = ClientCoreLoader.Verify(m_cs.getHandle(), ec, 1);
        if (errCode != null) {
            errCode.Value = ec[0];
        }
        return res;
    }
}
