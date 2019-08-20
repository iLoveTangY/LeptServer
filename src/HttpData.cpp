//
// Created by tang on 19-6-27.
//

#include <sys/epoll.h>
#include <Utils.h>
#include <Logger.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "HttpData.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"

static char favicon[555] = {
        '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',
        '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
        '\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10',
        '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
        'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X',
        't', 'S', 'o', 'f', 't', 'w', 'a', 'r',
        'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
        'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a',
        'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
        '\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA',
        '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
        '\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm',
        '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
        'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8',
        'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
        'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4',
        'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
        'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB',
        'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
        'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v',
        'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
        '\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5',
        '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
        '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
        '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
        '\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r',
        '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
        '\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
        '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
        'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R',
        '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
        '\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0',
        '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
        'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11',
        '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
        '3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4',
        '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
        '\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3',
        '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
        '\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB',
        '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
        'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6',
        '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
        '\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f',
        'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
        'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F',
        '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
        '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
        '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
        '\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F',
        'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
        '\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A',
        '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
        '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
        '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
        '\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB',
        '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
        '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4',
        '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
        '\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91',
        '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
        'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N',
        'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
        '\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u',
        '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
        '\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
        '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
        '\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0',
        '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE',
        'B', '\x60', '\x82',
};

namespace lept_server
{
    pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
    std::unordered_map<std::string, std::string> MimeType::mime;


    const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
    const int DEFAULT_EXPIRED_TIME = 2000; // ms
    const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000; // ms


    HttpData::HttpData(EventLoop *loop, int connfd)
            : loop_(loop),
              channel_(new Channel(loop, connfd)),
              fd_(connfd),
              error_(false),
              connection_state_(H_CONNECTED),
              method_(METHOD_GET),
              http_version_(HTTP_11),
              now_read_pos_(0),
              process_state_(STATE_PARSE_URI),
              parse_state_(H_START),
              keep_alive_(false)
    {
        // HttpData管控的都是Acceptfd，这里要设置好这些listenfd的回调函数
        channel_->set_read_handler(std::bind(&HttpData::handle_read, this));
        channel_->set_write_handler(std::bind(&HttpData::handle_write, this));
        channel_->set_connection_handler(std::bind(&HttpData::handle_connection, this));
    }

    void HttpData::reset()
    {
        file_name_.clear();
        path_.clear();
        now_read_pos_ = 0;
        process_state_ = STATE_PARSE_URI;
        parse_state_ = H_START;
        headers_.clear();
        if (timer_.lock())
        {
            std::shared_ptr<TimerNode> my_timer(timer_.lock());
            my_timer->clear_data();
            timer_.reset();
        }
    }

    void HttpData::seprate_timer()
    {
        if (timer_.lock())
        {
            std::shared_ptr<TimerNode> my_timer(timer_.lock());
            my_timer->clear_data();
            timer_.reset();
        }
    }

    void HttpData::handle_close()
    {
        connection_state_ = H_DISCONNECTED;
        std::shared_ptr<HttpData> guard(shared_from_this());
        loop_->remove_from_poller(channel_);
    }

    void HttpData::new_event()
    {
        channel_->set_events(DEFAULT_EVENT);
        loop_->add_to_poller(channel_, DEFAULT_EXPIRED_TIME);
    }

