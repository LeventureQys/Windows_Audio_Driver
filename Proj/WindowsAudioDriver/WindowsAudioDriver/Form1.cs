
using System.Linq;
using System.Text;

namespace WindowsAudioDriver
{
    public partial class Form1 : Form
    {
        private MediaAudio? media_audio;
        public Form1()
        {
            InitializeComponent();
            this.media_audio = new MediaAudio();
        }


        private void btn_init_graph_Click(object sender, EventArgs e)
        {
            if (this.media_audio != null)
            {
                this.media_audio.initialiazer_audioAsync();
            }
        }

        private async void btn_enum_devices_Click(object sender, EventArgs e)
        {
            if (this.media_audio != null)
            {
                List<System.String> list_ret = await this.media_audio.ShowAllCaptureDevice();
                // StringBuilder name = new StringBuilder();  // 不需要 StringBuilder 了
                foreach (var item in list_ret)
                {
                    this.richTextBox1.AppendText(item);  // 先追加设备名称
                    this.richTextBox1.AppendText(Environment.NewLine); // 然后立即追加换行符
                }
                // this.richTextBox1.AppendText(name.ToString()); // 不需要这行了
            }
        }
        private void Add(string name)
        {
            this.richTextBox1.AppendText(name);
            this.richTextBox1.AppendText(Environment.NewLine);
        }

        private async void btn_get_alldevice_output_Click(object sender, EventArgs e)
        {
            if (this.media_audio != null)
            {
                List<System.String> list_ret = await this.media_audio.ShowAllOutputDevice();
                // StringBuilder name = new StringBuilder();  // 不需要 StringBuilder 了
                foreach (var item in list_ret)
                {
                    this.richTextBox1.AppendText(item);  // 先追加设备名称
                    this.richTextBox1.AppendText(Environment.NewLine); // 然后立即追加换行符
                }
                // this.richTextBox1.AppendText(name.ToString()); // 不需要这行了
            }
        }
    }
}
