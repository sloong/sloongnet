using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sloong.Interface
{
    /// <summary>
    /// Host of IPluginPage.
    /// Expose interfaces Plugins
    /// </summary>
    public interface IDataCenter
    {
        object this[ShareItem item] { get; set; }
        void Add(ShareItem key, object value);
        bool Remove(ShareItem key);

        void SetTemp(string key, object value);
        object GetTemp(string key, bool noremove = false);
        void SendMessage(MessageType t, Object obj = null, Func<object, bool> pCallBack = null);

        event EventHandler<PageMessage> PageMessage;
    }
}
