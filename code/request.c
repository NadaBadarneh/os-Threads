//
// request.c: Does the bulk of the work for the web server.
// 

#include "segel.h"
#include "request.h"

// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(char *cause, char *errnum, char *shortmsg, char *longmsg,Request request, threads_stats t_stats)
{
	char buf[MAXLINE], body[MAXBUF];
    int fd=request->fd;

	// Create the body of the error message
	sprintf(body, "<html><title>OS-HW3 Error</title>");
	sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

	// Write out the header information for this response
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);

	sprintf(buf, "Content-Type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);

	sprintf(buf, "Content-Length: %lu\r\n", strlen(body));


	sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, request->ArrivalTime.tv_sec, request->ArrivalTime.tv_usec);

	sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, request->dispachTime.tv_sec, request->dispachTime.tv_usec);

	sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);

	sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->count);

	sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);

	sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, t_stats->dynm_req);
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);
	Rio_writen(fd, body, strlen(body));
	printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
	char buf[MAXLINE];

	Rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n")) {
		Rio_readlineb(rp, buf, MAXLINE);
	}
	return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs)
{
	char *ptr;
	// if (ptr = strstr(uri, "est=")) {
	// 	char* end_arg; 
	// 	if (end_arg = strstr(uri, "&"))
	// 		memmove(ptr, end_arg + 1, strlen(end_arg));
	// 	else
	// 		ptr = '\0';
	// }
	if (strstr(uri, "..")) {
		sprintf(filename, "./public/home.html");
		return 1;
	}
	if (!strstr(uri, "cgi")) {
		// static
		strcpy(cgiargs, "");
		sprintf(filename, "./public/%s", uri);
		if (uri[strlen(uri)-1] == '/') {
			strcat(filename, "home.html");
		}
		return 1;
	} else {
		// dynamic
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		} else {
			strcpy(cgiargs, "");
		}
		sprintf(filename, "./public/%s", uri);
		return 0;
	}
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}

void requestServeDynamic( char *filename, char *cgiargs, Request request, threads_stats t_stats)
{
	char buf[MAXLINE], *emptylist[] = {NULL};
    int fd = request->fd;

	// The server does only a little bit of the header.
	// The CGI script has to finish writing out the header.
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);

	sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf,request->ArrivalTime.tv_sec, request->ArrivalTime.tv_usec);

	sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, request->dispachTime.tv_sec, request->dispachTime.tv_usec);

	sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);

	sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->count);

	sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);

	sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, t_stats->dynm_req);

	Rio_writen(fd, buf, strlen(buf));
   	int pid = 0;
   	if ((pid = Fork()) == 0) {
     	 /* Child process */
     	 Setenv("QUERY_STRING", cgiargs, 1);
     	 /* When the CGI process writes to stdout, it will instead go to the socket */
     	 Dup2(fd, STDOUT_FILENO);
     	 Execve(filename, emptylist, environ);
   	}
  	WaitPid(pid, NULL, WUNTRACED);
}


void requestServeStatic( char *filename, int filesize,Request request,threads_stats t_stats)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];

    int fd = request->fd;

	requestGetFiletype(filename, filetype);

	srcfd = Open(filename, O_RDONLY, 0);

	// Rather than call read() to read the file into memory,
	// which would require that we allocate a buffer, we memory-map the file
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);

	// put together response
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
	sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
	sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, request->ArrivalTime.tv_sec, request->ArrivalTime.tv_usec);

	sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, request->dispachTime.tv_sec, request->dispachTime.tv_usec);

	sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, t_stats->id);

	sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, t_stats->count);

	sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, t_stats->stat_req);

	sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, t_stats->dynm_req);

	Rio_writen(fd, buf, strlen(buf));

	//  Writes out to the client socket the memory-mapped file
	Rio_writen(fd, srcp, filesize);
	Munmap(srcp, filesize);
}

//  Returns True/False if realtime event
int getRequestMetaData(int fd /*, int* est* for future use ignore this*/)
{
	char buf[MAXLINE], method[MAXLINE];
	int bytesRead  = recv(fd, buf, MAXLINE - 1, MSG_PEEK);
	 if (bytesRead == -1) {
		perror("recv");
		return 1;
	 }
	sscanf(buf, "%s ", method);
	int isRealTime = !strcasecmp(method, "REAL");
	return isRealTime;
}


