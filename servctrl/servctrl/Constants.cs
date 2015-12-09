using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace servctrl
{
    public enum RunStatus
    {
        Stop,
        Run,
        Exit,
    }

    public class ApplicationStatus
    {
        public RunStatus RunStatus { get; set; }
        public string ServerIP { get; set; }
        public int SendProt { get; set; }
        public int RecvBufferSize { get; set; }
        public bool AutoHide { get; set; }
        public bool ExitApp { get; set; }
    }
}
