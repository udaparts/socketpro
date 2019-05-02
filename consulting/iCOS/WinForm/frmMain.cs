using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WinForm
{
    public partial class frmMain : Form
    {
        public frmMain()
        {
            InitializeComponent();
        }

        private void frmMain_Load(object sender, EventArgs e)
        {

        }

        private async void btnDoLookup_Click(object sender, EventArgs e)
        {
            string res;
            if (iCOS.Lookup.Working)
            {
                iCOS.CRCode crc = iCOS.Lookup.ToCRCode(await iCOS.Lookup.GeoIp.DoLookup("113.123.212.145")); //127.0.0.1, 111.123.212.145, 112.123.212.145, 113.123.212.145
                res = "Country code = " + crc.CountryCode + ", region code = " + crc.RegionCode;
            }
            else
                res = "No connection to backend server";
            MessageBox.Show(res);
        }
    }
}
