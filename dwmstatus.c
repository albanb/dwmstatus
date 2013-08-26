/*Include libraries*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

/* Macros definition */
#define LENGTH(X) (sizeof X / sizeof X[0])
#define BUF_SIZE (2048)

/* structures declaration*/
typedef struct {
	int (*func)(char *);
	int refresh;
	char * const *cmd;
} Stbar;

int protocol_priority[] = {GNUTLS_TLS1_2, GNUTLS_TLS1_1, GNUTLS_TLS1, GNUTLS_SSL3, 0};

typedef enum { imap, imaps } Protocol;

typedef struct
{
	gnutls_session_t session;
	gnutls_certificate_credentials_t xcred;
} SslSession;

typedef union Session
{
	FILE *f;
	SslSession s;
} Comm;

typedef struct
{
	char *serverName;
	Protocol protocol;
	int port;
	char *user;
	char *passwd;
	char *inbox;
	Comm comm;
} Box;

typedef struct
{
	int (*mailInit) (Box *boxes);
	int (*mailRead) (char *buf, char *ctl, Box boxes);
	int (*mailWrite) (char *buf, Box boxes);
	int (*mailClose) (int fd, Box boxes);
} MailFunction;

/* Functions declaration */
static int alsa_sound(char *stat);
static int bat(char *stat);
static int mktimes(char *stat);
static int proc(char *stat);
static int mem(char *stat);
static int is_up(char *device);
static int net(char *stat);
static int todo(char *stat);
static int mount(char *stat);
static int os_kernel(char *stat);
static int runevery(time_t *ltime, int sec);
static int mailImap (char *stat);
static int mail_socket_init (Box *boxes);
static int mail_socket_write (char *buf, Box boxes);
static int mail_socket_read (char *buf, char *ctl, Box boxes);
static int mail_socket_close (int fd, Box boxes);
static int mail_ssl_init (Box *boxes);
static int mail_ssl_write (char *buf, Box boxes);
static int mail_ssl_read (char *buf, char *ctl, Box boxes);
static int mail_ssl_close (int fd, Box boxes);
static int sock_connect (char *hostname, int port);
static int buffer_init (char *buf);

/*Yours configuration*/
#include <dwmstatus.h>

/*Main function*/
int main() {
	Display *dpy;
	Window root;
	char statnext[100]; 
	char status[300];
	unsigned int i;
	time_t lastrefresh[LENGTH(stbar)]={0};
	char statbck[LENGTH(stbar)][100]; 

	setlocale(LC_ALL, "");
/* Setup X display and root window id: */
	dpy=XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy,DefaultScreen(dpy));
/* MAIN LOOP STARTS HERE */
	while (1) {
		statnext[0] = '\0';
		status[0] = '\0';

		for (i=0; i < LENGTH(stbar); i++)
		{
			if(runevery(&lastrefresh[i],stbar[i].refresh))
			{
				stbar[i].func(statnext);
				strcat(status,statnext);
				statbck[i][0]='\0';
				strcat(statbck[i],statnext);

			}
			else
				strcat(status,statbck[i]);
		}
	/* Set root name */
		XStoreName(dpy,root,status);
		XFlush(dpy);
		sleep(INTERVAL);
	}
/* NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy Trilby's O.C.D. ;) */
	XCloseDisplay(dpy);
	return 0;
}

/* To manage sequencement*/
int runevery(time_t *ltime, int sec)
{
	time_t now = time(NULL);
	if (difftime(now, *ltime) >= sec)
	{
		*ltime =now;
		return(1);
	}
	else
		return(0);
}

/* Set time information */
int mktimes(char *stat)
{
	time_t current;
	int lnum = 0;

	time(&current);
	lnum = strftime(stat,38,DATE_TIME_STR,localtime(&current));
	
	return lnum;
}