// handle a request
void requestHandle(Request request, threads_stats t_stats, Queue waiting_queue)
{
	int if_skip = 0;
	int req_valid=0;


	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	Rio_readinitb(&rio, request->fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri, version);
	const char *skip_ext = ".skip";
    ssize_t uri_len = strlen(uri);
    ssize_t skip_len = strlen(skip_ext);

    if (uri_len >= skip_len && strcmp(uri + uri_len - skip_len, skip_ext) == 0) {
        if_skip = 1;
    }
    Request nextRequest = NULL;
	Request request1 = NULL;
	Request request2 = NULL;

    if (if_skip) {
        uri[uri_len-skip_len]='\0';
        nextRequest= dequeue_last(waiting_queue);
        // if(nextRequest != NULL) {
        //   req_valid = 1;
        // }
       // is_dequeued = dequeue(wai,&nextRequest);
    }

    printf("%s %s %s\n", method, uri, version);

    if (strcasecmp(method, "GET") && strcasecmp(method, "REAL")) {
        addThread(t_stats,3);

		requestError( method, "501", "Not Implemented", "OS-HW3 Server does not implement this method",request, t_stats);
		return;
	}

	requestReadhdrs(&rio);

	is_static = requestParseURI(uri, filename, cgiargs);
	if (stat(filename, &sbuf) < 0) {
        addThread(t_stats,3);
		requestError( filename, "404", "Not found", "OS-HW3 Server could not find this file", request, t_stats);
		return;
	}

	if (is_static) {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            addThread(t_stats,3);
			requestError( filename, "403", "Forbidden", "OS-HW3 Server could not read this file", request, t_stats);
			return;
		}
        addThread(t_stats,1);
		//(t_stats->stat_req)++;
		requestServeStatic( filename, sbuf.st_size, request, t_stats);
	} else {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            addThread(t_stats,3);
			requestError( filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program", request, t_stats);
			return;
		}
        addThread(t_stats,2);
		//(t_stats->dynm_req)++;
		requestServeDynamic( filename, cgiargs, request, t_stats);
    }
    if(nextRequest) {
        rio_t next_rio;
        char next_buf[MAXLINE], next_method[MAXLINE], next_uri[MAXLINE], next_version[MAXLINE];
        char next_filename[MAXLINE], next_cgiargs[MAXLINE];
        struct stat next_sbuf;
        int next_is_static;

        // Initialize the rio for nextRequest
        Rio_readinitb(&next_rio, nextRequest->fd);
        // Read the request line for nextRequest
        Rio_readlineb(&next_rio, next_buf, MAXLINE);
        sscanf(next_buf, "%s %s %s", next_method, next_uri, next_version);
        printf("%s %s %s\n", next_method, next_uri, next_version); /// here we know there is no .skip


        if (strcasecmp(next_method, "GET")&& strcasecmp(method, "REAL")) {
            addThread(t_stats, 3);
            requestError(next_method, "501", "Not Implemented", "OS-HW3 Server does not implement this method", nextRequest, t_stats);
            return;
        }
        // Read the request headers for nextRequest
        requestReadhdrs(&next_rio);

        // Determine if the nextRequest is static or dynamic
        next_is_static = requestParseURI(next_uri, next_filename, next_cgiargs);
        if (stat(next_filename, &next_sbuf) < 0) {
            addThread(t_stats, 3);
            requestError(next_filename, "404", "Not found", "OS-HW3 Server could not find this file", nextRequest, t_stats);
            return;
        }

        if (next_is_static) {
            if (!(S_ISREG(next_sbuf.st_mode)) || !(S_IRUSR & next_sbuf.st_mode)) {
                addThread(t_stats, 3);
                requestError(next_filename, "403", "Forbidden", "OS-HW3 Server could not read this file", nextRequest, t_stats);
                return;
            }
            addThread(t_stats, 1);
            requestServeStatic(next_filename, next_sbuf.st_size, nextRequest, t_stats);
        } else {
            if (!(S_ISREG(next_sbuf.st_mode)) || !(S_IXUSR & next_sbuf.st_mode)) {
                addThread(t_stats, 3);
                requestError(next_filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program", nextRequest, t_stats);
                return;
            }
            addThread(t_stats, 2);
            requestServeDynamic(next_filename, next_cgiargs, nextRequest, t_stats);
        }
    }

}