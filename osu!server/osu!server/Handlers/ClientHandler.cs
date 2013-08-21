using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;   
using System.Net.Sockets;
using System.Threading;
using System.IO;

namespace osu_server.Handlers
{
    public class ClientHandler
    {
        public TcpClient client;
        public Thread myThread;
        public bool running = true;
        BinaryReader br;
        StreamWriter bw;
        public ClientHandler(TcpClient c)
        {
            this.client = c;
            br = new BinaryReader(c.GetStream());
            bw = new StreamWriter(c.GetStream());
        }
        public void run()
        {
            int sendcount = 0;
            while (running)
            {
                try
                {
                    byte[] data = new byte[1024];
                    int x = br.Read(data, 0, 1024);
                    sendcount++;
                    List<StringBuilder> sbl = new List<StringBuilder>();
                    StringBuilder sb = new StringBuilder();
                    for (int y = 0; y < 1024; y++)
                    {                        
                        sb.Append(data[y].ToString("X2"));
                        if (data[y] == 10)
                        {
                            sbl.Add(sb);
                            sb = new StringBuilder();
                        }
                        if (data[y] == 0)
                        {
                            break;
                        }
                    }
                    replyLogin();                    
                   /* string s = br.ReadLine();
                    if (s == null)
                    {
                        running = false;
                        break;
                    }*/
                    StreamWriter sw = new StreamWriter("log", true);
                    foreach (StringBuilder sb2 in sbl)
                    {
                        Console.Write(sb2.ToString());
                        sw.WriteLine(sb2.ToString());
                    }
                    sw.Close();
                    }
                catch
                {
                    running = false;
                }
            }
            Console.WriteLine("Client disconnecting on " + (client.Client.RemoteEndPoint as IPEndPoint).Address.ToString());
        }
        private void replyLogin()
        {
            bw.Write((short)5);
            bw.Write((short)1024);
            bw.Write((short)0);
            bw.Write((short)-256);
            bw.Write((short)-1);
        }
    }
}
