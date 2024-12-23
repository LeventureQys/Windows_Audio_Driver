namespace WindowsAudioDriver
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            btn_init_graph = new Button();
            btn_enum_devices = new Button();
            richTextBox1 = new RichTextBox();
            tabControl1 = new TabControl();
            tabPage1 = new TabPage();
            tabPage2 = new TabPage();
            btn_get_alldevice_output = new Button();
            tabControl1.SuspendLayout();
            tabPage1.SuspendLayout();
            SuspendLayout();
            // 
            // btn_init_graph
            // 
            btn_init_graph.Location = new Point(6, 6);
            btn_init_graph.Name = "btn_init_graph";
            btn_init_graph.Size = new Size(117, 23);
            btn_init_graph.TabIndex = 0;
            btn_init_graph.Text = "初始化音频图";
            btn_init_graph.UseVisualStyleBackColor = true;
            btn_init_graph.Click += btn_init_graph_Click;
            // 
            // btn_enum_devices
            // 
            btn_enum_devices.Location = new Point(6, 35);
            btn_enum_devices.Name = "btn_enum_devices";
            btn_enum_devices.Size = new Size(117, 23);
            btn_enum_devices.TabIndex = 1;
            btn_enum_devices.Text = "枚举所有输入设备";
            btn_enum_devices.UseVisualStyleBackColor = true;
            btn_enum_devices.Click += btn_enum_devices_Click;
            // 
            // richTextBox1
            // 
            richTextBox1.Location = new Point(279, 12);
            richTextBox1.Name = "richTextBox1";
            richTextBox1.Size = new Size(371, 426);
            richTextBox1.TabIndex = 2;
            richTextBox1.Text = "";
            // 
            // tabControl1
            // 
            tabControl1.Controls.Add(tabPage1);
            tabControl1.Controls.Add(tabPage2);
            tabControl1.Location = new Point(12, 12);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new Size(261, 426);
            tabControl1.TabIndex = 3;
            // 
            // tabPage1
            // 
            tabPage1.Controls.Add(btn_get_alldevice_output);
            tabPage1.Controls.Add(btn_init_graph);
            tabPage1.Controls.Add(btn_enum_devices);
            tabPage1.Location = new Point(4, 26);
            tabPage1.Name = "tabPage1";
            tabPage1.Padding = new Padding(3);
            tabPage1.Size = new Size(253, 396);
            tabPage1.TabIndex = 0;
            tabPage1.Text = "tabPage1";
            tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            tabPage2.Location = new Point(4, 26);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new Size(253, 396);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "tabPage2";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // btn_get_alldevice_output
            // 
            btn_get_alldevice_output.Location = new Point(6, 64);
            btn_get_alldevice_output.Name = "btn_get_alldevice_output";
            btn_get_alldevice_output.Size = new Size(117, 23);
            btn_get_alldevice_output.TabIndex = 2;
            btn_get_alldevice_output.Text = "获得所有输出设备";
            btn_get_alldevice_output.UseVisualStyleBackColor = true;
            btn_get_alldevice_output.Click += btn_get_alldevice_output_Click;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 17F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(668, 449);
            Controls.Add(tabControl1);
            Controls.Add(richTextBox1);
            Name = "Form1";
            Text = "Form1";
            tabControl1.ResumeLayout(false);
            tabPage1.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private Button btn_init_graph;
        private Button btn_enum_devices;
        private RichTextBox richTextBox1;
        private TabControl tabControl1;
        private TabPage tabPage1;
        private TabPage tabPage2;
        private Button btn_get_alldevice_output;
    }
}
