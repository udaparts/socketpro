using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace WebForm
{
    public partial class TestIp : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {

        }

        protected async void btnLookup_Click(object sender, EventArgs e)
        {
            string res;
            if (iCOS.Lookup.Working)
            {
                iCOS.CRCode crc = iCOS.Lookup.ToCRCode(await iCOS.Lookup.GeoIp.DoLookup("113.123.212.145")); //127.0.0.1, 111.123.212.145, 112.123.212.145, 113.123.212.145
                res = "Country code = " + crc.CountryCode + ", region code = " + crc.RegionCode;
            }
            else
                res = "No connection to backend server";
            txtCountries.Text = res;
        }
    }
}