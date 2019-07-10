using SharpDX.XInput;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace WaspCannonClient
{
    class Point
    {
        public float X, Y;
        public Point(float x, float y)
        {
            X = x;
            Y = y;
        }
    }

    class XInputController
    {
        Controller controller;
        Gamepad gamepad;
        public bool connected = false;
        public int deadband = 5000;
        public Point leftThumb = new Point(0, 0);
        public float leftTrigger, rightTrigger;

        public XInputController()
        {
            controller = new Controller(UserIndex.One);
            connected = controller.IsConnected;
        }

        // Call this method to update all class values
        public void Update()
        {
            if (!connected)
                return;

            gamepad = controller.GetState().Gamepad;

            leftThumb.X = (Math.Abs((float)gamepad.LeftThumbX) < deadband) ? 0 : (float)gamepad.LeftThumbX / short.MinValue * -100;
            leftThumb.Y = (Math.Abs((float)gamepad.LeftThumbY) < deadband) ? 0 : (float)gamepad.LeftThumbY / short.MaxValue * 100;

            leftTrigger = gamepad.LeftTrigger;
            rightTrigger = gamepad.RightTrigger;
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            XInputController controller = new XInputController();
            if (!controller.connected)
            {
                Console.WriteLine("Can't find controller!");
                Console.Read();
                return;
            }

            Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            var hosts = Dns.GetHostEntry("arduino-e40c");
            if (hosts.AddressList.Count() == 0)
            {
                Console.WriteLine("Can't find cannon!");
                Console.Read();
                return;
            }
            IPAddress serverAddr = hosts.AddressList[0];
            IPEndPoint endPoint = new IPEndPoint(serverAddr, 2390);

            while (true)
            {
                controller.Update();

                Console.WriteLine("{0}, {1}, {2}", fixThumbVal(controller.leftThumb.X), fixThumbVal(controller.leftThumb.Y), controller.rightTrigger);
                string message = string.Format("{0}/{1}/{2}", fixThumbVal(controller.leftThumb.X), fixThumbVal(controller.leftThumb.Y), controller.rightTrigger);
                sock.SendTo(Encoding.ASCII.GetBytes(message), endPoint);

                Thread.Sleep(40);
            }
        }

        static int fixThumbVal(double v)
        {
            if (Math.Abs(v) < 20)
                return 0;

            return (int) (v / -49);
        }
    }
}
