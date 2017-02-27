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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnMyEcho = new System.Windows.Forms.Button();
            this.btnSQL = new System.Windows.Forms.Button();
            this.txtSQL = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtTime = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnMyEcho);
            this.groupBox1.Controls.Add(this.btnSQL);
            this.groupBox1.Controls.Add(this.txtSQL);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.txtTime);
            this.groupBox1.Location = new System.Drawing.Point(22, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(646, 110);
            this.groupBox1.TabIndex = 14;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "WCF";
            // 
            // btnMyEcho
            // 
            this.btnMyEcho.Location = new System.Drawing.Point(8, 67);
            this.btnMyEcho.Name = "btnMyEcho";
            this.btnMyEcho.Size = new System.Drawing.Size(516, 24);
            this.btnMyEcho.TabIndex = 6;
            this.btnMyEcho.Text = "Execute 10000 requests";
            this.btnMyEcho.Click += new System.EventHandler(this.btnMyEcho_Click);
            // 
            // btnSQL
            // 
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
            this.txtSQL.Text = "Select * from Production.Culture";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(526, 54);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(78, 12);
            this.label5.TabIndex = 2;
            this.label5.Text = "Time (ms):";
            // 
            // txtTime
            // 
            this.txtTime.Location = new System.Drawing.Point(526, 70);
            this.txtTime.Name = "txtTime";
            this.txtTime.Size = new System.Drawing.Size(96, 20);
            this.txtTime.TabIndex = 0;
            this.txtTime.Text = "0";
            // 
            // frmPerfClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(692, 144);
            this.Controls.Add(this.groupBox1);
            this.Name = "frmPerfClient";
            this.Text = "WCF Performance Investigation";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnMyEcho;
        private System.Windows.Forms.Button btnSQL;
        private System.Windows.Forms.TextBox txtSQL;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtTime;
    }
}