/* Set volume information */
int alsa_sound(char *stat)
{
	int mute=0, realvol=0, len = 0;
	long vol=0, min=0, max=0;
	snd_mixer_t *handle; /* init alsa */
	snd_mixer_selem_id_t *vol_info; /* init channel with volume info */
	snd_mixer_selem_id_t *mute_info; /* init channel with mute info */

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	snd_mixer_selem_id_malloc(&vol_info);
	snd_mixer_selem_id_set_name(vol_info, VOL_CH);
	snd_mixer_elem_t* pcm_mixer = snd_mixer_find_selem(handle, vol_info);
	snd_mixer_selem_get_playback_volume_range(pcm_mixer, &min, &max); /* get volume */
	snd_mixer_selem_get_playback_volume(pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
	snd_mixer_selem_id_malloc(&mute_info);
	snd_mixer_selem_id_set_name(mute_info, VOL_CH);
	snd_mixer_elem_t* mas_mixer = snd_mixer_find_selem(handle, mute_info);
	snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute); /* get mute state */

	if(mute == 0)
		len = sprintf(stat, VOL_MUTE_STR, 0);
	else
		realvol = (vol*100)/max;
		len = sprintf(stat, VOL_STR, realvol);

	if(vol_info)
		snd_mixer_selem_id_free(vol_info);
	if (mute_info)
		snd_mixer_selem_id_free(mute_info);
	if (handle)
		snd_mixer_close(handle);

	return len;
}

/* Set battery information */
int bat(char *stat)
{
	FILE *infile;
	long lnum1, lnum2;
	int num, len;
	char stattmp[50];

	stattmp[0] = '\0';
	/* Power / Battery */
	infile = fopen(BATT_NOW,"r");
	fscanf(infile,"%ld\n",&lnum1); fclose(infile);
	infile = fopen(BATT_FULL,"r");
	fscanf(infile,"%ld\n",&lnum2); fclose(infile);
	infile = fopen(BATT_STAT,"r");
	fscanf(infile,"%s\n",stattmp); fclose(infile);
	num = lnum1*100/lnum2;
	if (strncmp(stattmp,"Charging",8) == 0) {
		len = sprintf(stat,BAT_CHRG_STR,num);
	}
	else if (strncmp(stattmp,"Full",8) == 0) {
		len = sprintf(stat,BAT_FULL_STR,num);
	}
	else {
		if (num <  BATT_LOW)
			len = sprintf(stat,BAT_LOW_STR,num);
		else
			len = sprintf(stat,BAT_STR,num);
	}
	return len;
} 

/* Set processor usage information */
int proc(char *stat)
{
	int len,num;
	static long sjif1=0,sjif2=0,sjif3=0,sjift=0;
	long lnum1,lnum2,lnum3,lnum4;
	FILE *infile;

	/* New values */
	infile = fopen(CPU_FILE,"r");
	fscanf(infile,"cpu %ld %ld %ld %ld",&lnum1,&lnum2,&lnum3,&lnum4);
	fclose(infile);
	if (lnum4 > sjift)
		num = (int) ((100*((lnum1-sjif1)+(lnum2-sjif2)+(lnum3-sjif3)))/((lnum1-sjif1)+(lnum2-sjif2)+(lnum3-sjif3)+(lnum4-sjift)));
	else
		num = 0;
	sjif1=lnum1; sjif2=lnum2; sjif3=lnum3; sjift=lnum4;
	if (num > CPU_HI)
		len=sprintf(stat,CPU_HI_STR,num);
	else
		len=sprintf(stat,CPU_STR,num);
	
	return len;
}

