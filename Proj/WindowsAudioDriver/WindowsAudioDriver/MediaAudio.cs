using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WindowsAudioDriver;
using Windows.Media.Capture;
using Windows.Media.Audio;
namespace WindowsAudioDriver
{
    internal class MediaAudio
    {
        private AudioDeviceInputNode? input_device;             //输入设备
        private AudioDeviceOutputNode? output_device;           //输出设备
        private AudioGraphSettings? audio_graph_settings;       //音频图
        private AudioGraph? audio_graph;


        /// <summary>
        /// 创建本文件所使用的音频图，将往其中插入各种Node
        /// </summary>
        /// <returns></returns>
        public async Task<int> initialiazer_audioAsync()
        {
            try
            {
                this.audio_graph_settings = new AudioGraphSettings(Windows.Media.Render.AudioRenderCategory.Media);
                CreateAudioGraphResult result = await AudioGraph.CreateAsync(this.audio_graph_settings);
                if (result.Status != AudioGraphCreationStatus.Success)
                {
                    //ShowErrorMessage("AudioGraph creation error: " + result.Status.ToString());
                    Console.WriteLine("AudioGraph creation error: " + result.Status.ToString());
                }

                this.audio_graph = result.Graph;

                //接下来要设置两个节点，分别是输入，这里输入设定为DM40设备，输出设定为QCY耳机
                //输入设备
                Windows.Devices.Enumeration.DeviceInformationCollection devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(Windows.Media.Devices.MediaDevice.GetAudioCaptureSelector());
                Windows.Devices.Enumeration.DeviceInformation? selectedDevice = null;
                Windows.Devices.Enumeration.DeviceInformation selected_output_device = null;
                foreach (var item in devices)
                {
                    if (item.Name.Contains("DM40"))
                    {
                        selectedDevice = item;
                        System.Console.Write("input device :" + item.Name);
                    }
                    else if (item.Name.Contains("QCY"))
                    {
                        selected_output_device = item;
                        //System.Console.WriteLine("output device:")
                    }
                }
                CreateAudioDeviceInputNodeResult ret = await this.audio_graph.CreateDeviceInputNodeAsync(Windows.Media.Capture.MediaCategory.Media, audio_graph.EncodingProperties, selectedDevice); ;
                var device_input = ret.DeviceInputNode;

                CreateAudioDeviceOutputNodeResult res = await this.audio_graph.CreateDeviceOutputNodeAsync();
                var device_output = res.DeviceOutputNode;
                device_input.AddOutgoingConnection(device_output);
                this.audio_graph.Start();
            }
            catch (Exception ex)
            {
                Console.Write(ex.ToString());
            }



            return 0;
        }

        public async Task<List<System.String>> ShowAllCaptureDevice()
        {
            //
            Windows.Devices.Enumeration.DeviceInformationCollection devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(Windows.Media.Devices.MediaDevice.GetAudioCaptureSelector());

            List<string> list_ret = new List<string>();
            foreach (var device in devices)
            {
                list_ret.Add(device.Name);
            }

            return list_ret;
        }

        public async Task<List<System.String>> ShowAllOutputDevice()
        {
            Windows.Devices.Enumeration.DeviceInformationCollection devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(Windows.Media.Devices.MediaDevice.GetAudioRenderSelector());

            List<string> list_ret = new List<string>();
            foreach (var device in devices)
            {
                list_ret.Add(device.Name);
            }

            return list_ret;
        }
    }
}
