namespace Client
{
    partial class SumClient
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
            this.txtSum = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.btnDoSum = new System.Windows.Forms.Button();
            this.btnPause = new System.Windows.Forms.Button();
            this.btnRedoSum = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtSum
            // 
            this.txtSum.Location = new System.Drawing.Point(48, 31);
            this.txtSum.Name = "txtSum";
            this.txtSum.Size = new System.Drawing.Size(152, 20);
            this.txtSum.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(48, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(64, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Sum Result:";
            // 
            // btnDoSum
            // 
            this.btnDoSum.Enabled = false;
            this.btnDoSum.Location = new System.Drawing.Point(202, 29);
            this.btnDoSum.Name = "btnDoSum";
            this.btnDoSum.Size = new System.Drawing.Size(84, 22);
            this.btnDoSum.TabIndex = 2;
            this.btnDoSum.Text = "Do Sum";
            this.btnDoSum.UseVisualStyleBackColor = true;
            this.btnDoSum.Click += new System.EventHandler(this.btnDoSum_Click);
            // 
            // btnPause
            // 
            this.btnPause.Enabled = false;
            this.btnPause.Location = new System.Drawing.Point(113, 58);
            this.btnPause.Name = "btnPause";
            this.btnPause.Size = new System.Drawing.Size(83, 23);
            this.btnPause.TabIndex = 3;
            this.btnPause.Text = "Pause";
            this.btnPause.UseVisualStyleBackColor = true;
            this.btnPause.Click += new System.EventHandler(this.btnPause_Click);
            // 
            // btnRedoSum
            // 
            this.btnRedoSum.Enabled = false;
            this.btnRedoSum.Location = new System.Drawing.Point(202, 57);
            this.btnRedoSum.Name = "btnRedoSum";
            this.btnRedoSum.Size = new System.Drawing.Size(84, 23);
            this.btnRedoSum.TabIndex = 4;
            this.btnRedoSum.Text = "Redo Sum";
            this.btnRedoSum.UseVisualStyleBackColor = true;
            this.btnRedoSum.Click += new System.EventHandler(this.btnRedoSum_Click);
            // 
            // SumClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(386, 97);
            this.Controls.Add(this.btnRedoSum);
            this.Controls.Add(this.btnPause);
            this.Controls.Add(this.btnDoSum);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtSum);
            this.Name = "SumClient";
            this.Text = "Cancel Demo with Keeping Stateful Members";
            this.Load += new System.EventHandler(this.SumClient_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtSum;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnDoSum;
        private System.Windows.Forms.Button btnPause;
        private System.Windows.Forms.Button btnRedoSum;
    }
}

