using System;
using System.Threading.Tasks;
using System.Windows.Forms;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace win_async
{
    public partial class AsyncTest : Form
    {
        public AsyncTest()
        {
            InitializeComponent();
        }

        Task<CScopeUQueue> GetTask()
        {
            HelloWorld hw = m_spHw.AsyncHandlers[0];
            return hw.Async(hwConst.idSayHelloHelloWorld, "Jack", "Smith");
        }

        Task<CScopeUQueue> GetTasksInBatch()
        {
            HelloWorld hw = m_spHw.AsyncHandlers[0];
            bool ok = hw.SendRequest(hwConst.idSleepHelloWorld, 5000, (ar) => { });
            Task<CScopeUQueue> task = hw.Async(hwConst.idSayHelloHelloWorld, "Jone", "Don");
            return task;
        }

        CSocketPool<HelloWorld> m_spHw = new CSocketPool<HelloWorld>(true);

        private void async_Load(object sender, EventArgs e)
        {
            CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "MyUserId", "MyPassword");
            btnTest.Enabled = m_spHw.StartSocketPool(cc, 1);
        }

        private void AsyncTest_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
        {
            //Crash may happen randomly if there is a request in processing! You can use the following logic to avoid the issue.
            if (m_spHw.Sockets[0].CountOfRequestsInQueue > 0)
            {
                //Optionally force to wait until all requests are processed
                //m_spHw.Sockets[0].WaitAll();

                //explicitly shutdown pool and ignore all requests in processing
                m_spHw.ShutdownPool();
            }
        }

        private async void btnTest_Click(object sender, EventArgs e)
        {
            if (m_spHw.ConnectedSockets == 0)
            {
                txtRes.Text = "No connection";
                return;
            }
            btnTest.Enabled = false;
            try
            {
                //execute one request asynchronously
                txtRes.Text = (await GetTask()).Load<string>();

                //execute multiple requests asynchronously in batch
                txtRes.Text = (await GetTasksInBatch()).Load<string>();
                btnTest.Enabled = true;
            }
            catch (Exception err)
            {
                txtRes.Text = err.Message;
            }
        }
    }
}
