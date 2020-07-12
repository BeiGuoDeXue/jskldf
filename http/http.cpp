#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>
#include "common.h"

#define BUFFER_SIZE 1024
#define HTTP_POST "POST /%s HTTP/1.1\r\n"         \
                  "Content-Type:%s%s\r\n"         \
                  "cache-control:no-cache\r\n"    \
                  "User-Agent:smartcamera\r\n"    \
                  "Accept:*/*\r\n"                \
                  "Host:%s:%d\r\n"                \
                  "content-length:%ld\r\n"        \
                  "Connection:keep-alive\r\n\r\n" \
                  "%s"

#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"
int changespace(char *src, char *desc)
{
    while (*src)
    {
        if (*src != ' ')
        {
            *desc = *src;
            desc++;
        }
        else
        {
            strcpy(desc, "%20");
            desc = desc + 3;
        }
        src++;
    }
    *desc = '\0';
    return 0;
}
static int http_tcpclient_create(const char *host, int port)
{
    int trycount = 5;
    struct hostent *he;
    struct sockaddr_in server_addr;
    // struct addrinfo   *curr, *answer, hint;
    int socket_fd = 0;

    // bzero(&hint, sizeof(hint));
    // hint.ai_family = AF_INET;
    // hint.ai_socktype = SOCK_STREAM;

//     int ret = getaddrinfo(host, NULL, &hint, &answer);
//     if (ret != 0)
//     {
//         ERR("getaddrinfo: %s\n", gai_strerror(ret));
//         goto exit;
//     }
//     // char ipstr[16];
//     for (curr = answer; curr != NULL; curr = curr->ai_next)
//     {
//         server_addr.sin_family = AF_INET;
//         server_addr.sin_port = htons(port);
//         server_addr.sin_addr = (((struct sockaddr_in *)(curr->ai_addr))->sin_addr);

//         if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
//         {
//             ERR("socket erro!\r\n");
//             goto exit;
//             return -1;
//         }

//         if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
//         {
//             ERR("connect erro!\r\n");
//             goto exit;
//             return -1;
//         }
//         DBG("socket_fd:%d!\r\n", socket_fd);
//     }

// exit:
//     if (answer != NULL)
//         freeaddrinfo(answer);
//     if (curr != NULL)
//         freeaddrinfo(curr);

    // DBG("tcp client create!\r\n");

    do
    {
        if ((he = gethostbyname(host)) == NULL)
        {
            usleep(100 * 1000);
        }
    } while ((NULL == he) && (trycount-- > 0));
    if (he == NULL)
    {
        ERR("gethostbyname erro!\r\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    // server_addr.sin_addr =(((struct sockaddr_in *)(curr->ai_addr))->sin_addr);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ERR("socket erro!\r\n");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        ERR("connect erro!\r\n");
        return -1;
    }
    DBG("socket_fd:%d!\r\n", socket_fd);

    return socket_fd;
}

static void http_tcpclient_close(int socket)
{
    close(socket);
}

static int http_parse_url(const char *url, char *host, char *file, int *port)
{
    char *ptr1, *ptr2;
    int len = 0;
    if (!url || !host || !file || !port)
    {
        return -1;
    }

    ptr1 = (char *)url;

    if (!strncmp(ptr1, "http://", strlen("http://")))
    {
        ptr1 += strlen("http://");
    }
    else
    {
        return -1;
    }

    ptr2 = strchr(ptr1, '/');
    if (ptr2)
    {
        len = strlen(ptr1) - strlen(ptr2);
        memcpy(host, ptr1, len);
        host[len] = '\0';
        if (*(ptr2 + 1))
        {
            memcpy(file, ptr2 + 1, strlen(ptr2) - 1);
            file[strlen(ptr2) - 1] = '\0';
        }
    }
    else
    {
        memcpy(host, ptr1, strlen(ptr1));
        host[strlen(ptr1)] = '\0';
    }
    //get host and ip
    ptr1 = strchr(host, ':');
    if (ptr1)
    {
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    }
    else
    {
        *port = HTTP_DEFAULT_PORT;
    }

    return 0;
}

static int http_tcpclient_recv(int socket, char *lpbuff)
{
    int recvnum = 0;
    recvnum = recv(socket, lpbuff, BUFFER_SIZE * 4, 0);
    return recvnum;
}

static int http_tcpclient_send(int socket, char *buff, int size)
{
    int sent = 0, tmpres = 0;

    while (sent < size)
    {
        tmpres = send(socket, buff + sent, size - sent, 0);
        if (tmpres == -1)
        {
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

static int http_parse_result(const char *lpbuf)
{
    char *ptmp = NULL;
    char *response = NULL;
    ptmp = (char *)strstr(lpbuf, "HTTP/1.1");
    if (!ptmp)
    {
        return -1;
    }
    if (atoi(ptmp + 9) != 200)
    {
        DBG("result:\n%s\n", lpbuf);
        return -1;
    }

    ptmp = (char *)strstr(lpbuf, "\r\n\r\n");
    if (!ptmp)
    {
        return -1;
    }

    response = (char *)malloc(strlen(ptmp) + 1);
    if (!response)
    {
        return -1;
    }
    strcpy(response, ptmp + 4);

    DBG("result:%s\r\n", response);

    free(response);

    return 0;
}

int http_post(const char *url, const char *post_str, const char *app_type)
{
    char newurl[200];
    memset(newurl, 0, sizeof(newurl));
    changespace((char *)url, newurl);
    int socket_fd = -1;
    char lpbuf[1024] = {'\0'};
    char host_addr[512] = {'\0'};
    char file[512] = {'\0'};
    int port = 0;
    if (!newurl[0] || !post_str)
    {
        return -1;
    }
    if (http_parse_url(newurl, host_addr, file, &port))
    {
        return -1;
    }

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0)
    {
        return -1;
    }

    sprintf(lpbuf, HTTP_POST, file, app_type, "", host_addr, port, (long)strlen(post_str), post_str);
    DBG("send:%s\r\n", lpbuf);
    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0)
    {
        return -1;
    }

    memset(lpbuf, 0, sizeof(lpbuf));
    if (http_tcpclient_recv(socket_fd, lpbuf) <= 0)
    {
        return -1;
    }
    http_tcpclient_close(socket_fd);
    DBG("return:%s\r\n", lpbuf);
    return http_parse_result(lpbuf);
}

int http_post_file(const char *url, const char *filename)
{
    if (is_file_exist(filename) == 0)
    {
        char newurl[300]={'\0'};
        int fd = -1;
        struct stat sbuf;
        char *ptr = NULL;
        char head[1024] = {'\0'};
        char body[1024] = {'\0'};
        char foot[200] = {'\0'};
        char recv[1024] = {'\0'};
        char host_addr[300];
        char file[1024];
        int socket_fd = -1;
        long len = 0;
        long headlen = 0;
        long bodylen = 0;
        long footlen = 0;
        char uuid[33] = {'\0'};

        execcmd("printf \"%s\" `cat //proc/sys/kernel/random/uuid |sed 's/-//g'`", uuid);

        memset(newurl, 0, sizeof(newurl));
        changespace((char *)url, newurl);

        fd = open(filename, O_RDONLY);
        if (fd < 0)
        {
            ERR("openfile erro!\n");
            return -1;
        }
        if (fstat(fd, &sbuf) < 0)
        {
            ERR("fstat  erro!\n");
            close(fd);
            return -1;
        }

        ptr = (char *)mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

        if (ptr == MAP_FAILED)
        {
            ERR("map file  erro!\n");
            close(fd);
            return -1;
        }
        int port = 0;
        if (!newurl[0])
        {
            ERR("no url!\n");
            return -1;
        }

        if (http_parse_url(newurl, host_addr, file, &port))
        {
            ERR("url erro!\n");
            return -1;
        }

        DBG("newurl:%s\r\nhostaddr:%s \r\nport:%d\r\n", newurl, host_addr, port);
        socket_fd = http_tcpclient_create(host_addr, port);
        if (socket_fd < 0)
        {
            ERR("tcpclien create erro!\n");
            close(fd);
            return -1;
        }

        bodylen = sprintf(body, "--%s\r\n"
                                "Content-Disposition: form-data; name=\"image\"; filename=\"%s\" \r\n"
                                "Content-Type: image/jpeg\r\n\r\n",
                          uuid, get_filename(filename));
        footlen = sprintf(foot, "\r\n--%s--\r\n", uuid);

        len += sbuf.st_size;
        len += bodylen;
        len += footlen;

        headlen = sprintf(head, HTTP_POST, file, "multipart/form-data;boundary=", uuid, host_addr, port, len, "");

        DBG("head:%s\r\n", head);
        DBG("body:%s\r\n", body);
        DBG("foot:%s\r\n", foot);

        if (http_tcpclient_send(socket_fd, head, headlen) < 0)
        {
            ERR("tcpclien send head erro!\n");
            close(socket_fd);
            close(fd);
            return -1;
        }

        // DBG("send head\r\n");
        if (http_tcpclient_send(socket_fd, body, bodylen) < 0)
        {
            ERR("tcpclien send body erro!\n");
            close(socket_fd);
            close(fd);
            return -1;
        }
        // DBG("send body\r\n");
        if (http_tcpclient_send(socket_fd, (char *)ptr, sbuf.st_size) < 0)
        {
            ERR("tcpclien send file erro!%ld\n", sbuf.st_size);
            close(socket_fd);
            close(fd);
            return -1;
        }
        // DBG("send file\r\n");
        if (http_tcpclient_send(socket_fd, foot, footlen) < 0)
        {
            ERR("tcpclien send foot erro!\n");
            close(socket_fd);
            close(fd);
            return -1;
        }
        DBG("send foot\r\n");

        if (ptr)
            munmap((void *)ptr, sbuf.st_size);
        close(fd);

        if (http_tcpclient_recv(socket_fd, recv) <= 0)
        {
            ERR("tcpclien receive erro!\n");
            return -1;
        }
        http_tcpclient_close(socket_fd);

        return http_parse_result(recv);
    }
    return -1;
}
int http_post_str(const char *url)
{
    return http_post(url, "", "application/x - www - form - urlencoded");
}
int http_get(const char *url)
{
    char newurl[200];
    memset(newurl, 0, sizeof(newurl));
    changespace((char *)url, newurl);
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = {'\0'};
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;

    if (!newurl[0])
    {
        return -1;
    }

    if (http_parse_url(newurl, host_addr, file, &port))
    {
        return -1;
    }

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0)
    {
        return -1;
    }

    sprintf(lpbuf, HTTP_GET, file, host_addr, port);

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0)
    {
        return -1;
    }

    if (http_tcpclient_recv(socket_fd, lpbuf) <= 0)
    {
        return -1;
    }
    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
}