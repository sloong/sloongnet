using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sloong.Interfaqce
{
    public class GlobalShare : IPageHost
    {
        public event EventHandler<PageMessage> PageMessage;

        private IDictionary<ShareItem, object> _internalDictionary;
        public GlobalShare()
        {
            _internalDictionary = new Dictionary<ShareItem, object>();
        }

        public void Add(ShareItem key, object value)
        {
            _internalDictionary.Add(key, value);
            SendMessage(MessageType.ObjectAdd, key);
        }

        public bool ContainsKey(ShareItem key)
        {
            return _internalDictionary.ContainsKey(key);
        }

        public bool Remove(ShareItem key)
        {
            SendMessage(MessageType.ObjectRemove, key);
            return _internalDictionary.Remove(key);
        }

        public bool TryGetValue(ShareItem key, out object value)
        {
            return _internalDictionary.TryGetValue(key, out value);
        }

        public object this[ShareItem item]
        {
            get
            {
                if (ContainsKey(item))
                {
                    return _internalDictionary[item];
                }
                else
                {
                    return null;
                }
            }
            set
            {
                bool isExist = this.ContainsKey(item);
                _internalDictionary[item] = value;
                if (!isExist)
                {
                    SendMessage(MessageType.ObjectAdd, item);
                }
                else
                {
                    SendMessage(MessageType.ObjectModify, item);
                }
            }
        }

        public void SendMessage(MessageType t, Object obj = null)
        {
            if (this.PageMessage != null)
            {
                this.PageMessage(this, new PageMessage(t, obj));
            }
        }
    }
}
