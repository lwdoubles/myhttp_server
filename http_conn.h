#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;		//文件名最大长度
    static const int READ_BUFFER_SIZE = 2048;	//读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;	//写缓冲区大小
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };	//仅支持GET方法
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };	//解析客户请求时，服务器所处状态
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };	//HTTP处理可能结果
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };	//行的解析状态

public:
    http_conn(){}
    ~http_conn(){}

public:
    void init( int sockfd, const sockaddr_in& addr );	//初始化新接受的连接
    void close_conn( bool real_close = true );			//关闭连接
    void process();										//处理客户请求
    bool read();										//非阻塞读操作
    bool write();										//非阻塞写操作

private:
	//初始化连接，单例模式
    void init();
	//解析http请求
    HTTP_CODE process_read();
	//填充http应答
    bool process_write( HTTP_CODE ret );

	//下面的函数用于http应答
    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    void unmap();
	//以下都是process_write调用以填充http应答
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();

public:
	
    static int m_epollfd;
	//统计用户数量
    static int m_user_count;

private:
	//客户的socket
    int m_sockfd;
    sockaddr_in m_address;

    char m_read_buf[ READ_BUFFER_SIZE ];
	//标识读缓冲中已经读入的客户数据的最后一个字节的下一个位置
    int m_read_idx;
	//当前正在分析的字符在读缓冲区中的位置
    int m_checked_idx;
	//当前正在解析的行的起始位置
    int m_start_line;
    char m_write_buf[ WRITE_BUFFER_SIZE ];
	//写缓冲区中待发送的字节数
    int m_write_idx;
	//服务器当前所处在的状态
    CHECK_STATE m_check_state;
	//请求方法
    METHOD m_method;
	//客户请求的目标文件的完整路径名，相当于doc_root + m_url
    char m_real_file[ FILENAME_LEN ];
	//客户请求的文件名
    char* m_url;
    char* m_version;
	//主机名
    char* m_host;
	//http请求的消息体的总长度
    int m_content_length;
	//http是否要求保持连接
    bool m_linger;
	//客户请求的目标文件被mmap到内存中的起始位置
    char* m_file_address;
	//目标文件的状态，通过它我们可以判断文件是否存在、是否为目录、是否可读等等
    struct stat m_file_stat;
    struct iovec m_iv[2];
	//被写内存块的数量
    int m_iv_count;
};

#endif
