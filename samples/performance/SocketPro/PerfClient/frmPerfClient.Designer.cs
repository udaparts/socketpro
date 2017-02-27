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
            this.label1 = new System.Windows.Forms.Label();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnEcho = new System.Windows.Forms.Button();
            this.txtTime = new System.Windows.Forms.TextBox();
            this.btnSQL = new System.Windows.Forms.Button();
            this.txtSQL = new System.Windows.Forms.TextBox();
            this.chkAsync = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(24, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(72, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Remote Host:";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(27, 25);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(166, 20);
            this.txtHost.TabIndex = 1;
            this.txtHost.Text = "localhost";
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(210, 24);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(100, 20);
            this.txtPort.TabIndex = 2;
            this.txtPort.Text = "21911";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(210, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(36, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Port#:";
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(457, 25);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(75, 23);
            this.btnConnect.TabIndex = 4;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(539, 24);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(75, 23);
            this.btnDisconnect.TabIndex = 5;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnEcho
            // 
            this.btnEcho.Enabled = false;
            this.btnEcho.Location = new System.Drawing.Point(27, 61);
            this.btnEcho.Name = "btnEcho";
            this.btnEcho.Size = new System.Drawing.Size(166, 23);
            this.btnEcho.TabIndex = 9;
            this.btnEcho.Text = "Test Echo 10000 times";
            this.btnEcho.UseVisualStyleBackColor = true;
            this.btnEcho.Click += new System.EventHandler(this.btnEcho_Click);
            // 
            // txtTime
            // 
            this.txtTime.Location = new System.Drawing.Point(457, 67);
            this.txtTime.Name = "txtTime";
            this.txtTime.ReadOnly = true;
            this.txtTime.Size = new System.Drawing.Size(125, 20);
            this.txtTime.TabIndex = 10;
            // 
            // btnSQL
            // 
            this.btnSQL.Enabled = false;
            this.btnSQL.Location = new System.Drawing.Point(27, 118);
            this.btnSQL.Name = "btnSQL";
            this.btnSQL.Size = new System.Drawing.Size(166, 23);
            this.btnSQL.TabIndex = 11;
            this.btnSQL.Text = "Execute SQL 100 times";
            this.btnSQL.UseVisualStyleBackColor = true;
            this.btnSQL.Click += new System.EventHandler(this.btnSQL_Click);
            // 
            // txtSQL
            // 
            this.txtSQL.Location = new System.Drawing.Point(27, 92);
            this.txtSQL.Name = "txtSQL";
            this.txtSQL.Size = new System.Drawing.Size(587, 20);
            this.txtSQL.TabIndex = 12;
            this.txtSQL.Text = "Select * from Production.Culture";
            // 
            // chkAsync
            // 
            this.chkAsync.AutoSize = true;
            this.chkAsync.Checked = true;
            this.chkAsync.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkAsync.Location = new System.Drawing.Point(210, 69);
            this.chkAsync.Name = "chkAsync";
            this.chkAsync.Size = new System.Drawing.Size(64, 17);
            this.chkAsync.TabIndex = 13;
            this.chkAsync.Text = "Async ?";
            this.chkAsync.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(396, 70);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 14;
            this.label3.Text = "Time (ms):";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // frmPerfClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(635, 156);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.chkAsync);
            this.Controls.Add(this.txtSQL);
            this.Controls.Add(this.btnSQL);
            this.Controls.Add(this.txtTime);
            this.Controls.Add(this.btnEcho);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label1);
            this.Name = "frmPerfClient";
            this.Text = "SocketPro Performance Investigation";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtHost;
        private System.Windows.Forms.TextBox txtPort;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Button btnEcho;
        private System.Windows.Forms.TextBox txtTime;
        private System.Windows.Forms.Button btnSQL;
        private System.Windows.Forms.TextBox txtSQL;
        private System.Windows.Forms.CheckBox chkAsync;
        private System.Windows.Forms.Label label3;
    }
}

