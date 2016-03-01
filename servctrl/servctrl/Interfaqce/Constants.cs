using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sloong.Interface
{
    public enum RunStatus
    {
        Stop,
        Run,
        Exit,
    }

    public class PageMessage
    {
        public PageMessage(MessageType t, Object[] p = null, Func<object, bool> f = null)
        {
            Type = t;
            Params = p;
            CallBackFunc = f;
        }
        public MessageType Type;
        public Object[] Params;
        public Func<object, bool> CallBackFunc;
    }

    public enum MessageType
    {
        ShowHideForm,
        RefreshWallpaper,
        SwitchConfig,
        SetWallpaper,
        RefreshPictureBox,
        ResetIndexList,
        RefreshPictureBoxAndWallpaper,
        ObjectAdd,
        ObjectRemove,
        ObjectModify,
        IndexMoveToNext,
        IndexMoveToPrev,
        SendRequest,
        SendRequestWithData,
        ExitApp,
        SwitchUI,
        ShowBalloonMessage,
        WaitThumbImage,
    }

    public enum UIType
    {
        Main,
        Online,
        Local,
        UserInfo,
        Setting,
        Config,
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
        MessageList,
        SendList,
        ConnectStatus,
        StyleList,
        MainUIContext,
        SocketMap,
        Log,
    }

    public static class RecvPackCode
    {
        public const byte StyleList = 0x01;
        public const byte PictureList = 0x02;
        public const byte OperationResult = 0x03;
    }

    public static class SendPackCode
    {
        public const byte GetStyleList = 0x01;
        public const byte GetPictureList = 0x02;
        public const byte UserLogin = 0x03;
        public const byte UpdateUserSetting = 0x04;
    }

    public static class StaticDefine
    {
        public const string DefaultWallpaper = @"C:\Windows\Web\Wallpaper\Windows\img0.jpg";
        public const int HotKeyMagic = 52313;
        public const int HotKeyNextPic = 52314;
        public const int HotKeyPrevPic = 52315;
        public const int HotKeyNextCon = 52316;
        public const int HotKeyPrevCon = 52317;
    }

    public static class MessageDefine
    {
        public const string TargetPathNonexistent = "目标路径不存在";
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

    [Serializable()]
    public class PictureInfo
    {
        /// <summary>
        /// The file md5, if this file is web file, used this can get the original file and thumb file.
        /// </summary>
        public string MD5;
        /// <summary>
        /// used in web file. 
        /// </summary>
        public int StyleID;
        /// <summary>
        /// is not web file
        /// </summary>
        public bool IsOnline;
        /// <summary>
        /// the file local path, if is web file, may be is empty. and should check the file is not exist.
        /// </summary>
        public string Path;
        /// <summary>
        /// file name.
        /// </summary>
        public string Name;
        /// <summary>
        /// file thumb path. the size is 600x600.
        /// </summary>
        public string ThumbPath;
        /// <summary>
        /// file preview path. the size is 100x100
        /// </summary>
        public string PreviewPath;
    }

    public delegate bool CallBackFunc(MessagePackage page);
    public class MessagePackage
    {
        public string SendMessage = string.Empty;
        public bool IsSent = false;
        public string[] ReceivedMessages = null;
        public bool IsReceived = false;
        public byte[] ReceivedExData = null;
        public bool NeedExData = false;
        public long SwiftNumber = -1;
        public CallBackFunc ReceivedHandler = null;
        public object[] MessageExInfo = null;
    }
}
