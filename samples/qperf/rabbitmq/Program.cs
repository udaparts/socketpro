using System;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using System.Text;
using System.IO;

class Program
{
    //https://github.com/rabbitmq/rabbitmq-tutorials/blob/master/dotnet/NewTask/NewTask.cs
    static void Enqueue(string queueName, string hostName, string msg, int cycles)
    {
        System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
        var factory = new ConnectionFactory() { HostName = hostName };
        factory.UserName = "rabbit";
        factory.Password = "password";
        using (var connection = factory.CreateConnection())
        {
            using (var channel = connection.CreateModel())
            {
                byte[] body;
                QueueDeclareOk res = channel.QueueDeclare(queueName, true, false, false, null);
                if (msg != null)
                    body = Encoding.UTF8.GetBytes(msg);
                else
                    body = new byte[0];
                var properties = channel.CreateBasicProperties();
                properties.SetPersistent(true);
                sw.Start();
                for (int n = 0; n < cycles; ++n)
                {
                    channel.BasicPublish("", queueName, properties, body);
                }
                sw.Stop();
            }
        }
        Console.WriteLine(" [x] Enqueue {0} with count = {1}, time = {2} ms", "", cycles, sw.ElapsedMilliseconds);
    }

    //https://github.com/rabbitmq/rabbitmq-tutorials/blob/master/dotnet/Worker/Worker.cs
    static void Dequeue(string queueName, string hostName, int expected)
    {
        int count = 0;
        System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
        var factory = new ConnectionFactory() { HostName = hostName };
        factory.UserName = "rabbit";
        factory.Password = "password";
        using (var connection = factory.CreateConnection())
        {
            using (var channel = connection.CreateModel())
            {
                channel.QueueDeclare(queueName, true, false, false, null);
                channel.BasicQos(0, 1, false);
                var consumer = new QueueingBasicConsumer(channel);
                channel.BasicConsume(queueName, false, consumer);
                sw.Start();
                while (count < expected)
                {
                    BasicDeliverEventArgs ea = consumer.Queue.Dequeue();
                    var body = ea.Body;
                    var message = Encoding.UTF8.GetString(body);
                    channel.BasicAck(ea.DeliveryTag, false);
                    ++count;
                }
                sw.Stop();
            }
        }
        Console.WriteLine(" [x] {0} messages dequeued in time = {1} ms", count, sw.ElapsedMilliseconds);
    }

    static void Main(string[] args)
    {
        string queueName = "hello_three";
        Console.WriteLine("Tell me the remote server address: ");
        string ipaddr = Console.ReadLine();

        //execute the following commands to prepare the comming testings
        //rabbitmqctl add_user rabbit password
        //rabbitmqctl set_permissions rabbit ".*" ".*" ".*"

        //a string having 200 chars
        string s = "SocketPro is a world-leading package of secured communication software components written with request batching, asynchrony and parallel computation in mind. It offers superior performance and scalabi";
        Enqueue(queueName, ipaddr, s, 1000000);
        Dequeue(queueName, ipaddr, 1000000);

        string s1024 = "";
        for (int n = 0; n < 6; ++n)
        {
            s1024 += s;
        }
        s1024 = s1024.Substring(0, 1024);
        Enqueue(queueName, ipaddr, s1024, 1000000);
        Dequeue(queueName, ipaddr, 1000000);

        string s10240 = "";
        for (int n = 0; n < 10; ++n)
        {
            s10240 += s1024;
        }
        Enqueue(queueName, ipaddr, s10240, 100000);
        Dequeue(queueName, ipaddr, 100000);

        Console.WriteLine("Press key ENTER to shutdown the application ......");
        Console.ReadLine();
    }
}