    void HttpData::handle_read()
    {
        auto events = channel_->get_events();
        do
        {
            if (connection_state_ == H_DISCONNECTING)
            {
                in_buffer_.clear();
                break;
            }
            bool zero = false;
            auto read_num = readn(fd_, in_buffer_, zero);
            LOG_DEBUG << "Request: " << in_buffer_;

            if (read_num < 0)
            {
                LOG_DEBUG << "Bad Request\n";
                error_ = true;
                handle_error(fd_, 400, "Bad Request");
            }
            else if (zero)
            {
                // 有请求出现但是没有读到数据，最可能是对端关闭，直接按照关闭处理
                connection_state_ = H_DISCONNECTING;
                if (read_num == 0)
                {
                    break;
                }
            }

            if (process_state_ == STATE_PARSE_URI)
            {
                URIState flag = this->parse_uri();
                if (flag == PARSE_URI_AGAIN)  // URI请求行未接收完全
                    break;
                else if (flag == PARSE_URI_ERROR)
                {
                    LOG_DEBUG << "parse fd_ = " << fd_ << " URI ERROR\n" << in_buffer_;
                    in_buffer_.clear();
                    error_ = true;
                    handle_error(fd_, 400, "Bad Request");
                    break;
                }
                else
                    process_state_ = STATE_PARSE_HEADERS;
            }

            if (process_state_ == STATE_PARSE_HEADERS)
            {
                auto flag = this->parse_headers();
                if (flag == PARSE_HEADER_AGAIN)
                    break;
                else if (flag == PARSE_HEADER_ERROR)
                {
                    LOG_DEBUG << "parse fd_ = " << fd_ << " HEADER ERROR\n" << in_buffer_;
                    error_ = true;
                    handle_error(fd_, 400, "Bad Request");
                    break;
                }

                if (method_ == METHOD_POST)
                    process_state_ = STATE_RECV_BODY;
                else
                    process_state_ = STATE_ANALYSIS;
            }

            if (process_state_ == STATE_RECV_BODY)
            {
                int content_len;
                if (headers_.find("Content-length") != headers_.end())
                    content_len = stoi(headers_["Content-length"]);
                else
                {
                    error_ = true;
                    LOG_DEBUG << "LACK of Argument(Content-length)\n";
                    handle_error(fd_, 400, "Bad Request: Lack of argument (Content-length)");
                    break;
                }
                if (static_cast<int>(in_buffer_.size()) < content_len)
                    break;
                process_state_ = STATE_ANALYSIS;
            }

            if (process_state_ == STATE_ANALYSIS)
            {
                auto flag = this->analysis_request();
                if (flag == ANALYSIS_SUCCESS)
                {
                    process_state_ = STATE_FINISH;
                    break;
                }
                else
                {
                    error_ = true;
                    break;
                }
            }
        } while (false);

        if (!error_)
        {
            if (!out_buffer_.empty())
                handle_write();
            // error_ 可能会改变
            if (!error_ && process_state_ == STATE_FINISH)
            {
                this->reset();
                if (!in_buffer_.empty())
                    if (connection_state_ != H_DISCONNECTING)
                        handle_read();
            }
            else if (!error_ && connection_state_ != H_DISCONNECTED)
                channel_->set_events(events | EPOLLIN);
        }
    }

    URIState HttpData::parse_uri()
    {
        std::string &str = in_buffer_;
//        std::string cop = str;

        // 读到完整的请求行之后才开始解析
        auto pos = str.find('\r', now_read_pos_);
        if (pos < 0)
            return PARSE_URI_AGAIN;

        // 去掉请求行所占的空间
        std::string request_line = str.substr(0, pos);
        if (str.size() > pos + 1)
            str = str.substr(pos + 1);
        else
            str.clear();

        // method
        auto pos_GET = request_line.find("GET");
        auto pos_POST = request_line.find("POST");
        auto pos_HEAD = request_line.find("HEAD");

        if (pos_GET >= 0)
        {
            pos = pos_GET;
            method_ = METHOD_GET;
        }
        else if (pos_POST >= 0)
        {
            pos = pos_POST;
            method_ = METHOD_POST;
        }
        else if (pos_HEAD >= 0)
        {
            pos = pos_HEAD;
            method_ = METHOD_HEAD;
        }
        else
            return PARSE_URI_ERROR;

        // 解析请求文件名
        pos = request_line.find('/', pos);
        if (pos < 0)
        {
            file_name_ = "index.html";
            http_version_ = HTTP_11;
            return PARSE_URI_SUCCESS;
        }
        else
        {
            auto _pos = request_line.find(' ', pos);
            if (_pos < 0)
                return PARSE_URI_ERROR;
            else
            {
                if (_pos - pos > 1)
                {
                    file_name_ = request_line.substr(pos + 1, _pos - pos - 1);
                    auto __pos = file_name_.find('?');
                    if (__pos >= 0)
                        file_name_ = file_name_.substr(0, __pos);
                }
                else
                    file_name_ = "index.html";
            }
            pos = _pos;
        }

        LOG_INFO << "request file name: " << file_name_;

        // 解析http版本号
        pos = request_line.find('/', pos);
        if (pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if (request_line.size() - pos <= 3)
                return PARSE_URI_ERROR;
            else
            {
                auto ver = request_line.substr(pos + 1, 3);
                if (ver == "1.0")
                    http_version_ = HTTP_10;
                else if (ver == "1.1")
                    http_version_ = HTTP_11;
                else
                    return PARSE_URI_ERROR;
            }
        }
        return PARSE_URI_SUCCESS;
    }

