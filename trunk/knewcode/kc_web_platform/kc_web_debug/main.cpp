#include "std.h"
#include "web_work.h"

int main(int argc, char* argv[])
{
    try
    {
        // 主程序路径
        filesystem::path MainPath(argv[0]);
        string sMPath = MainPath.branch_path().string();
        // 格式化路径字符串
        sMPath = CUtilFunc::FormatPath(sMPath);
        // 主处理类实例
        CWebWork WebWork(sMPath.c_str());
        // 所有asio类都需要io_service对象
        asio::io_service iosev;
        // 监听端口
        asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 9100);
        asio::ip::tcp::acceptor acceptor(iosev, ep);
        // 循环等待客户端连接
        cout << "begin" << endl;
        for(;;)
        {
            try
            {
                // socket对象
                asio::ip::tcp::socket socket(iosev);
                // 等待直到客户端连接进来
                acceptor.accept(socket);
                // 得到客户端请求信息
                string request;
                char buf[100] = {0};
                while (socket.read_some(asio::buffer(buf)) == 100)    // 还需判断请求的结尾“\r\n\r\n”
                {
                    if (buf[0] == 0) break;
                    request += buf;
                }
                cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
                cout << request.c_str() << endl;
                cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
                // 处理请求
                string response = WebWork.Work(request.c_str());
                // 向客户端发送信息
                socket.write_some(asio::buffer(response));
                cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
                cout << response << endl;
                cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
            }
            catch(...)
            {
                cout << "Unknow error." << endl;
            }
        }
    }
    catch(...)
    {
        cout << "Unknow error." << endl;
    }
    //pause();
    return 0;
}
