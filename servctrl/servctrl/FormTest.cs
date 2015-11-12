using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace servctrl
{
    public partial class FormTest : Form
    {
        Socket m_Socket;
        public FormTest( ref Socket sock)
        {
            InitializeComponent();
            m_Socket = sock;
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            try
            {
                var msg = textBoxMsg.Text;
                byte[] bmsg = Encoding.ASCII.GetBytes(msg);
                var len = string.Format("{0:D8}", bmsg.Length);
                msg = len + msg;
                listBoxLog.Items.Add(msg);
                byte[] sendByte = Encoding.ASCII.GetBytes(msg);
                lock (m_Socket)
                {
                    m_Socket.Send(sendByte, sendByte.Length, 0);
                }

                // send end, rece the return.
                byte[] leng = new byte[8];
                m_Socket.Receive(leng, leng.Length, 0);
                var nlen = BitConverter.ToInt64(leng, 0);
                byte[] data = new byte[nlen];
                m_Socket.Receive(data, data.Length, 0);
                listBoxLog.Items.Add(Encoding.ASCII.GetString(data));
            }
            catch (Exception ex)
            {
                listBoxLog.Items.Add(ex.ToString());
            }
        }

        private void listBoxLog_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            groupBox1.Enabled = !checkBox1.Checked;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            AskForm askform = new AskForm();
            askform.ShowDialog();
            MessageBox.Show(askform.InputText);
        }
    }
}
