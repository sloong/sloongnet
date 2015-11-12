namespace servctrl
{
    partial class FormStatus
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
            this.components = new System.ComponentModel.Container();
            this.dataGridViewStatus = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn7 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn8 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataTableStatusBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.dataSetStatus = new servctrl.DataSetStatus();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewStatus)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataTableStatusBindingSource)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataSetStatus)).BeginInit();
            this.SuspendLayout();
            // 
            // dataGridViewStatus
            // 
            this.dataGridViewStatus.AutoGenerateColumns = false;
            this.dataGridViewStatus.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewStatus.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn1,
            this.dataGridViewTextBoxColumn2,
            this.dataGridViewTextBoxColumn3,
            this.dataGridViewTextBoxColumn4,
            this.dataGridViewTextBoxColumn5,
            this.dataGridViewTextBoxColumn6,
            this.dataGridViewTextBoxColumn7,
            this.dataGridViewTextBoxColumn8});
            this.dataGridViewStatus.DataSource = this.dataTableStatusBindingSource;
            this.dataGridViewStatus.Location = new System.Drawing.Point(12, 12);
            this.dataGridViewStatus.Name = "dataGridViewStatus";
            this.dataGridViewStatus.RowTemplate.Height = 23;
            this.dataGridViewStatus.Size = new System.Drawing.Size(876, 476);
            this.dataGridViewStatus.TabIndex = 0;
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.DataPropertyName = "地址";
            this.dataGridViewTextBoxColumn1.HeaderText = "地址";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            // 
            // dataGridViewTextBoxColumn2
            // 
            this.dataGridViewTextBoxColumn2.DataPropertyName = "端口";
            this.dataGridViewTextBoxColumn2.HeaderText = "端口";
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.DataPropertyName = "负载";
            this.dataGridViewTextBoxColumn3.HeaderText = "负载";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            // 
            // dataGridViewTextBoxColumn4
            // 
            this.dataGridViewTextBoxColumn4.DataPropertyName = "内存";
            this.dataGridViewTextBoxColumn4.HeaderText = "内存";
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            // 
            // dataGridViewTextBoxColumn5
            // 
            this.dataGridViewTextBoxColumn5.DataPropertyName = "当前IO";
            this.dataGridViewTextBoxColumn5.HeaderText = "当前IO";
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            // 
            // dataGridViewTextBoxColumn6
            // 
            this.dataGridViewTextBoxColumn6.DataPropertyName = "当前流量";
            this.dataGridViewTextBoxColumn6.HeaderText = "当前流量";
            this.dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
            // 
            // dataGridViewTextBoxColumn7
            // 
            this.dataGridViewTextBoxColumn7.DataPropertyName = "数据库地址";
            this.dataGridViewTextBoxColumn7.HeaderText = "数据库地址";
            this.dataGridViewTextBoxColumn7.Name = "dataGridViewTextBoxColumn7";
            // 
            // dataGridViewTextBoxColumn8
            // 
            this.dataGridViewTextBoxColumn8.DataPropertyName = "数据库端口";
            this.dataGridViewTextBoxColumn8.HeaderText = "数据库端口";
            this.dataGridViewTextBoxColumn8.Name = "dataGridViewTextBoxColumn8";
            // 
            // dataTableStatusBindingSource
            // 
            this.dataTableStatusBindingSource.DataMember = "DataTableStatus";
            this.dataTableStatusBindingSource.DataSource = this.dataSetStatus;
            // 
            // dataSetStatus
            // 
            this.dataSetStatus.DataSetName = "DataSetStatus";
            this.dataSetStatus.SchemaSerializationMode = System.Data.SchemaSerializationMode.IncludeSchema;
            // 
            // FormStatus
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(900, 500);
            this.Controls.Add(this.dataGridViewStatus);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "FormStatus";
            this.ShowInTaskbar = false;
            this.Text = "FormStatus";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewStatus)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataTableStatusBindingSource)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataSetStatus)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView dataGridViewStatus;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn7;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn8;
        private System.Windows.Forms.BindingSource dataTableStatusBindingSource;
        private DataSetStatus dataSetStatus;
 

    }
}