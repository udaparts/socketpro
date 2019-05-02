namespace MyRAdoCe
{
    partial class myado
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.MainMenu mainMenu1;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.mainMenu1 = new System.Windows.Forms.MainMenu();
            this.dgTable = new System.Windows.Forms.DataGrid();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.txtSQL = new System.Windows.Forms.TextBox();
            this.btnDO = new System.Windows.Forms.Button();
            this.chkSSL = new System.Windows.Forms.CheckBox();
            this.chkZip = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // dgTable
            // 
            this.dgTable.BackgroundColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(128)))), ((int)(((byte)(128)))));
            this.dgTable.Location = new System.Drawing.Point(4, 48);
            this.dgTable.Name = "dgTable";
            this.dgTable.Size = new System.Drawing.Size(240, 179);
            this.dgTable.TabIndex = 0;
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(165, 4);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(72, 20);
            this.btnConnect.TabIndex = 1;
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnClose
            // 
            this.btnClose.Location = new System.Drawing.Point(165, 27);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(72, 20);
            this.btnClose.TabIndex = 2;
            this.btnClose.Text = "Close";
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(4, 4);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(100, 21);
            this.txtHost.TabIndex = 3;
            this.txtHost.Text = "192.168.1.102";
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(107, 4);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(56, 21);
            this.txtPort.TabIndex = 4;
            this.txtPort.Text = "20901";
            // 
            // txtSQL
            // 
            this.txtSQL.Location = new System.Drawing.Point(4, 231);
            this.txtSQL.Name = "txtSQL";
            this.txtSQL.Size = new System.Drawing.Size(164, 21);
            this.txtSQL.TabIndex = 5;
            this.txtSQL.Text = "Select * from Customers";
            // 
            // btnDO
            // 
            this.btnDO.Enabled = false;
            this.btnDO.Location = new System.Drawing.Point(174, 232);
            this.btnDO.Name = "btnDO";
            this.btnDO.Size = new System.Drawing.Size(61, 20);
            this.btnDO.TabIndex = 6;
            this.btnDO.Text = "Execute";
            this.btnDO.Click += new System.EventHandler(this.btnDO_Click);
            // 
            // chkSSL
            // 
            this.chkSSL.Location = new System.Drawing.Point(4, 29);
            this.chkSSL.Name = "chkSSL";
            this.chkSSL.Size = new System.Drawing.Size(83, 17);
            this.chkSSL.TabIndex = 7;
            this.chkSSL.Text = "SSL/TLS";
            // 
            // chkZip
            // 
            this.chkZip.Location = new System.Drawing.Point(94, 30);
            this.chkZip.Name = "chkZip";
            this.chkZip.Size = new System.Drawing.Size(65, 15);
            this.chkZip.TabIndex = 8;
            this.chkZip.Text = "Zip";
            this.chkZip.CheckStateChanged += new System.EventHandler(this.chkZip_CheckStateChanged);
            // 
            // myado
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(240, 268);
            this.Controls.Add(this.chkZip);
            this.Controls.Add(this.chkSSL);
            this.Controls.Add(this.btnDO);
            this.Controls.Add(this.txtSQL);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.dgTable);
            this.Menu = this.mainMenu1;
            this.Name = "myado";
            this.Text = "Demo RADO on CE";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGrid dgTable;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.TextBox txtHost;
        private System.Windows.Forms.TextBox txtPort;
        private System.Windows.Forms.TextBox txtSQL;
        private System.Windows.Forms.Button btnDO;
        private System.Windows.Forms.CheckBox chkSSL;
        private System.Windows.Forms.CheckBox chkZip;
    }
}

