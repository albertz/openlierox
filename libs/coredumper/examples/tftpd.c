/* Copyright (c) 2005, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ---
 * Author: Markus Gutschke
 *
 * This file demonstrates how to add a core dump interface to an existing
 * service. Typically, you would want to add the call to GetCoreDump()
 * to an existing interface exposed by your server. But if no such interface
 * exists, you could also adapt the TFTP code in this module to run as a
 * thread in your server.
 *
 * The code in this module does not perform any type of access control.
 * As corefiles can expose security sensitive data (e.g. passwords), you
 * would need to add appropriate access controls when using this (or similar)
 * code in a production environment.
 */

#include <arpa/inet.h>
#include <arpa/tftp.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <google/coredumper.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#undef  NO_THREADS      /* Support only one connection at a time.            */
#undef  CAN_READ_FILES  /* Supports reading files from "/tftpboot/..."       */
#undef  CAN_WRITE_FILES /* Supports updating files in "/tftpboot/..."        */
#define CAN_READ_CORES  /* Supports serving "core" snapshot files.           */


#ifndef TFTPHDRSIZE
  #define TFTPHDRSIZE 4
#endif


/* The "Request" structure contains all the parameters that get passed
 * from the main server loop to the thread that is processing a given
 * TFTP connection.
 */
typedef struct Request {
  int                debug;
  int                id;
  int                fd;
  struct tftphdr     *tftp;
  char               buf[TFTPHDRSIZE + SEGSIZE];
  size_t             count;
  struct sockaddr_in addr;
  socklen_t          len;
  const char         *core_name;
  char               **dirs;
  int                no_ack;
  int                sanitize;
} Request;

#define DBG(...) do {                                                         \
                   if (debug)                                                 \
                     fprintf(stderr, __VA_ARGS__);                            \
                 } while (0)

/* tftp_thread() runs in its own thread (unless NO_THREADS is defined). This
 * function processes a single TFTP connection.
 */
