/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Jingxia Pang  jp2338                                               *
*          Yanbo Li      yl2556                                               *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "parse.h"
#include "log.h"

#define ECHO_PORT 9999
#define BUF_SIZE 40960
#define MIN_LINE 64
#define MAX_LINE 81920

static char WWW[] = "www";

// Functions clarification
void server_shutdown(int sig);
//FILE *serverlog;
void *get_in_addr(struct sockaddr * sa); /* get sockaddr, IPv4 or IPv6 */
int close_socket(int sock);
void close_sockets(int socks);

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
//        fprintf(serverlog,"Failed closing socket.\n");
        log_write("Failed closing socket.\n", 1111);
        return 1;
    }
    return 0;
}

void close_sockets(int socks){
    printf("Closing all sockets");
    for (int i = 1; i <= socks; i++){
        close_socket(i);
    }
}
void server_shutdown(int ret) {
//    fprintf(serverlog,"Server shutdown\n");
    log_write("Server shutdown\n", 1111);
//    fclose(serverlog);
    log_close();
    exit(ret);
}

/* handle signal */
void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGINT:
            printf("Caught SIGINT, exiting now\n");
//            fprintf(serverlog,"Caught SIGINT, exiting now\n");
            log_write("Caught SIGINT, exiting now\n", 1111);
            exit(0);

        case SIGHUP:
            /* rehash the server */
            break;
        case SIGTERM:
            /* finalize and shutdown the server */
            server_shutdown(EXIT_SUCCESS);
            break;
        default:
            /* unhandled signal */
            fprintf(stderr, "Caught unknown signal");
            return;
    }
}


/* get sockaddr, IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".css"))
        strcpy(filetype, "text/css");
    else if (strstr(filename, ".js"))
        strcpy(filetype, "application/javascript");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/* Server Error Response */
