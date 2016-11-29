#include "std.h"
#include "net_action.h"

////////////////////////////////////////////////////////////////////////////////
// CKCNetAction¿‡
CKCNetAction::CKCNetAction(IActionData& act, asio::io_service& ns) : m_act(act.GetActRoot()), m_io_serv(ns)
{
}

// ªÒ»°Õ¯“≥
const char* CKCNetAction::SyncHttpGet(const char* domain, const char* path, int port)
{
    const char* pResult = nullptr;
    try
    {
        // Get a list of endpoints corresponding to the server name.
        asio::ip::tcp::resolver resolver(m_io_serv);
        asio::ip::tcp::resolver::query query(domain, lexical_cast<string>(port));
        asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        asio::ip::tcp::socket socket(m_io_serv);
        asio::connect(socket, endpoint_iterator);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        asio::streambuf request;
        ostream request_stream(&request);
        request_stream << "GET " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << domain << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        istream response_stream(&response);
        string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        string status_message;
        getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            throw runtime_error("Invalid response\r\n");
        if (status_code != 200)
            throw runtime_error("Response returned with status code - " + lexical_cast<string>(status_code));

        // Read the response headers, which are terminated by a blank line.
        asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        string header;
        while (getline(response_stream, header) && header != "\r");

        // Read until EOF, writing data to output as we go.
        std::stringstream ssrespond;
        system::error_code error;
        while (asio::read(socket, response, asio::transfer_at_least(1), error))
            ssrespond << &response;
        ssrespond >> m_SyncHttpGet;
        if (error != asio::error::eof)
            throw system::system_error(error);
        pResult = m_SyncHttpGet.c_str();
    }
    catch (std::exception& ex)
    {
        string sClientIP = m_act.GetRequestRespond().GetClientIP();
        string sError = "[" + sClientIP + "]  " + domain + "  " + lexical_cast<string>(port) + "  " + path + "\r\n" + ex.what();
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, typeid(ex).name());
        pResult = nullptr;
    }
    return pResult;
}