static void *tftp_thread(void *arg) {
#define debug (request->debug)
  Request            *request    = (Request *)arg;
  struct tftphdr     *tftp       = request->tftp;
  int                fd          = -1;
  int                src         = -1;
  int                err         = EBADOP;
  int                ioerr       = 0;
  struct sockaddr_in addr;
  socklen_t          len;
  char               *raw_file_name, *mode, *msg, msg_buf[80];
  char               *file_name = NULL;

  /* Create a new socket for this connection.                                */
  if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  /* Connect this socket to the outgoing interface.                          */
  len = sizeof(addr);
  if (getsockname(request->fd, (struct sockaddr *)&addr, &len) >= 0 &&
      len >= sizeof(struct sockaddr_in)) {
    DBG("Responding to %s:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
    addr.sin_port = 0;
    bind(fd, (struct sockaddr *)&addr, len);
  }

  /* Get file name and transfer mode from the incoming TFTP packet.          */
  if (!(raw_file_name = request->buf + 2, mode = memchr(raw_file_name, '\000',
                       request->count - 1 - (raw_file_name - request->buf))) ||
      !(++mode, memchr(mode, '\000', request->count - (mode - request->buf)))){
    char *ptr;
    msg = "Truncated request";

  error:
    /* Send error message back to client.                                    */
    DBG("%s\n", msg);
    ptr = strrchr(strcpy(tftp->th_msg, msg), '\000') + 1;
    tftp->th_opcode = htons(ERROR);
    tftp->th_code   = htons(err);
    sendto(fd, tftp, ptr-request->buf, 0,
           (struct sockaddr *)&request->addr, request->len);
    goto done;
  }

  /* Only text and binary transfer modes are supported.                      */
  if (strcasecmp(mode, "netascii") && strcasecmp(mode, "octet")) {
    msg = "Unsupported transfer mode";
    goto error;
  }

  /* Check whether client requested a "core" snapshot of the running process.*/
  if (!strcmp(raw_file_name, request->core_name)) {
    #ifdef CAN_READ_CORES
      /* Core files must be transferred in binary.                           */
      if (strcasecmp(mode, "octet")) {
        err = EBADOP;
        msg = "Core files must be transferred in binary";
        goto error;
      }

      /* Writing core files is not a supported operation.                    */
      if (ntohs(tftp->th_opcode) == WRQ) {
        err = EBADOP;
        msg = "Core files cannot be written";
        goto error;
      }

      /* Here we go. Create a snapshot of this process.                      */
      src = GetCoreDump();

      /* If we failed to created a core file, report error to the client.    */
      if (src < 0) {
        err = ENOTFOUND;
        *msg_buf = '\000';
        msg = strerror_r(errno, msg_buf, sizeof(msg_buf));
        goto error;
      }
    #else
      err = ENOTFOUND;
      msg = "Core file support is not enabled";
      goto error;
    #endif
  } else {
    #if defined(CAN_READ_FILES) || defined(CAN_WRITE_FILES)
      /* TFTP is a very simple protocol, which does not support any user
       * authentication/authorization. So, we have to be very conservative
       * when accessing files. Unless overridden on the command line, this
       * server will only access files underneath the "/tftpboot" directory.
       * It only serves world-readable files, and it only allow writing to
       * world-writable files.
       */
      static char *tftpdirs[] = { "/tftpboot", NULL };
      char        **dirs      = tftpdirs;
      struct stat sb;

      /* Unless the user requested otherwise, restrict to "/tftpboot/..."    */
      if (*request->dirs) {
        dirs = request->dirs;
      }

      /* If "sanitize" option is set, prepend "/tftpboot" (or name of the first
       * source directory listed on the command line) to any absolute file
       * names.
       */
      if (*raw_file_name == '/' && request->sanitize) {
        char *path = dirs[0];
        strcat(strcat(strcpy(malloc(strlen(path) + strlen(raw_file_name) + 2),
                             path), "/"), raw_file_name);
        
      } else {
        file_name = strdup(raw_file_name);
      }
      
      /* Check file attributes.                                              */
      memset(&sb, 0, sizeof(sb));
      if (*file_name == '/') {
        int  ok = 0;
        char **ptr;
        
        /* Search for file in all source directories (normally just
         * "/tftpboot")
         */
        for (ptr = &dirs[0]; *ptr; ptr++) {
          if (!strncmp(*ptr, file_name, strlen(*ptr)) &&
              stat(file_name, &sb) >= 0 && S_ISREG(sb.st_mode)) {
            ok++;
            break;
          }
        }
        if (!ok) {
       file_not_found:
          /* Only pre-existing files can be accessed.                        */
          if (request->no_ack)
            goto done;
          else {
            err = ENOTFOUND;
            msg = "File not found";
            goto error;
          }
        }
      } else {
        char **ptr, *absolute_file_name = NULL;
        
        /* Search for file in all source directories (normally just
         * "/tftpboot")
         */
        for (ptr = &dirs[0]; *ptr; ptr++) {
          absolute_file_name = strcat(strcat(strcpy(malloc(strlen(*ptr) +
                                                        strlen(file_name) + 2),
                                                    *ptr), "/"), file_name);
          if (stat(absolute_file_name, &sb) >= 0 && S_ISREG(sb.st_mode))
            break;
          free(absolute_file_name);
          absolute_file_name = NULL;
        }
        if (!absolute_file_name)
          goto file_not_found;
        free(file_name);
        file_name = absolute_file_name;
      }
      
      /* Check whether the necessary support for reading/writing is compiled
       * into this server, and whether the file is world-readable/writable.
       */
      if (ntohs(tftp->th_opcode) == WRQ) {
        #ifdef CAN_WRITE_FILES
          if (!(sb.st_mode & S_IWOTH) ||
              (src = open(file_name, O_WRONLY)) < 0)
        #endif
        {
       access_denied:
          err = EACCESS;
          msg = "Access denied";
          goto error;
        }
      } else {
        #ifdef CAN_READ_FILES
          if (!(sb.st_mode & S_IROTH) ||
              (src = open(file_name, O_RDONLY)) < 0)
        #endif
            goto access_denied;
      }
    #else
      err = ENOTFOUND;
      msg = "File operations are not enabled";
      goto error;
    #endif
  }

  if (ntohs(tftp->th_opcode) == RRQ) {
    DBG("received RRQ <%s, %s>\n", raw_file_name, mode);
    #if defined(CAN_READ_FILES) || defined(CAN_READ_CORES)
      unsigned short block = 0;
      int            count;

      /* Mainloop for serving files to clients.                              */
      do {
        char           buf[TFTPHDRSIZE + SEGSIZE];
        struct tftphdr *send_tftp = (struct tftphdr *)buf;
        char           *ptr       = send_tftp->th_msg;
        int            retry;

        /* Deal with partial reads, and reblock in units of 512 bytes.       */
        count = 0;
        while (!ioerr && count < SEGSIZE) {
          int rc = read(src, ptr + count, SEGSIZE - count);
          if (rc < 0) {
            if (errno == EINTR)
              continue;
            ioerr = errno;
            break;
          } else if (rc == 0) {
            break;
          }
          count += rc;
        }

        /* Report any read errors back to the client.                        */
        if (count == 0 && ioerr) {
          err = ENOTFOUND;
          *msg_buf = '\000';
          msg = strerror_r(ioerr, msg_buf, sizeof(msg_buf));
          goto error;
        }
        send_tftp->th_opcode = htons(DATA);
        send_tftp->th_block  = htons(++block);

        /* Transmit a single packet. Retry if necessary.                     */
        retry = 10;
        for (;;) {
          int rc;

          /* Terminate entire transfers after too many retries.              */
          if (--retry < 0)
            goto done;

          /* Send one 512 byte packet.                                       */
          DBG("send DATA <block=%d, 512 bytes>\n", block);
          if (sendto(fd, send_tftp, TFTPHDRSIZE + count, 0,
                     (struct sockaddr *)&request->addr, request->len) < 0) {
            if (errno == EINTR)
              continue;
            goto done;
          }

          /* Wait for response from client.                                  */
          do {
            fd_set         in_fds;
            struct timeval timeout;
            FD_ZERO(&in_fds);
            FD_SET(fd, &in_fds);
            timeout.tv_sec  = 5;
            timeout.tv_usec = 0;
            rc = select(fd+1, &in_fds, NULL, NULL, &timeout);
          } while (rc < 0 && errno == EINTR);

          /* If no response received, try sending payload again.             */
          if (rc == 0)
            continue;

          /* Receive actual response.                                        */
          rc = recv(fd, tftp, TFTPHDRSIZE + SEGSIZE, MSG_TRUNC);

          /* If operation failed, terminate entire transfer.                 */
          if (rc < 0) {
            if (errno == EINTR)
              continue;
            goto done;
          }

          /* Done transmitting this block, after receiving matching ACK      */
          if (rc >= TFTPHDRSIZE) {
            switch (ntohs(tftp->th_opcode)) {
              case ACK:
                DBG("received ACK <block=%d>\n", ntohs(tftp->th_block));
                break;
              case RRQ:
                DBG("received RRQ\n");
                break;
              case WRQ:
                DBG("received WRQ\n");
                break;
              case DATA:
                DBG("received DATA\n");
                break;
              case ERROR:
                DBG("received ERROR\n");
                break;
              default:
                DBG("unexpected data, op=%d\n", ntohs(tftp->th_opcode));
                break;
            }
          }
          if (rc >= TFTPHDRSIZE &&
              ntohs(tftp->th_opcode) == ACK &&
              tftp->th_block == send_tftp->th_block)
            break;
        }
      } while (count);
    #endif
  } else {
    #ifdef CAN_WRITE_FILES
      /* TODO: Add support for writing files */
    #endif
  }

 done:
  /* Clean up, close all file handles, and release memory                    */
  if (fd >= 0)
    close(fd);
  if (src >= 0)
    close(src);
  if (file_name)
    free(file_name);
  free(request);
  return 0;
#undef debug
}


/* This is a very basic TFTP server implementing RFC 1350, but none of the
 * optional protocol extensions (e.g. no block size negotiation, and no
 * multicasting).
 */
int main(int argc, char *argv[]) {
  static const struct option long_opts[] = {
    /* Set file name for "core" snapshot file of running process.            */
    { "core",      1, NULL, 'c' },

    /* Enable debugging output.                                              */
    { "debug",     0, NULL, 'd' },

    /* Print usage information for this server.                              */
    { "help",      0, NULL, 'h' },

    /* Suppress negative acknowledge for non-existant files.                 */
    { "noack",     0, NULL, 'n' },

    /* Set port number to listen on.                                         */
    { "port",      1, NULL, 'p' },

    /* Sanitize requests for absolute filenames by prepending the name of the
     * first directory specified on the command line. If no directory is
     * given, prepend "/tftpboot/".
     */
    { "sanitize",  0, NULL, 's' },
    { NULL,          0, NULL, 0 } };
  static const char *opt_string = "c:dhnp:s";
  const char        *core_name  = "core";
  int               debug       = 0;
  int               no_ack      = 0;
  int               port        = -1;
  int               sanitize    = 0;
  char              **dirs      = NULL;
  int               server_fd   = 0;
  int               id          = 0;

  /* Parse command line options.                                             */
  for (;;) {
    int idx = 0;
    int c   = getopt_long(argc, argv, opt_string, long_opts, &idx);
    if (c == -1)
      break;
    switch (c) {
    case 'c':
      core_name = optarg;
      break;
    case 'd':
      debug = 1;
      break;
    case 'n':
      no_ack = 1;
      break;
    case 'p':
      port = atoi(optarg);
      if (port <= 0 || port > 65535) {
        fprintf(stderr, "Port out of range: %d\n", port);
        exit(1);
      }
      break;
    case 's':
      sanitize = 1;
      break;
    case 'h':
    default:
      fprintf(stderr,
           "Usage: %s --core <name> --debug --help --port <port> --noack "
           "--sanitize\n",
           argv[0]);
      exit(c != 'h');
    }
  }

  /* All remaining command line arguments (if any) list the directories of
   * files that this server can access. If no directories are given, then
   * only files in "/tftpboot/..." are accessible.
   */
  dirs = argv + optind;

  /* If a port is given on the command line, then listen on. Otherwise,
   * assume that stdin is already connected to a server port. This allows
   * us to run the server from inetd.
   */
  if (port >= 0) {
    struct sockaddr_in addr;

    server_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
      perror("socket");
      exit(1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) <0){
      perror("bind");
      exit(1);
    }
  }

  /* Server mainloop. Accept incoming connections and spawn threads to
   * serve the requests.
   */
  for (;;) {
    const char         *msg;
    char               buf[TFTPHDRSIZE + SEGSIZE];
    struct tftphdr     *tftp = (struct tftphdr *)buf;
    struct sockaddr_in addr;
    socklen_t          len;
    int                count, type;
    Request            *request;
    #ifndef NO_THREADS
    pthread_t          thread;
    #endif

    /* Receive next request.                                                 */
    len   = sizeof(addr);
    count = recvfrom(server_fd, tftp, sizeof(buf), MSG_TRUNC,
                     (struct sockaddr *)&addr, &len);
    if (count < 0) {
      if (errno == EINTR)
        continue;
      perror("recvfrom");
      exit(1);
    }

    /* If request arrived from unsupported address, just ignore it.          */
    if (len < sizeof(struct sockaddr_in))
      continue;

    /* If request was truncated, report error back to client.                */
    if (count < sizeof(tftp)) {
      char *ptr;
      msg = "Truncated request";
    send_error:
      /* Send error message to client.                                       */
      DBG("%s\n", msg);
      ptr = strrchr(strcpy(tftp->th_msg, msg), '\000') + 1;
      tftp->th_opcode = htons(ERROR);
      tftp->th_code   = htons(EBADOP);
      sendto(server_fd, tftp, ptr-buf, 0, (struct sockaddr *)&addr, len);
      continue;
    }
    
    /* Determine whether this was a read or write request.                   */
    type = ntohs(tftp->th_opcode);
    if (type != RRQ && type != WRQ) {
      msg = "Request must be RRQ or WRQ";
      goto send_error;
    }

    /* Build "Request" data structure with parameters describing connection. */
    request            = calloc(sizeof(Request), 1);
    request->debug     = debug;
    request->id        = id++;
    request->fd        = server_fd;
    request->tftp      = (struct tftphdr *)&request->buf;
    request->count     = count;
    request->len       = len;
    request->core_name = core_name;
    request->dirs      = dirs;
    request->no_ack    = no_ack;
    request->sanitize  = sanitize;
    memcpy(&request->buf,  buf,   count);
    memcpy(&request->addr, &addr, len);

    /* Hand request off to its own thread.                                   */
    #ifdef NO_THREADS
    tftp_thread(request);
    #else
    pthread_create(&thread, NULL, tftp_thread, request);
    #endif
  }
}