    HeaderState HttpData::parse_headers()
    {
        std::string &str = in_buffer_;
        int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
        int now_read_line_begin = 0;
        bool not_finish = true;
        size_t i = 0;
        for (; i < str.size() && not_finish; ++i)
        {
            switch (parse_state_)
            {
                case H_START:
                {
                    if (str[i] == '\n' || str[i] == '\r')
                        break;
                    parse_state_ = H_KEY;
                    key_start = static_cast<int>(i);
                    now_read_line_begin = static_cast<int>(i);
                    break;
                }
                case H_KEY:
                {
                    if (str[i] == ':')
                    {
                        key_end = static_cast<int>(i);
                        if (key_end - key_start <= 0)
                            return PARSE_HEADER_ERROR;
                        parse_state_ = H_COLON;
                    }
                    else if (str[i] == '\n' || str[i] == '\r')
                        return PARSE_HEADER_ERROR;
                    break;
                }
                case H_COLON:
                {
                    if (str[i] == ' ')
                    {
                        parse_state_ = H_SPACES_AFTER_COLON;
                    }
                    else
                        return PARSE_HEADER_ERROR;
                    break;
                }
                case H_SPACES_AFTER_COLON:
                {
                    parse_state_ = H_VALUE;
                    value_start = static_cast<int>(i);
                    break;
                }
                case H_VALUE:
                {
                    if (str[i] == '\r')
                    {
                        parse_state_ = H_CR;
                        value_end = static_cast<int>(i);
                        if (value_end - value_start <= 0)
                            return PARSE_HEADER_ERROR;
                    }
                    else if (i - value_start > 255)
                        return PARSE_HEADER_ERROR;
                    break;
                }
                case H_CR:
                {
                    if (str[i] == '\n')
                    {
                        parse_state_ = H_LF;
                        std::string key(str.begin() + key_start, str.begin() + key_end);
                        std::string value(str.begin() + value_start, str.begin() + value_end);
                        headers_[key] = value;
                        now_read_line_begin = static_cast<int>(i);
                    }
                    else
                        return PARSE_HEADER_ERROR;
                    break;
                }
                case H_LF:
                {
                    if (str[i] == '\r')
                    {
                        parse_state_ = H_END_CR;
                    }
                    else
                    {
                        key_start = static_cast<int>(i);
                        parse_state_ = H_KEY;
                    }
                    break;
                }
                case H_END_CR:
                {
                    if (str[i] == '\n')
                    {
                        parse_state_ = H_END_LF;
                    }
                    else
                        return PARSE_HEADER_ERROR;
                    break;
                }
                case H_END_LF:
                {
                    not_finish = false;
                    key_start = static_cast<int>(i);
                    now_read_line_begin = static_cast<int>(i);
                    break;
                }
            }
        }
        if (parse_state_ == H_END_LF)
        {
            str = str.substr(i);
            return PARSE_HEADER_SUCCESS;
        }
        str = str.substr(static_cast<unsigned long>(now_read_line_begin));
        return PARSE_HEADER_AGAIN;
    }

