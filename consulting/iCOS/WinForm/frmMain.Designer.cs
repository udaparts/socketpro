namespace WinForm
{
    partial class frmMain
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
            this.btnDoLookup = new System.Windows.Forms.Button();
            this.txtCountries = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // btnDoLookup
            // 
            this.btnDoLookup.Location = new System.Drawing.Point(49, 56);
            this.btnDoLookup.Name = "btnDoLookup";
            this.btnDoLookup.Size = new System.Drawing.Size(148, 33);
            this.btnDoLookup.TabIndex = 0;
            this.btnDoLookup.Text = "Test Ip lookups";
            this.btnDoLookup.UseVisualStyleBackColor = true;
            this.btnDoLookup.Click += new System.EventHandler(this.btnDoLookup_Click);
            // 
            // txtCountries
            // 
            this.txtCountries.Location = new System.Drawing.Point(203, 68);
            this.txtCountries.Name = "txtCountries";
            this.txtCountries.Size = new System.Drawing.Size(456, 20);
            this.txtCountries.TabIndex = 1;
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(688, 205);
            this.Controls.Add(this.txtCountries);
            this.Controls.Add(this.btnDoLookup);
            this.Name = "frmMain";
            this.Text = "Prescision Design Solutions iCOS Application Win Client Demo";
            this.Load += new System.EventHandler(this.frmMain_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnDoLookup;
        private System.Windows.Forms.TextBox txtCountries;
    }
}

