#include "std.h"
#include "web_work.h"

int main(int argc, char* argv[])
{
    try
    {
        // ������·��
        filesystem::path MainPath(argv[0]);
        string sMPath = MainPath.branch_path().string();
        // ��ʽ��·���ַ���
        sMPath = CUtilFunc::FormatPath(sMPath);
        // ��������ʵ��
        CWebWork WebWork(sMPath.c_str());
        // ����asio�඼��Ҫio_service����
        asio::io_service iosev;
        // �����˿�
        asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 9100);
        asio::ip::tcp::acceptor acceptor(iosev, ep);
        // ѭ���ȴ��ͻ�������
        cout << "begin" << endl;
        for(;;)
        {
            try
            {
                // socket����
                asio::ip::tcp::socket socket(iosev);
                // �ȴ�ֱ���ͻ������ӽ���
                acceptor.accept(socket);
                // �õ��ͻ���������Ϣ
                string request;
                char buf[100] = {0};
                while (socket.read_some(asio::buffer(buf)) == 100)    // �����ж�����Ľ�β��\r\n\r\n��
                {
                    if (buf[0] == 0) break;
                    request += buf;
                }
                cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
                cout << request.c_str() << endl;
                cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
                // ��������
                string response = WebWork.Work(request.c_str());
                // ��ͻ��˷�����Ϣ
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