    AnalysisState HttpData::analysis_request()
    {
        if (method_ == METHOD_POST)
        {

        }
        else if (method_ == METHOD_GET || method_ == METHOD_HEAD)
        {
            std::string header;
            header += "HTTP/1.1 200 OK\r\n";
            if (headers_.find("Connection") != headers_.end() &&
                (headers_["Connection"] == "Keep-Alive" || headers_["Connection"] == "keep-alive"))
            {
                keep_alive_ = true;
                header += std::string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +
                          std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
            }
            auto dot_pos = file_name_.find('.');
            std::string filetype;
            if (dot_pos < 0)
                filetype = MimeType::getMime("default");
            else
                filetype = MimeType::getMime(file_name_.substr(dot_pos));


            // echo test
            if (file_name_ == "hello")
            {
                out_buffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
                return ANALYSIS_SUCCESS;
            }
            if (file_name_ == "favicon.ico")
            {
                header += "Content-Type: image/png\r\n";
                header += "Content-Length: " + std::to_string(sizeof favicon) + "\r\n";
                header += "Server: Lept Web Server\r\n";

                header += "\r\n";
                out_buffer_ += header;
                out_buffer_ += std::string(favicon, favicon + sizeof favicon);;
                return ANALYSIS_SUCCESS;
            }

            struct stat sbuf{};
            if (stat(file_name_.c_str(), &sbuf) < 0)
            {
                header.clear();
                handle_error(fd_, 404, "Not Found!");
                return ANALYSIS_ERROR;
            }
            header += "Content-Type: " + filetype + "\r\n";
            header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
            header += "Server: Lept Web Server\r\n";
            // 头部结束
            header += "\r\n";
            out_buffer_ += header;

            if (method_ == METHOD_HEAD)
                return ANALYSIS_SUCCESS;

            int src_fd = open(file_name_.c_str(), O_RDONLY, 0);
            if (src_fd < 0)
            {
                out_buffer_.clear();
                handle_error(fd_, 404, "Not Found!");
                return ANALYSIS_ERROR;
            }
            void *mmap_ret = mmap(nullptr, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
            close(src_fd);
            if (mmap_ret == (void *) -1)
            {
                munmap(mmap_ret, sbuf.st_size);
                out_buffer_.clear();
                handle_error(fd_, 404, "Not Found!");
                return ANALYSIS_ERROR;
            }
            char *src_addr = static_cast<char *>(mmap_ret);
            out_buffer_ += std::string(src_addr, src_addr + sbuf.st_size);;
            munmap(mmap_ret, sbuf.st_size);
            return ANALYSIS_SUCCESS;
        }
        return ANALYSIS_ERROR;
    }

    void HttpData::handle_write()
    {
        if (!error_ && connection_state_ != H_DISCONNECTED)
        {
            auto events = channel_->get_events();
            if (writen(fd_, out_buffer_) < 0)
            {
                LOG_DEBUG << "writen error\n";
                channel_->set_events(0);
                error_ = true;
            }
            if (!out_buffer_.empty())
                channel_->set_events(events | EPOLLOUT);
        }
    }

    void HttpData::handle_connection()
    {
        seprate_timer();
        auto events = channel_->get_events();
        if (!error_ && connection_state_ == H_CONNECTED)
        {
            if (events != 0)
            {
                int timeout = DEFAULT_EXPIRED_TIME;
                if (keep_alive_)
                    timeout = DEFAULT_KEEP_ALIVE_TIME;
                if ((events & EPOLLIN) && (events & EPOLLOUT))
                {
                    events = __uint32_t(0);
                    events |= EPOLLOUT;
                    channel_->set_events(events);
                }
                events |= EPOLLET;
                channel_->set_events(events);
                loop_->update_poller(channel_, timeout);

            }
            else if (keep_alive_)
            {
                events |= (EPOLLIN | EPOLLET);
                channel_->set_events(events);
                int timeout = DEFAULT_KEEP_ALIVE_TIME;
                loop_->update_poller(channel_, timeout);
            }
            else
            {
                events |= (EPOLLIN | EPOLLET);
                channel_->set_events(events);
                int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
                loop_->update_poller(channel_, timeout);
            }
        }
        else if (!error_ && connection_state_ == H_DISCONNECTING && (events & EPOLLOUT))
        {
            events = (EPOLLOUT | EPOLLET);
            channel_->set_events(events);
        }
        else
        {
            loop_->run_in_loop(std::bind(&HttpData::handle_close, shared_from_this()));
        }
    }

    void HttpData::handle_error(int fd, int err_num, std::string short_message)
    {
        short_message = " " + short_message;
        char send_buff[4096];
        std::string body_buff, header_buff;
        body_buff += "<html><title>哎~出错了</title>";
        body_buff += "<body bgcolor=\"ffffff\">";
        body_buff += std::to_string(err_num) + short_message;
        body_buff += "<hr><em> Lept Web Server</em>\n</body></html>";

        header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_message + "\r\n";
        header_buff += "Content-Type: text/html\r\n";
        header_buff += "Connection: Close\r\n";
        header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
        header_buff += "Server: Lept Web Server\r\n";;
        header_buff += "\r\n";
        // 错误处理不考虑writen不完的情况
        sprintf(send_buff, "%s", header_buff.c_str());
        writen(fd, send_buff, strlen(send_buff));
        sprintf(send_buff, "%s", body_buff.c_str());
        writen(fd, send_buff, strlen(send_buff));
    }
}

void lept_server::MimeType::init()
{
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

std::string lept_server::MimeType::getMime(const std::string &suffix)
{
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}
