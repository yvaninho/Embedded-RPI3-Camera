#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <libv4l2.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#include "serveur.h"
#include "Client.h"

int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }



         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = { csock };
         strncpy(c.name, buffer, BUF_SIZE - 1);
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         int picnum=1;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
	       //printf("From client: %s\t ", buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);

                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
		               read_message_from_clients(clients, client, actual, buffer, 0,&picnum);
                  //send_message_to_all_clients(clients, client, actual, buffer, 0);
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}


static void read_message_from_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server,int *picnum)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
   puts(message);
   if (strncmp("p", buffer, 1) == 0) {
       printf("Server taking picture...\n");
       int fd;
       if((fd = open("/dev/video0", O_RDWR)) < 0){
           perror("open");
           exit(1);
       }

       struct v4l2_capability cap;
       if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
           perror("VIDIOC_QUERYCAP");
           exit(1);
       }

       if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
       fprintf(stderr, "The device does not handle single-planar video capture.\n");
       exit(1);
       }

       struct v4l2_format format;
       format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
       format.fmt.pix.width = 800;
       format.fmt.pix.height = 600;

       if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
           perror("VIDIOC_S_FMT");
           exit(1);
       }

       struct v4l2_requestbuffers bufrequest;
       bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       bufrequest.memory = V4L2_MEMORY_MMAP;
       bufrequest.count = 1;

       if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
         perror("VIDIOC_REQBUFS");
         exit(1);
       }

       struct v4l2_buffer bufferinfo;
       memset(&bufferinfo, 0, sizeof(bufferinfo));

       bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       bufferinfo.memory = V4L2_MEMORY_MMAP;
       bufferinfo.index = 0;

       if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
           perror("VIDIOC_QUERYBUF");
           exit(1);
       }

       void* buffer_start = mmap(
       NULL,
       bufferinfo.length,
       PROT_READ | PROT_WRITE,
       MAP_SHARED,
       fd,
       bufferinfo.m.offset
       );

       if(buffer_start == MAP_FAILED){
           perror("mmap");
           exit(1);
       }

       memset(buffer_start, 0, bufferinfo.length);



   memset(&bufferinfo, 0, sizeof(bufferinfo));
   bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   bufferinfo.memory = V4L2_MEMORY_MMAP;
   bufferinfo.index = 0; /* Queueing buffer index 0. */

   // Put the buffer in the incoming queue.
   if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
       perror("VIDIOC_QBUF");
       exit(1);
   }

   // Activate streaming
   int type = bufferinfo.type;
   if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
       perror("VIDIOC_STREAMON");
       exit(1);
   }

   int k =0;
   //clock_t temps;

   while(k<2){
       // Dequeue the buffer.
       if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
           perror("VIDIOC_QBUF");
           exit(1);
       }

       bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       bufferinfo.memory = V4L2_MEMORY_MMAP;
       /* Set the index if using several buffers */

       // Queue the next one.
       if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
           perror("VIDIOC_QBUF");
           exit(1);
       }
       k++;
   }


   int jpgfile;

   char* filename = "image.jpeg";
   /*
    int exist = cfileexists(filename);
    if(exist){
        printf("File %s exist",filename);
        strcat(filename,&picnum);
        jpgfile = open(filename, O_WRONLY | O_CREAT, 0660);
      }
    else
      if((jpgfile = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
        perror("open");
        exit(1);
      }*/

      if((jpgfile = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
        perror("open");
        exit(1);
      }
   //cfileexists



   write(jpgfile, buffer_start, bufferinfo.length);
   close(jpgfile);

   if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
       perror("VIDIOC_STREAMOFF");
       exit(1);
   }

       printf("picture taken and saved\n");

       close(fd);
   }


}


static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }
   printf("socket creation failed...\n");

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }
   printf("connected to the server..\n");

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
