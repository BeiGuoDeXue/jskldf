#ifndef __HTTP_H__
#define __HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif
#define HTTP_DEFAULT_PORT 80

int http_post_str(const char *url);
int http_post_file(const char *url, const char *filename);
int http_get(const char *url);

#ifdef __cplusplus
}
#endif

#endif // __HTTP_H__