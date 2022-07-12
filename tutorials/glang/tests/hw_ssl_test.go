package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"runtime"
	"testing"
)

func Test_ssl(t *testing.T) {
	if runtime.GOOS != "windows" {
		gcs.SetVerifyLocation("/home/yye/socketpro/bin/ca.cert.pem")
	}
	sp := gcs.NewPool(sidHelloWorld)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe != gcs.SpeTimer {
			//socket pool events
			fmt.Println("PoolId:", sp.GetPoolId(), "ServiceId:", sp.GetSvsID(), spe)
		}
	}).SetSslAuth(func(h *gcs.CAsyncHandler) bool {
		ci := h.GetCert()
		ec, em := ci.Verify()
		fmt.Println(ec, em, ci.Validity)
		fmt.Println(ci.CertPem)
		fmt.Println(ci.Issuer)
		fmt.Println(ci.Subject)
		fmt.Println(ci.NotAfter, ci.NotBefore)
		fmt.Println(ci.SigAlg)
		fmt.Println(ci.SessionInfo)
		return ec == 0
	}).SetAutoConn(false)
	cc := cc_hw
	cc.EncryptionMethod = gspa.TLSv1
	ok := sp.StartSocketPool(cc, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("Test_ssl error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
	}
}
