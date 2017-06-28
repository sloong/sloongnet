using Microsoft.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Sloong
{
    public static class Utility
    {
        static Dictionary<PictureBox, PictureBoxInfo> PictureBoxList = new Dictionary<PictureBox, PictureBoxInfo>();
        public enum Direction
        {
            Left,
            Right,
            Centre
        };
        public class PictureBoxInfo
        {
            public Size Size;
            public Point Centre;
            public Point Location;
        }

        public enum Wallstyle
        {
            Fill = 10,
            Fit = 6,
            Stretch = 2,
            Tile = 1,
            Center = 0,
        }

        public static Wallstyle SloongWallstyle
        {
            get
            {
                RegistryKey key = Registry.CurrentUser;
                RegistryKey subKey = key.OpenSubKey("Control Panel\\Desktop");
                Wallstyle style = (Wallstyle)Enum.Parse(typeof(Wallstyle), subKey.GetValue("TileWallpaper").ToString());
                if (style == Wallstyle.Tile)
                {
                    return Wallstyle.Tile;
                }
                else
                {
                    style = (Wallstyle)Enum.Parse(typeof(Wallstyle), subKey.GetValue("WallpaperStyle").ToString());
                }
                key.Close();
                return style;
            }

            set
            {
                RegistryKey key = Registry.CurrentUser;
                RegistryKey subKey = key.OpenSubKey("Control Panel\\Desktop", true);
                if (value == Wallstyle.Tile)
                {
                    subKey.SetValue("TileWallpaper", Convert.ToInt32(value).ToString());
                }
                else
                {
                    subKey.SetValue("WallpaperStyle", Convert.ToInt32(value).ToString());
                }
                key.Close();
            }
        }

        public static string AssemblyFileVersion
        {
            get
            {
                object[] attributes = Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(AssemblyFileVersionAttribute), false);
                if (attributes.Length == 0)
                    return "";
                return ((AssemblyFileVersionAttribute)attributes[0]).Version;
            }
        }

        public static bool isAutoStart
        {
            get
            {
                RegistryKey key = Registry.CurrentUser;
                RegistryKey subKey = key.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
                string path = string.Empty;
                try
                {
                    path = subKey.GetValue("lWallpapers").ToString();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                    path = null;
                }
                key.Close();
                if (null == path)
                {
                    return false;
                }
                else if (path != System.Windows.Forms.Application.ExecutablePath)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }

            set
            {
                RegistryKey key = Registry.CurrentUser;
                RegistryKey subKey = key.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Run", true);
                if (value == true)
                {
                    subKey.SetValue("lWallpapers", System.Windows.Forms.Application.ExecutablePath);
                }
                else
                {
                    subKey.DeleteValue("lWallpapers");
                }
            }
        }

        public static string AppDataFolder
        {
            get
            {
                return System.Windows.Forms.Application.UserAppDataPath;
            }
        }

        /// <summary>
        /// 设置系统壁纸的Style,如填充或者适应
        /// </summary>
        public static void SetWallpaperStyle(string style)
        {
            switch (style)
            {
                case "填充":
                    Utility.SloongWallstyle = Utility.Wallstyle.Fill;
                    break;
                case "适应":
                    Utility.SloongWallstyle = Utility.Wallstyle.Fit;
                    break;
                case "拉伸":
                    Utility.SloongWallstyle = Utility.Wallstyle.Stretch;
                    break;
                case "平铺":
                    Utility.SloongWallstyle = Utility.Wallstyle.Tile;
                    break;
                case "居中":
                    Utility.SloongWallstyle = Utility.Wallstyle.Center;
                    break;
                default:
                    break;
            }
        }

        public static Size CompareSize(Size bitmap, Size pictureBox)
        {
            Size res = new Size();
            if (bitmap.Width > pictureBox.Width || bitmap.Height > pictureBox.Height)
            {
                //SetPictureBoxSize(ref pictureBox, ptSize, direction);
            }
            return res;
        }


        public static Size NewSize(int maxWidth, int maxHeight, int width, int height)
        {
            double w = 0.0;
            double h = 0.0;
            double sw = Convert.ToDouble(width);
            double sh = Convert.ToDouble(height);
            double mw = Convert.ToDouble(maxWidth);
            double mh = Convert.ToDouble(maxHeight);

            if (sw < mw && sh < mh)
            {
                w = sw;
                h = sh;
            }
            else if ((sw / sh) > (mw / mh))
            {
                w = maxWidth;
                h = (w * sh) / sw;
            }
            else
            {
                h = maxHeight;
                w = (h * sw) / sh;
            }
            return new Size(Convert.ToInt32(w), Convert.ToInt32(h));
        }

        public static string GetThumbImageWithRatio(string filePath, string savefolder, Size targeSize)
        {
            if (string.IsNullOrEmpty(filePath) || !File.Exists(filePath))
            {
                return string.Empty;
            }
            Size newSize = Size.Empty;
            using (Image image = Image.FromFile(filePath))
            {
                newSize = GetSizeWithRatio(targeSize, image.Size);
            }

            return GetThumbImage(filePath, savefolder, newSize.Height, newSize.Width);
        }


        public static string GetThumbImagePath(string path, string savepath, int maxHeight, int maxWidth)
        {
            var thumbPath = savepath;

            FileInfo finfo = new FileInfo(thumbPath);
            if (finfo.Extension == "")
            {
                if (savepath[savepath.Length - 1] != '\\')
                    savepath += "\\";

                if (!Directory.Exists(savepath))
                {
                    Directory.CreateDirectory(savepath);
                }

                FileInfo fi = new FileInfo(path);
                thumbPath = string.Format("{0}{1}_{2}_{3}{4}", savepath, fi.Name.Substring(0, fi.Name.Length - fi.Extension.Length), maxWidth, maxHeight, fi.Extension);

            }

            return thumbPath;

        }

        /// <summary>
        /// 获取缩略图
        /// </summary>
        /// <remarks>如果缩略图不存在,则根据参数在临时目录中创建后返回全路径</remarks>
        /// <returns>缩略图所在的全路径</returns>
        public static string GetThumbImage(string filePath, string savepath, int maxHeight, int maxWidth)
        {
            var thumbPath = GetThumbImagePath(filePath, savepath, maxHeight, maxWidth);

            Image img = Image.FromFile(filePath);
            ImageFormat thisFormat = img.RawFormat;
            Size newSize = NewSize(maxWidth, maxHeight, img.Width, img.Height);
            Bitmap outBmp = new Bitmap(newSize.Width, newSize.Height);
            Graphics g = Graphics.FromImage(outBmp);
            // 设置画布的描绘质量
            g.CompositingQuality = CompositingQuality.HighSpeed;
            g.SmoothingMode = SmoothingMode.HighSpeed;
            g.InterpolationMode = InterpolationMode.Low;
            try
            {
                g.DrawImage(img, new Rectangle(0, 0, newSize.Width, newSize.Height), 0, 0, img.Width, img.Height, GraphicsUnit.Pixel);

                EncoderParameters encoderParams = new EncoderParameters();
                long[] quality = new long[1];
                quality[0] = 100;
                EncoderParameter encoderParam = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, quality);
                encoderParams.Param[0] = encoderParam;
                //获得包含有关内置图像编码解码器的信息的ImageCodecInfo 对象.
                ImageCodecInfo[] arrayICI = ImageCodecInfo.GetImageEncoders();
                ImageCodecInfo jpegICI = null;
                for (int x = 0; x < arrayICI.Length; x++)
                {
                    if (arrayICI[x].FormatDescription.Equals("JPEG"))
                    {
                        jpegICI = arrayICI[x];
                        //设置JPEG编码
                        break;
                    }
                }

                if (jpegICI != null)
                {
                    outBmp.Save(thumbPath, jpegICI, encoderParams);
                }
                else
                {
                    outBmp.Save(thumbPath, thisFormat);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
            finally
            {
                g.Dispose();
                img.Dispose();
                outBmp.Dispose();
            }
            return thumbPath;
        }

        /// <summary>
        /// 设置PictureBox大小和位置
        /// </summary>
        public static void SetPictureBoxSize(ref System.Windows.Forms.PictureBox picBox, Size sz, Direction direction)
        {
            PictureBoxInfo info = null;
            foreach (var item in PictureBoxList)
            {
                if (item.Key == picBox)
                {
                    info = item.Value;
                }
            }
            if (info == null)
            {
                info = new PictureBoxInfo();
                info.Centre = new Point(picBox.Location.X + picBox.Width / 2, picBox.Location.Y + picBox.Height / 2);
                info.Location = picBox.Location;
                info.Size = picBox.Size;
                PictureBoxList.Add(picBox, info);
            }

            Size newSize = GetSizeWithRatio(info.Size, sz);

            picBox.Width = newSize.Width;
            picBox.Height = newSize.Height;

            switch (direction)
            {
                case Direction.Left:
                    picBox.Location = new Point(info.Location.X, info.Centre.Y - picBox.Height / 2);
                    break;
                case Direction.Right:
                    picBox.Location = new Point(info.Location.X + info.Size.Width - picBox.Width, info.Centre.Y - picBox.Height / 2);
                    break;
                case Direction.Centre:
                default:
                    picBox.Location = new Point(info.Centre.X - picBox.Width / 2, info.Centre.Y - picBox.Height / 2);
                    break;
            }

        }

        /// <summary>
        /// 获取根据参考宽高计算的宽高值
        /// </summary>
        public static Size GetSizeWithRatio(Size targetSize, Size SourSize)
        {
            Size result = new Size();
            // 目标的当前宽高比
            double dTargRatio = (double)targetSize.Width / (double)targetSize.Height;
            // 参考的宽高比
            double dSrcRatio = (double)SourSize.Width / (double)SourSize.Height;
            if (1.0 < dSrcRatio)
            {
                result.Height = Convert.ToInt32(targetSize.Width / dSrcRatio);
                result.Width = targetSize.Width;
            }
            else
            {
                result.Width = Convert.ToInt32(dSrcRatio * targetSize.Height);
                result.Height = targetSize.Height;
            }
            return result;
        }
        /// <summary>
        /// 通过ping测试目标地址是否联通
        /// </summary>
        public static bool TestNetwork(string targetIP)
        {
            try
            {
                Ping pingSender = new Ping();
                PingOptions options = new PingOptions();
                options.DontFragment = true;
                string data = "Sloong soft Network Test";
                byte[] buffer = Encoding.ASCII.GetBytes(data);
                int timeout = 120;
                PingReply reply = pingSender.Send(targetIP, timeout, buffer, options);
                if (reply.Status == IPStatus.Success)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                return false;
            }
        }

        /// <summary>
        /// 计算文件的大小
        /// </summary>
        public static long FileSize(string filePath)
        {
            FileInfo fileInfo = new FileInfo(filePath);
            if (fileInfo.Exists)
            {
                return fileInfo.Length;
            }
            else
            {
                return 0;
            }
        }

        /// <summary>
        /// 计算目录的大小
        /// </summary>
        public static long GetDirectoryLength(string dirPath)
        {
            long len = 0;
            if (!Directory.Exists(dirPath))
            {
                len = FileSize(dirPath);
            }
            else
            {
                DirectoryInfo di = new DirectoryInfo(dirPath);
                foreach (FileInfo fi in di.GetFiles())
                {
                    len += fi.Length;
                }

                DirectoryInfo[] dis = di.GetDirectories();
                if (dis.Length > 0)
                {
                    for (int i = 0; i < dis.Length; i++)
                    {
                        len += GetDirectoryLength(dis[i].FullName);
                    }
                }
            }
            return len;
        }

        /// <summary>
        /// 删除一个目录和目录中的所有文件和文件夹
        /// </summary>
        public static void DeleteDirectory(string deleteDir)
        {
            try
            {
                DirectoryInfo di = new DirectoryInfo(deleteDir);
                foreach (var fi in di.GetFiles())
                {
                    try
                    {
                        fi.Delete();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.ToString());
                    }
                }

                DirectoryInfo[] dis = di.GetDirectories();
                if (dis.Length > 0)
                {
                    for (int i = 0; i < dis.Length; i++)
                    {
                        DeleteDirectory(dis[i].FullName);
                    }
                }
                di.Delete(true);
            }
            catch (Exception err)
            {
                Console.WriteLine(err.Message);
            }
        }

        public static List<string> GetAllSubFileList(DirectoryInfo dir, string[] fileFormat)
        {
            List<string> arr = new List<string>();
            FileInfo[] allFile = dir.GetFiles();
            foreach (FileInfo fi in allFile)
            {
                // Check the current file format
                foreach (var item in fileFormat)
                {
                    if (fi.Extension.ToUpper() == item.ToUpper())
                    {
                        string fullPath = string.Format("{0}\\{1}", fi.DirectoryName, fi.Name);
                        arr.Add(fullPath);
                        break;
                    }
                }
            }

            DirectoryInfo[] allDir = dir.GetDirectories();
            foreach (DirectoryInfo d in allDir)
            {
                var att = File.GetAttributes(d.FullName);
                var res = att & FileAttributes.Hidden;
                if (0 == (int)res)
                    arr.AddRange(GetAllSubFileList(d, fileFormat));
            }

            return arr;
        }

        /// <summary>
        /// 计算文件的MD5校验
        /// </summary>
        /// <param name="fileName"></param>
        /// <returns></returns>
        public static string GetMD5HashFromFile(string fileName)
        {
            try
            {
                if (File.Exists("target.tmp"))
                    File.Delete("target.tmp");
                File.Copy(fileName, "target.tmp");
                FileStream file = new FileStream("target.tmp", FileMode.Open);
                System.Security.Cryptography.MD5 md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
                byte[] retVal = md5.ComputeHash(file);
                file.Close();
                File.Delete("target.tmp");

                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < retVal.Length; i++)
                {
                    sb.Append(retVal[i].ToString("x2"));
                }
                return sb.ToString();
            }
            catch (Exception ex)
            {
                throw new Exception("GetMD5HashFromFile() fail,error:" + ex.Message);
            }
        }


        public static void HttpDownloadFile(string url, string savepath, bool hidefile = true)
        {
            try
            {
                if (File.Exists(savepath))
                    File.Delete(savepath);
                // 设置参数
                HttpWebRequest request = WebRequest.Create(url) as HttpWebRequest;
                //发送请求并获取相应回应数据
                Stream oStream = request.GetResponse().GetResponseStream();
                //创建本地文件写入流
                Stream stream = new FileStream(savepath, FileMode.Create);
                byte[] bArr = new byte[1024];
                int size = oStream.Read(bArr, 0, (int)bArr.Length);
                while (size > 0)
                {
                    stream.Write(bArr, 0, size);
                    size = oStream.Read(bArr, 0, (int)bArr.Length);
                }
                stream.Close();
                oStream.Close();
            }
            catch (System.Net.WebException e)
            {
                throw e;
            }
            catch (Exception e)
            {
                Console.Write(e.ToString());
            }

        }


        public static void Serialize(object obj, string path)
        {
            if (obj == null)
                return;
            string temp = path + ".tmp";
            BinaryFormatter formatter = new BinaryFormatter();
            Stream stream = new FileStream(temp, FileMode.Create, FileAccess.Write);
            formatter.Serialize(stream, obj);
            stream.Close();
            if (File.Exists(path))
                File.Delete(path);
            File.Move(temp, path);
        }

        public static object Deserialize(string path)
        {
            if (!File.Exists(path))
            {
                throw new Exception("Deserialize failed. File no exist");
            }
            BinaryFormatter formatter = new BinaryFormatter();
            FileStream stream = new FileStream(path, FileMode.Open, FileAccess.Read);
            var obj = formatter.Deserialize(stream);
            stream.Close();
            return obj;
        }

        public static string MD5_Encoding(string str, Encoding enc = null)
        {
            MD5 md = MD5.Create();
            if (enc == null)
                enc = Encoding.ASCII;
            return BitConverter.ToString(md.ComputeHash(enc.GetBytes(str))).Replace("-", "");
        }

        public static byte[] LongToBytes(long l)
        {
            return BitConverter.GetBytes(IPAddress.HostToNetworkOrder(l));
        }

        public static long BytesToLong(byte[] b)
        {
            return IPAddress.NetworkToHostOrder(BitConverter.ToInt64(b, 0));
        }

        public static byte[] RecvEx(Socket sock, long len, int nOverTime)
        {
            byte[] recvRes = new byte[len];
            int nRecvSize = 0;
            int nBufferSize = sock.ReceiveBufferSize;
            sock.ReceiveTimeout = nOverTime;
            try
            {
                while (nRecvSize < len)
                {
                    byte[] recv;
                    int nNorecvSize = (int)len - nRecvSize;
                    if (nNorecvSize > nBufferSize)
                    {
                        recv = new byte[nBufferSize];
                    }
                    else
                    {
                        recv = new byte[nNorecvSize];
                    }

                    int readsize = sock.Receive(recv, recv.Length, 0);
                    recv.CopyTo(recvRes, nRecvSize);
                    nRecvSize += readsize;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            return recvRes;
        }

        /// <summary>  
        /// 截取字节数组  
        /// </summary>  
        /// <param name="srcBytes">要截取的字节数组</param>  
        /// <param name="startIndex">开始截取位置的索引</param>  
        /// <param name="length">要截取的字节长度</param>  
        /// <returns>截取后的字节数组</returns>  
        public static byte[] SubByte(byte[] srcBytes, int startIndex, int length)
        {
            System.IO.MemoryStream bufferStream = new System.IO.MemoryStream();
            byte[] returnByte = new byte[] { };
            if (srcBytes == null) { return returnByte; }
            if (startIndex < 0) { startIndex = 0; }
            if (startIndex < srcBytes.Length)
            {
                if (length < 1 || length > srcBytes.Length - startIndex) { length = srcBytes.Length - startIndex; }
                bufferStream.Write(srcBytes, startIndex, length);
                returnByte = bufferStream.ToArray();
                bufferStream.SetLength(0);
                bufferStream.Position = 0;
            }
            bufferStream.Close();
            bufferStream.Dispose();
            return returnByte;
        }

        public static void SendEx(Socket sock, byte[] data)
        {
            SendEx(sock, data, data.Length);
        }
        
        public static void SendEx(Socket sock, List<byte> data)
        {
            SendEx(sock, data.ToArray());
        }

        public static void SendEx(Socket sock, byte[] data,int nSendLength)
        {
            int nSentSize = 0;
            int nBufferSize = sock.SendBufferSize;
            while (nSentSize < nSendLength)
            {
                byte[] send;
                int nNosendSize = nSendLength - nSentSize;
                if (nNosendSize > nBufferSize)
                {
                    send = SubByte(data, nSentSize, nBufferSize);
                }
                else
                {
                    send = SubByte(data, nSentSize, nNosendSize);
                }

                int nSent = sock.Send(send);
                nSentSize += nSent;
            }
        }

        /// <summary>
        /// Check the string is not null, empty or "", if have value, return true.
        /// </summary>
        /// <param name="str">the checked string</param>
        /// <returns>if is null or empty or '' return false.</returns>
        public static bool Check(string str)
        {
            if (str == null || str == string.Empty || str.Trim() == "")
                return false;
            else
                return true;
        }

        public static void SavePicture(byte[] picBytes, int fileSize, string path)
        {
            FileInfo fi = new FileInfo(path);
            if (!Directory.Exists(fi.DirectoryName))
                Directory.CreateDirectory(fi.DirectoryName);

            FileStream fs = new FileStream(path, FileMode.OpenOrCreate, FileAccess.ReadWrite);

            fs.Write(picBytes, 0, fileSize);
            fs.Dispose();
            fs.Close();
        }

        /// <summary>  
        /// 字符串压缩  
        /// </summary>  
        /// <param name="strSource"></param>  
        /// <returns></returns>  
        public static byte[] Compress(byte[] data)
        {
            try
            {
                MemoryStream ms = new MemoryStream();
                System.IO.Compression.GZipStream zip = new System.IO.Compression.GZipStream(ms, System.IO.Compression.CompressionMode.Compress, true);
                zip.Write(data, 0, data.Length);
                zip.Close();
                byte[] buffer = new byte[ms.Length];
                ms.Position = 0;
                ms.Read(buffer, 0, buffer.Length);
                ms.Close();
                return buffer;

            }
            catch (Exception e)
            {
                throw new Exception(e.Message);
            }
        }

        /// <summary>  
        /// 字符串解压缩  
        /// </summary>  
        /// <param name="strSource"></param>  
        /// <returns></returns>  
        public static byte[] Decompress(byte[] data)
        {
            try
            {
                MemoryStream ms = new MemoryStream(data);
                System.IO.Compression.GZipStream zip = new System.IO.Compression.GZipStream(ms, System.IO.Compression.CompressionMode.Decompress, true);
                MemoryStream msreader = new MemoryStream();
                byte[] buffer = new byte[0x1000];
                while (true)
                {
                    int reader = zip.Read(buffer, 0, buffer.Length);
                    if (reader <= 0)
                    {
                        break;
                    }
                    msreader.Write(buffer, 0, reader);
                }
                zip.Close();
                ms.Close();
                msreader.Position = 0;
                buffer = msreader.ToArray();
                msreader.Close();
                return buffer;
            }
            catch (Exception e)
            {
                throw new Exception(e.Message);
            }
        }

        public static string CompressString(string str,Encoding enc = null)
        {
            if (enc == null)
                enc = Encoding.ASCII;
            string compressString = "";
            byte[] compressBeforeByte = enc.GetBytes(str);
            byte[] compressAfterByte = Compress(compressBeforeByte);
            compressString = enc.GetString(compressAfterByte);
            return compressString;
        }

        public static string DecompressString(string str,Encoding enc = null)
        {
            if (enc == null)
                enc = Encoding.ASCII;
            string compressString = "";
            byte[] compressBeforeByte = enc.GetBytes(str);
            byte[] compressAfterByte = Decompress(compressBeforeByte);
            compressString = enc.GetString(compressAfterByte);
            return compressString;
        }
    }
}
