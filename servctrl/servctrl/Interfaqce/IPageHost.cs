using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sloong.Interfaqce
{
    public enum MessageType
    {
        ShowHideMainForm,
        RefreshWallpaper,
        SwitchConfig,
        RefreshImageList,
        RefreshPictureBox,
        RefreshPictureBoxAndWallpaper,
        ObjectAdd,
        ObjectRemove,
        ObjectModify,
        IndexMoveToNext,
        IndexMoveToPrev,
        SendPake,
        ReceivePake,
        ExitApp,
    }

    public enum ShareItem
    {
        ConfigSelecter,
        LeftStatusbar,
        RightStatusbar,
        LoginUser,
        AppStatus,
        Configuation,
        ImageList,
        PreloadList,
        IndexList,
        PicSize,
        SendPack,
        ReceivePack,
        Log,
        SocketMap,
    }

    public interface IPageHost
    {
        object this[ShareItem item] { get; set; }
        void Add(ShareItem key, object value);
        bool Remove(ShareItem key);

        void SendMessage(MessageType t, Object obj = null);

        event EventHandler<PageMessage> PageMessage;
    }

    public class PageMessage
    {
        public PageMessage(MessageType t, Object p = null)
        {
            Type = t;
            Params = p;
        }
        public MessageType Type;
        public Object Params;
    }
}
