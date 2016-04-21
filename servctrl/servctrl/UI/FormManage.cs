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
    public partial class FormManage : Form
    {
        IDataCenter _DC = null;
        public FormManage( IDataCenter dc )
        {
            InitializeComponent();
            this.TopLevel = false;
            _DC = dc;
        }
    }
}