/* Set memory usage information */
int mem(char *stat)
{
	int len;
	FILE *infile;
	long lnum1,lnum2,lnum3,lnum4,lnum5,lnum6;

	infile=fopen(MEM_FILE,"r");
	fscanf(infile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",&lnum1,&lnum2,&lnum3,&lnum4);
	fseek(infile,364,SEEK_SET);
	fscanf(infile,"SwapTotal: %ld kB\nSwapFree: %ld kB\n",&lnum5,&lnum6);
	fclose(infile);
	if (lnum5 != lnum6)
		len=sprintf(stat,MEM_SWAP_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M');
	else
		len=sprintf(stat,MEM_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M');

	return len;
}

/*Check if a device is up or not*/
int is_up(char *device)
{
	char sdevicepath[35],sdevicestate[5];
	FILE *infile;

	sprintf(sdevicepath,FILE_OPER,device);
	infile=fopen(sdevicepath,"r");
	if (infile != NULL)
	{
		fscanf(infile,"%s\n",sdevicestate);
		fclose(infile);
		if(strncmp(sdevicestate,"up",2) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*Set net information */
int net(char *stat)
{
	char sfilepath[43];
	FILE *infile;
	static long slup=0,sldown=0;
	long lupload=0,ldownload=0,luploadrate,ldownloadrate;
	int len;

	if(is_up(WIRED) == 1)
	{
		sprintf(sfilepath,FILE_UPLOAD,WIRED);
		infile=fopen(sfilepath,"r");
		fscanf(infile,"%ld\n",&lupload);
		fclose(infile);
		sprintf(sfilepath,FILE_DOWNLOAD,WIRED);
		infile=fopen(sfilepath,"r");
		fscanf(infile,"%ld\n",&ldownload);
		fclose(infile);
		luploadrate=(lupload-slup)/512;
		ldownloadrate=(ldownload-sldown)/512;
		len = sprintf(stat,NET_STR,luploadrate,ldownloadrate);
	}
	else if(is_up(WIRELESS) == 1)
	{
		sprintf(sfilepath,FILE_UPLOAD,WIRELESS);
		infile=fopen(sfilepath,"r");
		fscanf(infile,"%ld\n",&lupload);
		fclose(infile);
		sprintf(sfilepath,FILE_DOWNLOAD,WIRELESS);
		infile=fopen(sfilepath,"r");
		fscanf(infile,"%ld\n",&ldownload);
		fclose(infile);
		luploadrate=(lupload-slup)/512;
		ldownloadrate=(ldownload-sldown)/512;
		len = sprintf(stat,WIFI_STR,luploadrate,ldownloadrate);
	}
	else
	{
		len = sprintf(stat,NET_DOWN_STR,lupload,ldownload);
	}
	slup=lupload;
	sldown=ldownload;
	return len;
}	

/* Set todo number */
int todo(char *stat)
{
	int len,itasks=0;
	char *buf;
	int bufsize =1000;
	FILE *infile;

	buf= (char *) calloc(bufsize,1);
	
	infile=fopen(TODO_FILE,"r");
	while(fgets(buf,bufsize,infile))
	{
		itasks++;
	}
	fclose(infile);
	free(buf);
	
	len=sprintf(stat,TODO_STR,itasks);

	return len;
}

/* Set mount drive notification */
int mount(char *stat)
{
	int len,imount=0;
	DIR *dir=NULL;
	struct dirent *rf=NULL;

	/* Open media directory */
	dir=opendir(MOUNT_DIR);

	/*Read media directory to check mounted media */
	while ((rf=readdir(dir))!=NULL)
	{
		/*Ignore . and .. directories */
		if (strcmp(rf->d_name,".") != 0 && strcmp(rf->d_name,"..") != 0)
			imount++;
	}

	/* Close media */
	closedir(dir);

	if (imount == 0)
		len = 0;
	else
		len=sprintf(stat,MOUNT_STR,imount);

	return len;
}

/* Release number */
int os_kernel(char *stat)
{
	int len;
	char *buf;
	int bufsize =1000;
	FILE *infile;

	buf = (char *) calloc(bufsize,1);
	
	infile=fopen(KERNELOS,"r");
	fscanf(infile,"%s\n",buf);
	fclose(infile);
	len=sprintf(stat,KERNEL_STR,buf);
	free(buf);

	return len;
}

/*Mail notification via IMAP protocol*/
int mailImap(char *stat)
{
	MailFunction mailFunction;
	int fd, err, len = 0;
	unsigned int newMail, nbBox;
	char buf[BUF_SIZE];
	char protCtl[128];
	
	/*Loop on all mailbox*/
	for (nbBox = 0; nbBox < LENGTH(boxes); nbBox++)
	{
		/*Set functions to use depending on the mailbox protocol*/

  		if (boxes[nbBox].protocol == imap)
		{
			mailFunction.mailInit = mail_socket_init;
			mailFunction.mailRead = mail_socket_read;
			mailFunction.mailWrite = mail_socket_write;
			mailFunction.mailClose = mail_socket_close;
		}
		else if (boxes[nbBox].protocol == imaps )
		{
			mailFunction.mailInit = mail_ssl_init;
			mailFunction.mailRead = mail_ssl_read;
			mailFunction.mailWrite = mail_ssl_write;
			mailFunction.mailClose = mail_ssl_close;
		}
		else
		{
			return -1;
		}

		/*Init environment to communicate with the server*/
		fd = mailFunction.mailInit(&boxes[nbBox]);

		/*Login to account*/
		sprintf (buf, "a001 LOGIN %s %s\r\n", boxes[nbBox].user, boxes[nbBox].passwd);
		mailFunction.mailWrite (buf, boxes[nbBox]);
		strcpy (protCtl,"a001 ");
		do
		{
			err = mailFunction.mailRead (buf, protCtl, boxes[nbBox]);
		} while (err == 1);
		if (strstr (buf,"a001 OK") == NULL)
		{
			return -1;
		};
		
		/*Ask status to mailbox*/
		sprintf (buf,"a002 STATUS %s (UNSEEN)\r\n",boxes[nbBox].inbox);
		mailFunction.mailWrite (buf, boxes[nbBox]);
		strcpy (protCtl,"a002 ");

		/*Reinit buffer before reading status*/
		buffer_init (buf);
		do
		{
			err = mailFunction.mailRead (buf, protCtl, boxes[nbBox]);
			sscanf(buf,"* STATUS %*s (UNSEEN %u)", &newMail);
		} while (err == 1);
		if (strstr (buf,"a002 OK") == NULL)
		{
			return -1;
		};
		/*Reinit buffer before reading status*/
		buffer_init (buf);
		/*Logout from account*/
		sprintf (buf,"a003 LOGOUT\r\n");
		mailFunction.mailWrite (buf, boxes[nbBox]);
		strcpy (protCtl,"a003 ");
		do
		{
			err = mailFunction.mailRead (buf, protCtl, boxes[nbBox]);
		} while (err == 1);
		if (strstr (buf,"a003 OK") == NULL)
		{
			return -1;
		};
		/*Close and free environment*/
		mailFunction.mailClose (fd, boxes[nbBox]);
		if (newMail > 0)
		{
		switch (nbBox)
		{
				case 0:
					len = sprintf (stat+len, MAIL_STR_0, newMail);
					break;
				case 1:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_1, newMail);
					break;
				case 2:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_2, newMail);
					break;
				case 3:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_3, newMail);
					break;
			}
		}
	}

	return 0;

}

int mail_socket_init (Box *boxes)
{
	int fd;
	FILE *fp;

	fd = sock_connect (boxes->serverName, boxes->port);
	if (fd == -1)
	{
		return -1;
	}
	fp = fdopen (fd, "r+");
	if (fp == NULL)
	{
		perror ("dwmstatus error opening file");
		return -1;
	}
	boxes->comm.f = fp;
	
	return fd;
}

int mail_socket_write (char *buf, Box boxes)
{
	int size, err;

	size = fprintf (boxes.comm.f, "%s", buf);
	if (size < 0)
	{
		perror ("dwmstatus error writing");
		return -1;
	}

	err = fflush (boxes.comm.f);
	if (err != 0)
	{
		perror ("dwmstatus error flushing");
		return -1;
	}

	return size;
}

int mail_socket_read (char *buf, char *ctl, Box boxes)
{
	int err, len = 0;

	buf[0] = '\0';

	do
	{
		len = strlen (buf);
		err = fflush (boxes.comm.f);
		if (err != 0)
		{
			perror ("dwmstatus error flushing");
			return -1;
		}
		
		if (fgets (buf+len, BUF_SIZE, boxes.comm.f) == NULL)
		{
			perror ("dwmstatus error reading");
			return -1;
		}
	} while ((strstr(buf+len,ctl) == NULL) && (len < BUF_SIZE));

	return 0;
}

int mail_socket_close (int fd, Box boxes)
{
	int err;

	err = fclose (boxes.comm.f);
	if (err !=0)
	{
		perror ("dwmstatus error closing");
		return -1;
	}
	err = close (fd);
	if (err == -1)
	{
		perror ("dwmstatus error closing");
		return -1;
	}

	return err;
}

int mail_ssl_init (Box *boxes)
{
	int fd, err, type;
	unsigned int status;
	gnutls_certificate_credentials_t xcred;
	gnutls_session_t session;
	gnutls_datum_t out;

	/*Init SSL session and certificates */
	gnutls_global_init();
	gnutls_certificate_allocate_credentials (&xcred);
  	gnutls_certificate_set_x509_trust_file (xcred, CAFILE, GNUTLS_X509_FMT_PEM);
	gnutls_init (&session, GNUTLS_CLIENT);

	/*Create kernel socket to use for SSL session*/
	fd = sock_connect (boxes->serverName, boxes->port);

	/*Initialise transport layer for SSL connection*/
	gnutls_transport_set_ptr (session, (gnutls_transport_ptr)fd);
  	gnutls_set_default_priority (session);
  	gnutls_protocol_set_priority (session, protocol_priority);
	gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);
	/*SSL handshake to negociate connection with server*/
	do
	{
	    err = gnutls_handshake (session);
    	} while (err < 0 && gnutls_error_is_fatal (err) == 0);
  
	/*Check server credentials*/
	gnutls_certificate_verify_peers3 (session, boxes->serverName, &status);
	type = gnutls_certificate_type_get (session);
	gnutls_certificate_verification_status_print( status, type, &out, 0);
	gnutls_free(out.data);

	boxes->comm.s.session = session;
	boxes->comm.s.xcred = xcred;

	return fd;
}

int mail_ssl_write (char *buf, Box boxes)
{
	int len, err, sent = 0;

	len = strlen(buf);
	do
	{
		err = gnutls_record_send (boxes.comm.s.session, buf+sent, len-sent);
		sent+=err;
	}
	while (sent < len);

	return len;
}

int mail_ssl_read (char *buf, char *ctl, Box boxes)
{
	int err;
	int len = 0;

	do
	{
		err = gnutls_record_recv (boxes.comm.s.session,buf+len,BUF_SIZE);
		len = strlen (buf);
	}
	while ((err == GNUTLS_E_AGAIN || err == GNUTLS_E_INTERRUPTED || strstr(buf,ctl) == NULL) && len < BUF_SIZE);

	return 0;
}

int mail_ssl_close (int fd, Box boxes)
{
	gnutls_certificate_credentials_t xcred;

	/*End SSL connection*/
	gnutls_bye (boxes.comm.s.session, GNUTLS_SHUT_RDWR);
	gnutls_deinit (boxes.comm.s.session);
	gnutls_certificate_free_credentials (boxes.comm.s.xcred);
	gnutls_global_deinit ();
	close (fd);

	return 0;
}

int sock_connect (char *hostname, int port)
{
  struct hostent *host;
  struct sockaddr_in addr;
  int fd, i;

  host = gethostbyname (hostname);
  if (host == NULL)
    {
      errno = h_errno;
      perror("dwmstatus gethostbyname");
      return (-1);
    };

  fd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    {
      perror("dwmstatus error opening socket");
      return (-1);
    };

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = *(unsigned long *) host->h_addr_list[0];
  addr.sin_port = htons (port);
  i = connect (fd, (struct sockaddr *) &addr, sizeof (struct sockaddr));
  if (i == -1)
    {
      perror("dwmstatus error connecting");
      close (fd);
      return (-1);
    };
  return (fd);
}

int buffer_init(char *buf)
{
	int i;

	for (i=0; i < BUF_SIZE; i++)
	{
		buf[i] = '\0';
	}

	return 0;
}
