namespace PerfClient
{
    partial class frmPerfClient
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

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
            this.txtNetPort = new System.Windows.Forms.TextBox();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnMyEcho = new System.Windows.Forms.Button();
            this.btnSQL = new System.Windows.Forms.Button();
            this.txtSQL = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtTime = new System.Windows.Forms.TextBox();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // txtNetPort
            // 
            this.txtNetPort.Location = new System.Drawing.Point(230, 39);
            this.txtNetPort.Name = "txtNetPort";
            this.txtNetPort.Size = new System.Drawing.Size(76, 20);
            this.txtNetPort.TabIndex = 12;
            this.txtNetPort.Text = "21910";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(14, 39);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(207, 20);
            this.txtHost.TabIndex = 11;
            this.txtHost.Text = "localhost";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(230, 12);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(128, 21);
            this.label2.TabIndex = 10;
            this.label2.Text = "dotNet Remoting Port:";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(14, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(132, 20);
            this.label1.TabIndex = 9;
            this.label1.Text = "Host Address:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnMyEcho);
            this.groupBox1.Controls.Add(this.btnSQL);
            this.groupBox1.Controls.Add(this.txtSQL);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.txtTime);
            this.groupBox1.Location = new System.Drawing.Point(13, 69);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(646, 105);
            this.groupBox1.TabIndex = 13;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "dotNet Remoting";
            // 
            // btnMyEcho
            // 
            this.btnMyEcho.Enabled = false;
            this.btnMyEcho.Location = new System.Drawing.Point(8, 67);
            this.btnMyEcho.Name = "btnMyEcho";
            this.btnMyEcho.Size = new System.Drawing.Size(516, 24);
            this.btnMyEcho.TabIndex = 6;
            this.btnMyEcho.Text = "Execute 10000 requests";
            this.btnMyEcho.Click += new System.EventHandler(this.btnMyEcho_Click);
            // 
            // btnSQL
            // 
            this.btnSQL.Enabled = false;
            this.btnSQL.Location = new System.Drawing.Point(468, 30);
            this.btnSQL.Name = "btnSQL";
            this.btnSQL.Size = new System.Drawing.Size(154, 20);
            this.btnSQL.TabIndex = 5;
            this.btnSQL.Text = "Execute SQL 100 Times";
            this.btnSQL.Click += new System.EventHandler(this.btnSQL_Click);
            // 
            // txtSQL
            // 
            this.txtSQL.Location = new System.Drawing.Point(8, 28);
            this.txtSQL.Name = "txtSQL";
            this.txtSQL.Size = new System.Drawing.Size(454, 20);
            this.txtSQL.TabIndex = 4;
            this.txtSQL.Text = "Select * from Shippers";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(526, 54);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(78, 12);
            this.label5.TabIndex = 2;
            this.label5.Text = "Time (us):";
            // 
            // txtTime
            // 
            this.txtTime.Location = new System.Drawing.Point(526, 70);
            this.txtTime.Name = "txtTime";
            this.txtTime.Size = new System.Drawing.Size(96, 20);
            this.txtTime.TabIndex = 0;
            this.txtTime.Text = "0";
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(559, 35);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(100, 24);
            this.btnDisconnect.TabIndex = 15;
            this.btnDisconnect.Text = "Disconnect";
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(559, 7);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(100, 24);
            this.btnConnect.TabIndex = 14;
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // frmPerfClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(682, 192);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.txtNetPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "frmPerfClient";
            this.Text = "DotNet Remoting Performance Investigation";
            this.Load += new System.EventHandler(this.frmPerfClient_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtNetPort;
        private System.Windows.Forms.TextBox txtHost;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnMyEcho;
        private System.Windows.Forms.Button btnSQL;
        private System.Windows.Forms.TextBox txtSQL;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtTime;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Button btnConnect;
    }
}

