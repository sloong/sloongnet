using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sloong.Interface
{
    public class DataCenter : IDataCenter
    {
        public event EventHandler<PageMessage> PageMessage;

        private IDictionary<ShareItem, object> _internalDictionary;
        private IDictionary<string, object> _tempDictionary;
        public DataCenter()
        {
            _internalDictionary = new Dictionary<ShareItem, object>();
            _tempDictionary = new Dictionary<string, object>();
        }

        public void Add(ShareItem key, object value)
        {
            _internalDictionary.Add(key, value);
            SendMessage(MessageType.ObjectAdd, key);
        }

        public void SetTemp(string key, object value)
        {
            if (value != null)
                _tempDictionary.Add(key, value);
        }

        public object GetTemp(string key, bool noremove = false)
        {
            object value = null;
            if (_tempDictionary.ContainsKey(key))
            {
                value = _tempDictionary[key];
                if (!noremove)
                    _tempDictionary.Remove(key);
            }
            return value;
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

        public void SendMessage(MessageType t, Object obj = null, Func<object, bool> pCallBack = null)
        {

            if (this.PageMessage != null)
            {
                if (obj is object[])
                    this.PageMessage(this, new PageMessage(t, obj as object[], pCallBack));
                else
                    this.PageMessage(this, new PageMessage(t, new object[] { obj }, pCallBack));
            }
        }
    }
}
