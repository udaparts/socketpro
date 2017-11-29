namespace test_cache
{
    partial class frmCache
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
            this.lstTables = new System.Windows.Forms.ListBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtUser = new System.Windows.Forms.TextBox();
            this.Password = new System.Windows.Forms.Label();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.dgvShow = new System.Windows.Forms.DataGridView();
            this.label3 = new System.Windows.Forms.Label();
            this.txtMessage = new System.Windows.Forms.TextBox();
            this.btnDoSQL = new System.Windows.Forms.Button();
            this.txtSQL = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.dgvShow)).BeginInit();
            this.SuspendLayout();
            // 
            // lstTables
            // 
            this.lstTables.FormattingEnabled = true;
            this.lstTables.Location = new System.Drawing.Point(412, 8);
            this.lstTables.Name = "lstTables";
            this.lstTables.Size = new System.Drawing.Size(125, 82);
            this.lstTables.TabIndex = 0;
            this.lstTables.SelectedIndexChanged += new System.EventHandler(this.lstTables_SelectedIndexChanged);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(101, 32);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(96, 26);
            this.btnConnect.TabIndex = 1;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Enabled = false;
            this.btnDisconnect.Location = new System.Drawing.Point(217, 32);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(96, 26);
            this.btnDisconnect.TabIndex = 2;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(5, 11);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Remote host:";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(82, 4);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(76, 20);
            this.txtHost.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(162, 11);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(32, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "User:";
            // 
            // txtUser
            // 
            this.txtUser.Location = new System.Drawing.Point(203, 4);
            this.txtUser.Name = "txtUser";
            this.txtUser.Size = new System.Drawing.Size(58, 20);
            this.txtUser.TabIndex = 6;
            // 
            // Password
            // 
            this.Password.AutoSize = true;
            this.Password.Location = new System.Drawing.Point(269, 11);
            this.Password.Name = "Password";
            this.Password.Size = new System.Drawing.Size(56, 13);
            this.Password.TabIndex = 7;
            this.Password.Text = "Password:";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(332, 4);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(65, 20);
            this.txtPassword.TabIndex = 8;
            // 
            // dgvShow
            // 
            this.dgvShow.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvShow.Location = new System.Drawing.Point(5, 96);
            this.dgvShow.Name = "dgvShow";
            this.dgvShow.Size = new System.Drawing.Size(532, 356);
            this.dgvShow.TabIndex = 9;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 69);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(53, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "Message:";
            // 
            // txtMessage
            // 
            this.txtMessage.Location = new System.Drawing.Point(72, 67);
            this.txtMessage.Name = "txtMessage";
            this.txtMessage.ReadOnly = true;
            this.txtMessage.Size = new System.Drawing.Size(334, 20);
            this.txtMessage.TabIndex = 11;
            this.txtMessage.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // btnDoSQL
            // 
            this.btnDoSQL.Location = new System.Drawing.Point(8, 459);
            this.btnDoSQL.Name = "btnDoSQL";
            this.btnDoSQL.Size = new System.Drawing.Size(57, 23);
            this.btnDoSQL.TabIndex = 12;
            this.btnDoSQL.Text = "DoSQL";
            this.btnDoSQL.UseVisualStyleBackColor = true;
            this.btnDoSQL.Click += new System.EventHandler(this.btnDoSQL_Click);
            // 
            // txtSQL
            // 
            this.txtSQL.Location = new System.Drawing.Point(72, 461);
            this.txtSQL.Name = "txtSQL";
            this.txtSQL.Size = new System.Drawing.Size(465, 20);
            this.txtSQL.TabIndex = 13;
            // 
            // frmCache
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(553, 522);
            this.Controls.Add(this.txtSQL);
            this.Controls.Add(this.btnDoSQL);
            this.Controls.Add(this.txtMessage);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.dgvShow);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.Password);
            this.Controls.Add(this.txtUser);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.lstTables);
            this.Name = "frmCache";
            this.Text = "MySQL/Mariadb Cache Demonstration";
            this.Load += new System.EventHandler(this.frmCache_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgvShow)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox lstTables;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtHost;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtUser;
        private System.Windows.Forms.Label Password;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.DataGridView dgvShow;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtMessage;
        private System.Windows.Forms.Button btnDoSQL;
        private System.Windows.Forms.TextBox txtSQL;
    }
}

