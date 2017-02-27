namespace win_async
{
    partial class AsyncTest
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
            this.btnTest = new System.Windows.Forms.Button();
            this.txtRes = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // btnTest
            // 
            this.btnTest.Location = new System.Drawing.Point(34, 31);
            this.btnTest.Name = "btnTest";
            this.btnTest.Size = new System.Drawing.Size(84, 35);
            this.btnTest.TabIndex = 0;
            this.btnTest.Text = "Test";
            this.btnTest.UseVisualStyleBackColor = true;
            this.btnTest.Click += new System.EventHandler(this.btnTest_Click);
            // 
            // txtRes
            // 
            this.txtRes.Location = new System.Drawing.Point(138, 31);
            this.txtRes.Name = "txtRes";
            this.txtRes.Size = new System.Drawing.Size(254, 20);
            this.txtRes.TabIndex = 1;
            // 
            // async
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(428, 113);
            this.Controls.Add(this.txtRes);
            this.Controls.Add(this.btnTest);
            this.Name = "async";
            this.Text = "Async Wait Demo";
            this.Load += new System.EventHandler(this.async_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

            this.FormClosed += AsyncTest_FormClosed;

        }

        #endregion

        private System.Windows.Forms.Button btnTest;
        private System.Windows.Forms.TextBox txtRes;
    }
}

