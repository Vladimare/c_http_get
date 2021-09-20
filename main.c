#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <arpa/inet.h> // inet_addr

void error(const char *msg) { perror(msg); exit(0); }

// curl http://127.0.0.1:9090/status
//#define PORT 9090
//unsigned char ip[4] = { 127, 0, 0, 1 };
#define MAXMSGLEN 1024

int get_value_for_key(const char *json, const char *key)
{
    char *key_ptr = strstr(json, key);
    if (key_ptr == NULL) error("ERROR no key in json");

    char *value_ptr = (key_ptr + strlen(key) + 3); // strlen("\":\"") == 3 -- delimeter length
    char *endp = strchr(value_ptr, '"');
    char avalue[10];
    int numlen = endp-value_ptr;
    for (int i = 0; i < numlen; i++)
        avalue[i] = value_ptr[i];
    avalue[numlen] = '\0';
    return atoi(avalue);
}

int get_status_json(const char *addr, int port, char *request, char *response)
{
    char message[MAXMSGLEN];
    sprintf(message, "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", 
        request, addr);
    char resp[MAXMSGLEN] = { 0 };
    struct sockaddr_in server;
    int sockfd, n;

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd < 0) error("ERROR opening socket");
    if (sockfd < 0) return -1;

    /* fill in the structure */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(addr);
//    server.sin_addr.s_addr = *(unsigned long *)&ip;

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
//        error("ERROR connecting");
        return -1;

    /* send the request */
    n = write(sockfd, message, strlen(message));
    if (n < 0)
//        error("ERROR writing message to socket");
        return -1;

    /* receive the response */
    n = read(sockfd, resp, MAXMSGLEN);
    if (n < 0)
//        error("ERROR reading response from socket");
        return -1;

    /* close the socket */
    close(sockfd);

    char *Ok = "HTTP/1.1 200 OK";
    int ret = memcmp(Ok, resp, strlen(Ok)-1);

    char *body = strchr(resp, '{');
    if (body == NULL) return -1;

    strcpy(response, body);

    return ret;
}

int main(int argc,char *argv[])
{
    char json[MAXMSGLEN];
    int value;

    int ret = get_status_json("127.0.0.1", 9090, "status", json);
    if (ret != 0) error("ERROR gettin json");
    printf("Body:\n%s\n", json);

    value = get_value_for_key(json, "master_offset");

    printf("\nmaster_offset value:\n%d\n", value);

    return 0;
}
