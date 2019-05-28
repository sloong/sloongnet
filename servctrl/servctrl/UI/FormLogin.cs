using Sloong.Interface;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace servctrl.UI
{
    public partial class FormLogin : Form
    {
        IDataCenter _DC = null;
        public FormLogin( IDataCenter dc )
        {
            InitializeComponent();
            this.TopLevel = false;
            _DC = dc;
        }

        private void buttonLogin_Click(object sender, EventArgs e)
        {
            bool? bConnect = _DC[ShareItem.ConnectStatus] as bool?;

            if (bConnect == true)
                _DC.SendMessage(MessageType.SendRequest, string.Format("Sloong_User_Login|{0}|{1}", textBoxUser.Text, textBoxPwd.Text));
            else
                labelInfo.Text = "No connect to server.";
        }

        private void UserLoginCBF( object param )
        {
            MessageData pack = param as MessageData;

        }
    }
}
