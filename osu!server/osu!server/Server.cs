using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using osu_server.Handlers;

namespace osu_server
{
    class Server
    {
        static int port = 3333;
        static bool run = true;
        public static void start()
        {
            Console.WriteLine("Starting server");
            TcpListener listener = new TcpListener(port);
            listener.Start();
            new Thread(new ThreadStart(input)).Start();
            while (run) //multicliented through use of multiple threads, but should still be somewhat efficient as read() is a blocking method so most threads will be blocked
            {
                TcpClient c = listener.AcceptTcpClient();
                Console.WriteLine("Client connected at {0}", (c.Client.RemoteEndPoint as IPEndPoint).Address.ToString());
                ClientHandler ch = new ClientHandler(c);
                Thread t = new Thread(new ThreadStart(ch.run));
                ch.myThread = t;
                t.Start();
            }
        }
        public static void input()
        {
            while (run)
            {
                string[] line = Console.ReadLine().ToLower().Split(' ');
                switch (line[0])
                {
                    case "exit":
                        run = false;
                        Environment.Exit(0);
                        break;
                }
            }
        }
    }
}
