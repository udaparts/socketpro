namespace ParameterizedSQL
{
    partial class frmPSQL
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
            this.btnTestPSQL = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.dgvRowset = new System.Windows.Forms.DataGridView();
            ((System.ComponentModel.ISupportInitialize)(this.dgvRowset)).BeginInit();
            this.SuspendLayout();
            // 
            // btnTestPSQL
            // 
            this.btnTestPSQL.Enabled = false;
            this.btnTestPSQL.Location = new System.Drawing.Point(259, 12);
            this.btnTestPSQL.Name = "btnTestPSQL";
            this.btnTestPSQL.Size = new System.Drawing.Size(240, 33);
            this.btnTestPSQL.TabIndex = 0;
            this.btnTestPSQL.Text = "Test PSQL";
            this.btnTestPSQL.UseVisualStyleBackColor = true;
            this.btnTestPSQL.Click += new System.EventHandler(this.btnTestPSQL_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(12, 12);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(111, 31);
            this.btnConnect.TabIndex = 1;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(129, 12);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(106, 31);
            this.btnDisconnect.TabIndex = 2;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // dgvRowset
            // 
            this.dgvRowset.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvRowset.Location = new System.Drawing.Point(12, 51);
            this.dgvRowset.Name = "dgvRowset";
            this.dgvRowset.Size = new System.Drawing.Size(487, 256);
            this.dgvRowset.TabIndex = 3;
            // 
            // frmPSQL
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(511, 319);
            this.Controls.Add(this.dgvRowset);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.btnTestPSQL);
            this.Name = "frmPSQL";
            this.Text = "Demo to PSQL";
            this.Load += new System.EventHandler(this.frmPSQL_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgvRowset)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnTestPSQL;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.DataGridView dgvRowset;
    }
}

