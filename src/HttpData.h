//
// Created by tang on 19-6-27.
//

#ifndef LEPTSERVER_HTTPDATA_H
#define LEPTSERVER_HTTPDATA_H

#include <memory>
#include <unordered_map>
#include <unistd.h>
#include <iostream>

namespace lept_server
{
    class EventLoop;

    class Channel;

    class TimerNode;

    enum ProcessState
    {
        STATE_PARSE_URI = 1,
        STATE_PARSE_HEADERS,
        STATE_RECV_BODY,
        STATE_ANALYSIS,
        STATE_FINISH
    };

    enum URIState
    {
        PARSE_URI_AGAIN = 1,
        PARSE_URI_ERROR,
        PARSE_URI_SUCCESS,
    };

    enum HeaderState
    {
        PARSE_HEADER_SUCCESS = 1,
        PARSE_HEADER_AGAIN,
        PARSE_HEADER_ERROR
    };

    enum AnalysisState
    {
        ANALYSIS_SUCCESS = 1,
        ANALYSIS_ERROR
    };

    enum ParseState
    {
        H_START = 0,
        H_KEY,
        H_COLON,
        H_SPACES_AFTER_COLON,
        H_VALUE,
        H_CR,
        H_LF,
        H_END_CR,
        H_END_LF
    };

    enum ConnectionState
    {
        H_CONNECTED = 0,
        H_DISCONNECTING,
        H_DISCONNECTED
    };

    enum HttpMethod
    {
        METHOD_POST = 1,
        METHOD_GET,
        METHOD_HEAD
    };

    enum HttpVersion
    {
        HTTP_10 = 1,
        HTTP_11
    };

    class MimeType
    {
    private:
        static void init();

        static std::unordered_map<std::string, std::string> mime;

    public:
        static std::string getMime(const std::string &suffix);

    private:
        static pthread_once_t once_control;
    };

    // 搞清楚HttpData的生命周期
    class HttpData : public std::enable_shared_from_this<HttpData>
    {
    public:
        HttpData(EventLoop *loop, int connfd);

        ~HttpData()
        {
            close(fd_);
        }

        void reset();

        void seprate_timer();

        void link_timer(const std::shared_ptr<TimerNode> &timer)
        {
            timer_ = timer;
        }

        std::shared_ptr<Channel> get_channel()
        {
            return channel_;
        }

        void handle_close();

        void new_event();

    private:
        EventLoop *loop_;
        std::shared_ptr<Channel> channel_;
        int fd_;
        std::string in_buffer_;
        std::string out_buffer_;
        bool error_;

        ConnectionState connection_state_;
        HttpMethod method_;
        HttpVersion http_version_;
        std::string file_name_;
        std::string path_;
        std::string::size_type now_read_pos_;
        ProcessState process_state_;
        ParseState parse_state_;
        bool keep_alive_;
        std::unordered_map<std::string, std::string> headers_;
        std::weak_ptr<TimerNode> timer_;

        // 以下是此fd的四个回调函数
        void handle_read();
        void handle_write();
        void handle_connection();
        void handle_error(int fd, int err_num, std::string short_message);

        URIState  parse_uri();
        HeaderState parse_headers();
        AnalysisState analysis_request();

    };
}


#endif //LEPTSERVER_HTTPDATA_H