void serve_error(int id, char *errnum, char *shortmsg, char *longmsg){
    // return error message to client
    struct tm tm;
    time_t now;
    char buf[MAX_LINE], body[MAX_LINE], dbuf[MIN_LINE];

    now = time(0);
    tm = *gmtime(&now);
    strftime(dbuf, MIN_LINE, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    // build HTTP response body
    sprintf(body, "<html><title>Lisod Error</title>");
    sprintf(body, "%s<body>\r\n", body);
    sprintf(body, "%sError %s -- %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<br><p>%s</p></body></html>\r\n", body, longmsg);

    // print HTTP response
    sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(buf, "%sDate: %s\r\n", buf, dbuf);
    sprintf(buf, "%sServer: Liso/1.0\r\n", buf);
    sprintf(buf, "%sContent-type: text/html\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, (int)strlen(body));
    send(id, buf, strlen(buf), 0);
    send(id, body, strlen(body), 0);
}

/* Validate uri filename*/
int validate_file(int id, Request *request){
    // TODO: modify parser.c and .h request structure for filename
    struct stat sbuf;
    printf ("stat result: %d\n", stat(request->http_uri, &sbuf));
    if (stat(request->http_uri, &sbuf) < 0)
    {
        // TODO Log("Error: Cann't open file \n");
        printf("File Validation Erorr - 404 Not Found");
        serve_error(id, "404", "Not Found",
                    "Server couldn't find this file");
        return -1;
    }
    return 0;
}

/* Parse HEAD request*/
void head_request(int id, Request *request){
    printf("Recieved HEAD request\n");
    struct tm tm;
    struct stat sbuf;
    time_t now;
    // char buf[BUF_SIZE], tbuf[MIN_LINE], dbuf[MIN_LINE];
    // TODO: get filetype
    // char filetype[] = ".css";
    char   buf[BUF_SIZE], filetype[MIN_LINE], tbuf[MIN_LINE], dbuf[MIN_LINE];

    // TODO: validate file
    if(validate_file(id, request->http_uri) < 0) return;
    stat(request->http_uri, &sbuf);
    get_filetype(request->http_uri, filetype);

    // get time string
    tm = *gmtime(&sbuf.st_mtime);
    strftime(tbuf, MIN_LINE, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    now = time(0);
    tm = *gmtime(&now);
    strftime(dbuf, MIN_LINE, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sDate: %s\r\n", buf, dbuf);
    sprintf(buf, "%sServer: Liso/1.0\r\n", buf);
    // if (is_closed) sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-Length: %ld\r\n", buf, sbuf.st_size);
    sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
    sprintf(buf, "%sLast-Modified: %s\r\n\r\n", buf, tbuf);
    send(id, buf, strlen(buf), 0);
}

/* HTTP Response body */
void response_body(int id, Request *request){
    int fd, filesize;
    char *ptr;
    struct stat sbuf;

    if(validate_file(id, request->http_uri) < 0) return;
    if ((fd = open(request->http_uri, O_RDONLY, 0)) < 0)
    {
        // TODO Log("Error: Cann't open file \n");
        printf("Error: Response body Cannot open file");
        serve_error(id, "404", "Not Found",
                    "Server couldn't find this file");
        return;
    }
    stat(request->http_uri, &sbuf);
    filesize = sbuf.st_size;
    ptr = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    send(id, ptr, filesize, 0);
    munmap(ptr, filesize);
}

/* Parse GET request*/
void get_request(int id, Request *request){
    // get
    // head_request(id,request);
    // response_body(id,request);

    struct tm tm;
    struct stat sbuf;
    time_t now;
    int    fd, filesize;
    char   *ptr;
    char   buf[BUF_SIZE], filetype[MIN_LINE], tbuf[MIN_LINE], dbuf[MIN_LINE];

    // if(validate_file(id, request->http_uri) < 0) return;


    if ((fd = open(request->http_uri, O_RDONLY, 0)) < 0)
    {
        // TODO Log("Error: Cann't open file \n");
        printf("Response body error");
        serve_error(id, "404", "Not Found",
                    "Server couldn't find this file");
        return;
    }
    // char buf[BUF_SIZE], tbuf[MIN_LINE], dbuf[MIN_LINE];
    // TODO: get filetype
    // char filetype[] = ".css";
    // TODO: validate file

    get_filetype(request->http_uri, filetype);
    stat(request->http_uri, &sbuf);
    filesize = sbuf.st_size;
    ptr = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    // get time string
    tm = *gmtime(&sbuf.st_mtime);
    strftime(tbuf, MIN_LINE, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    now = time(0);
    tm = *gmtime(&now);
    strftime(dbuf, MIN_LINE, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sDate: %s\r\n", buf, dbuf);
    sprintf(buf, "%sServer: Liso/1.0\r\n", buf);
    // if (is_closed) sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-Length: %ld\r\n", buf, sbuf.st_size);
    sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
    sprintf(buf, "%sLast-Modified: %s\r\n\r\n", buf, tbuf);
    int bufLen = strlen(buf);
    strncpy(buf + bufLen, ptr, filesize);
    printf("Server response: %s\n",buf);
    munmap(ptr, filesize);
    send(id, buf, bufLen + filesize, 0);
    // send(id, ptr, filesize, 0);
    // close_socket(id);
}

/* Parse POST request*/
void post_request(int id, Request *request){
    // post
}

/* parse http request */
void parse_request(int id, char *buff, int BUF_SIZE_P){
    printf(" \n [parse_request] parsing request. \n");
    int index;
    int readRet = BUF_SIZE_P;
    printf("reading socket %d \n", id);

    // parse buffer to request structure
    Request *request = parse(buff,readRet,id);

    // If parsing failed, return 400-Bad Request
    if (!request) {
        serve_error(id, "400", "Bad Request",
            "The request is not understood by the server");
        // TODO: log
        return;
    }

    // parse uri, if root, return index.html
    char uri[1024] = "";
    strcpy(uri, WWW);
    if(!strcmp(request->http_uri, "/")){
      strcat(uri,"/index.html");
      strcpy(request->http_uri, uri);
      printf("GET Request for ROOT %s \n", request->http_uri);
    }else{
      // sprintf(request->http_uri, "%s%s", WWW, request->http_uri);
      strcat(uri,request->http_uri);
      strcpy(request->http_uri, uri);
      printf("GET Request for ROOT %s \n", request->http_uri);
    }
    //Print Everything
    // TODO: log
    // printf("Http Method %s\n",request->http_method);
    // printf("Http Version %s\n",request->http_version);
    // printf("Http Uri %s\n",request->http_uri);

    // check HTTP method (support GET, POST, HEAD now)
    if (strcasecmp(request->http_method, "GET")  &&
        strcasecmp(request->http_method, "HEAD") &&
        strcasecmp(request->http_method, "POST"))
    {
        // send error (501)
        serve_error(id,"501", "Not Implemented",
                   "The method is not valid or not implemented by the server");
        goto Done;
    }

    // send response
    if (!strcasecmp(request->http_method, "GET"))
        get_request(id, request);
    else if (!strcasecmp(request->http_method, "POST"))
        post_request(id, request);
    else if (!strcasecmp(request->http_method, "HEAD"))
        head_request(id, request);

    // do something with the headers
    // for(index = 0;index < request->header_count;index++){
        // printf("Request Header\n");
        // printf("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
    // }

    Done:
    free(request->headers);
    free(request);
    //TODO: log
    return;
}


int main(int argc, char* argv[])
{
    fd_set master; // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax; // maximum file descriptor numbe

//    int sock, client_sock;
//    ssize_t readret;
    int sock;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    if(argc > 1){
        strcpy(WWW, argv[1]);
    }

    int yes = 1; // for setsockopt() SO_REUSERADDR, below

    signal(SIGINT, signal_handler);
//    serverlog = fopen( "log.txt", "w" ); // Open file
    log_create("./serverlog");
//
//    if (serverlog == NULL) {
//        fprintf(stdout, "log file failed to open. Will have no log but still continue.");
//    }

    fprintf(stdout, "----- Echo Server -----\n");
//    fprintf(serverlog, "----- Echo Server -----\n");
    log_write("----- Echo Server -----\n", 1111);

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
//        fprintf(serverlog, "Failed creating socket.\n");
        log_write("Failed creating socket.\n", 1111);
        return EXIT_FAILURE;
    }

//    fprintf(serverlog,"Server Socket created. \n ");
    log_write("Server Socket created. \n ", 1111);

    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    // lose the pesky "address already in use" error message
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

//    fprintf(serverlog,"Server Socket binded. \n ");
    log_write("Server Socket binded. \n ", 1111);

    if (listen(sock, 1024))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
//        fprintf(serverlog, "Error listening on socket.\n");
        log_write("Error listening on socket.\n", 1111);
        return EXIT_FAILURE;
    }

    // add the listener to the master set
    FD_SET(sock, &master);

    //keep track of the biggest file descriptor
    fdmax = sock;// so far this is the one

//    fprintf(serverlog,"Server Socket listening. \n ");
    log_write("Server Socket listening. \n ",1111);

    /* finally, loop waiting for input and then write it back */
    int newfd; // newly accept()ed socket descriptor
    int nbytes;
    while (1)
    {
        read_fds = master; // copy it

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) { //select method
            fprintf(stderr, "Error when selecting, ready to exist.\n");
//            fprintf(serverlog,"Error when selecting, ready to exist.\n");
            log_write("Error when selecting, ready to exist.\n", 1111);
            return EXIT_FAILURE;
        }

        // run through the existed connections, looking for data to read
        for (int i = 0; i <= fdmax; i++){
            if (FD_ISSET(i, &read_fds)){ // select one successfully
                if (i == sock){
                    cli_size = sizeof(cli_addr);
                   if ((newfd = accept(sock, (struct sockaddr *) &cli_addr, &cli_size)) == -1)
                   {
                       close(sock);
                       fprintf(stderr, "Error accepting connection.\n");
//                       fprintf(serverlog, "Error accepting connection.\n");
                       log_write("Error accepting connection.\n", 1111);
                       return EXIT_FAILURE;
                   }
                   else{
                        FD_SET(newfd, &master); // add to aster set
                        if (newfd > fdmax) { // keep track of the max
                            fdmax = newfd;
                        }
                        fprintf(stdout, "New selection for socket %d connection.\n", newfd);
//                        fprintf(serverlog,"New selection for socket %d connection.\n", newfd);
                       log_write("New selection for socket %d connection.\n", 1111);

                    }
                }
                else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // If no data received
                        if (nbytes == 0) {
                            fprintf(stdout, "connection closed on socket: %d\n", i);
//                            fprintf(serverlog,"connection closed on socket");
                            log_write("Connection closed on socket\n", 1111);
                        }
                        else{
                            fprintf(stderr, "Error when connection closing on socket: %d\n", i);
//                            fprintf(serverlog,"connection closed on socket");
                            log_write("Error when connection closing on socket\n", 1111);
                        }
                        close_socket(i);
                        FD_CLR(i, &master);
                    }
                    else{
                        // If received properly, parse the request
                        printf("Socket %d Request parsed. \n", newfd);
                        printf("buf: %s", buf);
                        parse_request(i, buf, nbytes);
                        // if (send(i, buf, nbytes, 0) != nbytes) {
                        //     fprintf(stderr, "Error sending to client.\n");
                        //     return EXIT_FAILURE;
                        // }
//                        fprintf(serverlog ,"sent data to client: %d\n", i);
                        log_write("Sent data to client.\n", 1111);
                        memset(buf, 0, BUF_SIZE);
                    }
                }
            }
        }
    }

    close_sockets(fdmax);
//    fclose(serverlog);
    log_close();

    return EXIT_SUCCESS;
}
