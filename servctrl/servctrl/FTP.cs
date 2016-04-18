using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace Sloong
{
    class FTP
    {
        string User;
        string Password;
        string Server;
        NetworkCredential Credential;
        public FTP( string user, string password, string server )
        {
            User = user;
            Password = password;
            Server = server;
            Credential = new NetworkCredential(User, Password);
        }
        
        public void Upload(string localFile, string ftpPath)
        {
            CheckDirectory(ftpPath);
            FileInfo fileInf = new FileInfo(localFile);
            FtpWebRequest reqFTP;
            // 根据uri创建FtpWebRequest对象   
            reqFTP = (FtpWebRequest)FtpWebRequest.Create(new Uri(Server + "/" + ftpPath + fileInf.Name));
            // ftp用户名和密码  
            reqFTP.Credentials = Credential;

            reqFTP.UsePassive = false;
            // 默认为true，连接不会被关闭  
            // 在一个命令之后被执行  
            reqFTP.KeepAlive = false;
            // 指定执行什么命令  
            reqFTP.Method = WebRequestMethods.Ftp.UploadFile;
            // 指定数据传输类型  
            reqFTP.UseBinary = true;
            // 上传文件时通知服务器文件的大小  
            reqFTP.ContentLength = fileInf.Length;
            // 缓冲大小设置为2kb  
            int buffLength = 2048;
            byte[] buff = new byte[buffLength];
            int contentLen;
            // 打开一个文件流 (System.IO.FileStream) 去读上传的文件  
            FileStream fs = fileInf.OpenRead();
            try
            {
                // 把上传的文件写入流  
                Stream strm = reqFTP.GetRequestStream();
                // 每次读文件流的2kb  
                contentLen = fs.Read(buff, 0, buffLength);
                // 流内容没有结束  
                while (contentLen != 0)
                {
                    // 把内容从file stream 写入 upload stream  
                    strm.Write(buff, 0, contentLen);
                    contentLen = fs.Read(buff, 0, buffLength);
                }
                // 关闭两个流  
                strm.Close();
                fs.Close();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public void Delete( string ftpPath )
        {
            FtpWebRequest reqFTP;
            try
            {
                reqFTP = (FtpWebRequest)FtpWebRequest.Create(new Uri(Server + ftpPath));
                reqFTP.Method = WebRequestMethods.Ftp.DeleteFile;
                reqFTP.UseBinary = true;
                reqFTP.Credentials = Credential;
                reqFTP.UsePassive = false;
                FtpWebResponse listResponse = (FtpWebResponse)reqFTP.GetResponse();
                string sStatus = listResponse.StatusDescription;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        //从ftp服务器上下载文件的功能  
        public void Download( string ftpFolder, string saveFolder, string file )
        {
            FtpWebRequest reqFTP;
            try
            {
                FileStream outputStream = new FileStream(saveFolder + "\\" + file, FileMode.Create);
                reqFTP = (FtpWebRequest)FtpWebRequest.Create(new Uri(Server + ftpFolder + file));
                reqFTP.Method = WebRequestMethods.Ftp.DownloadFile;
                reqFTP.UseBinary = true;
                reqFTP.Credentials = new NetworkCredential(User, Password);
                reqFTP.UsePassive = false;
                FtpWebResponse response = (FtpWebResponse)reqFTP.GetResponse();
                Stream ftpStream = response.GetResponseStream();
                long cl = response.ContentLength;
                int bufferSize = 2048;
                int readCount;
                byte[] buffer = new byte[bufferSize];
                readCount = ftpStream.Read(buffer, 0, bufferSize);
                while (readCount > 0)
                {
                    outputStream.Write(buffer, 0, readCount);
                    readCount = ftpStream.Read(buffer, 0, bufferSize);
                }
                ftpStream.Close();
                outputStream.Close();
                response.Close();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public void CheckDirectory(string ftpPath)
        {
            string fullDir = ParseDirectory(ftpPath);
            string[] dirs = fullDir.Split('/');
            string curDir = "/";
            for (int i = 0; i < dirs.Length; i++)
            {
                string dir = dirs[i];
                //如果是以/开始的路径,第一个为空    
                if (dir != null && dir.Length > 0)
                {
                    try
                    {
                        curDir += dir + "/";
                        if (!DirectoryIsExist(curDir))
                            CreateDirectory( curDir);
                    }
                    catch (Exception)
                    { }
                }
            }
        }

        public static string ParseDirectory(string ftpPath)
        {
            return ftpPath.Substring(0, ftpPath.LastIndexOf("/"));
        }

        //创建目录  
        public bool CreateDirectory(string ftpPath)
        {
            FtpWebRequest req = (FtpWebRequest)WebRequest.Create(Server + ftpPath);
            req.Credentials = new NetworkCredential(User, Password);
            req.Method = WebRequestMethods.Ftp.MakeDirectory;
            try
            {
                FtpWebResponse response = (FtpWebResponse)req.GetResponse();
                response.Close();
            }
            catch (Exception)
            {
                req.Abort();
                return false;
            }
            req.Abort();
            return true;
        }

        /// <summary>
        /// 检测目录是否存在
        /// </summary>
        /// <param name="pFtpServerIP"></param>
        /// <param name="pFtpUserID"></param>
        /// <param name="pFtpPW"></param>
        /// <returns>false不存在，true存在</returns>
        public bool DirectoryIsExist(string ftpPath)
        {
            string[] value = GetFileList(ftpPath);
            if (value == null)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        public string[] GetFileList(string ftpPath)
        {
            StringBuilder result = new StringBuilder();
            try
            {
                FtpWebRequest reqFTP = (FtpWebRequest)WebRequest.Create(Server + ftpPath);
                reqFTP.UseBinary = true;
                reqFTP.Credentials = new NetworkCredential(User, Password);
                reqFTP.Method = WebRequestMethods.Ftp.ListDirectoryDetails;

                WebResponse response = reqFTP.GetResponse();
                StreamReader reader = new StreamReader(response.GetResponseStream());
                string line = reader.ReadLine();
                while (line != null)
                {
                    result.Append(line);
                    result.Append("\n");
                    line = reader.ReadLine();
                }
                reader.Close();
                response.Close();
                return result.ToString().Split('\n');
            }
            catch
            {
                return null;
            }
        }
    }
}
