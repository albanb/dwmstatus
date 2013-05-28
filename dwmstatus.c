/*Include libraries*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <dirent.h>

#define BATT_LOW		15			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL		1			// Sleeps for INTERVAL seconds between updates
#define CPU_HI			80			// Above CPU_HIGH, cpu display turns red

/* Files to read system information */
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
#define CPU_FILE		"/proc/stat"
#define MEM_FILE		"/proc/meminfo"
#define NET_FILE_UPLOAD		"/sys/class/net/enp10s0/statistics/tx_bytes"
#define NET_FILE_DOWNLOAD	"/sys/class/net/enp10s0/statistics/rx_bytes"
#define NET_FILE_OPER		"/sys/class/net/enp10s0/operstate"
#define WIFI_FILE_UPLOAD	"/sys/class/net/wlp3s0/statistics/tx_bytes"
#define WIFI_FILE_DOWNLOAD	"/sys/class/net/wlp3s0/statistics/rx_bytes"
#define WIFI_FILE_OPER		"/sys/class/net/wlp3s0/operstate"
#define TODO_FILE		"/home/alban/.local/share/todo/todo.txt"
#define MOUNT_DIR		"/media"
#define KERNELOS		"/proc/sys/kernel/osrelease"
#define NOTIFILE		"/home/alban/.local/share/dwm/notification"

#define VOL_CH			"Master"	// Channel to watch for volume

/* Strings format for information display in the status bar */
#define VOL_STR			"\x01\uE05D %d%%"			// Volume
#define VOL_MUTE_STR		"\x05\uE04F %d%%"			// Volume
#define BAT_STR			"\x04\uE032 %d%%"			// Battery, BAT, above BATT_LOW percentage
#define BAT_LOW_STR		"\x03\uE031 %d%%"			// Battery, BAT, below BATT_LOW percentage
#define BAT_FULL_STR		"\x01\uE033 %d%%"			// Battery, full
#define BAT_CHRG_STR		"\x05\uE032 %d%%"			// Battery, AC
#define CPU_STR			"\x01\uE026 %d%%"			// CPU percent
#define CPU_HI_STR		"\x03\uE026 %d%%"			// CPU percent when above CPU_HI%
#define MEM_STR			"\x01\uE020 %ld%ciB/%ld%ciB"		// MEM percent
#define MEM_SWAP_STR		"\x04\uE020 %ld%ciB/%ld%ciB"		// MEM percent
#define NET_STR			"\x01\uE060 %ldKbp/s \uE061 %ldKbp/s"	//NET oper string up and down
#define WIFI_STR		"\x01\uE02D \uE060 %ldKbp/s \uE061 %ldKbp/s"	//NET down string up and down
#define NET_DOWN_STR		"\x03\uE060 %ldKbp/s \uE061 %ldKbp/s"	//NET down string up and down
#define DATE_TIME_STR		"\x01\uE015 %a %d %b %Y %H:%M"		// This is a strftime format string which is passed localtime
#define TODO_STR		"\x01\uE01F %d"			// Todo string
#define MOUNT_STR		"\x01\uE00C %d"			// Mount string
#define KERNEL_STR		"\x01\uE00E %s"			// Kernel string
#define TORRENT_STR		"\x01\uE065 %d"			// Torrent string
#define REMIND_STR		"\x01\uE01C %d"			// Reminder string

/* Macros definition */
#define LENGTH(X) (sizeof X / sizeof X[0])

/* structures declaration*/
typedef struct {
	int (*func)(char *);
	int refresh;
	char * const *cmd;
} Stbar;

typedef struct {
	char *cmp;
	char *chaine;
} Notification;

/* Functions declaration */
static int alsa_sound(char *stat);
static int bat(char *stat);
static int mktimes(char *stat);
static int proc(char *stat);
static int mem(char *stat);
static int net(char *stat);
static int todo(char *stat);
static int mount(char *stat);
static int os_kernel(char *stat);
static int notifications(char *stat);
//static void spawn(char * const *arg);
static int runevery(time_t *ltime, int sec);

//static char * const video[]  = { "vlc", NULL };

static Stbar stbar[] = {
	{ notifications, 60, NULL },
	{ mount, 1, NULL },
	{ todo, 60, NULL },
	{ os_kernel, 60, NULL },
	{ bat, 1, NULL },
	{ proc, 1, NULL },
	{ mem, 1, NULL },
	{ net, 1, NULL },
	{ alsa_sound, 1, NULL },
	{ mktimes, 1, NULL },
};

static Notification notification[] = {
	{"torrent;", TORRENT_STR },
	{"cal;", REMIND_STR },
};
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
	for (;;) {
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
		len = sprintf(stat, VOL_MUTE_STR, 10);
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
		len=sprintf(stat,MEM_SWAP_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M',lnum1/1024,'M');
	else
		len=sprintf(stat,MEM_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M',lnum1/1024,'M');

	return len;
}

/*Set net information */
int net(char *stat)
{
	int len;
	FILE *infile;
	static long slup=0,sldown=0;
	long lnum1,lnum2,lnum1net,lnum2net,lnum1wifi,lnum2wifi,luploadnet,ldownloadnet,luploadwifi,ldownloadwifi;
	char netstattmp[50],wifistattmp[50];

	infile=fopen(NET_FILE_UPLOAD,"r");
	if (infile != NULL)
	{
		fscanf(infile,"%ld\n",&lnum1net);
		fclose(infile);
		infile=fopen(NET_FILE_DOWNLOAD,"r");
		fscanf(infile,"%ld\n",&lnum2net);
		fclose(infile);
		infile=fopen(NET_FILE_OPER,"r");
		fscanf(infile,"%s\n",netstattmp);
		fclose(infile);
		luploadnet=(lnum1net-slup)/512;
		ldownloadnet=(lnum2net-sldown)/512;
	}
	else
	{
		netstattmp[0] ='\0';
	}

	infile=fopen(WIFI_FILE_OPER,"r");
	if (infile != NULL)
	{
		fscanf(infile,"%s\n",wifistattmp);
		fclose(infile);
		infile=fopen(WIFI_FILE_UPLOAD,"r");
		fscanf(infile,"%ld\n",&lnum1wifi);
		fclose(infile);
		infile=fopen(WIFI_FILE_DOWNLOAD,"r");
		fscanf(infile,"%ld\n",&lnum2wifi);
		fclose(infile);
		luploadwifi=(lnum1wifi-slup)/512;
		ldownloadwifi=(lnum2wifi-sldown)/512;
	}
	else
	{
		wifistattmp[0]='\0';
	}

	if ((strncmp(netstattmp,"down",4) == 0) && (strncmp(wifistattmp,"down",4) == 0))
	{
		lnum1=0;
		lnum2=0;
		len=sprintf(stat,NET_DOWN_STR,luploadnet,ldownloadnet);
	}
	else if (strncmp(netstattmp,"up",2) == 0)
	{
		lnum1=lnum1net;
		lnum2=lnum2net;
		len=sprintf(stat,NET_STR,luploadnet,ldownloadnet);
	}
	else if (strncmp(wifistattmp,"up",2) == 0)
	{
		lnum1=lnum1wifi;
		lnum2=lnum2wifi;
		len=sprintf(stat,WIFI_STR,luploadwifi,ldownloadwifi);
	}
	else
	{
		lnum1=0;
		lnum2=0;
		len=sprintf(stat,NET_DOWN_STR,luploadnet,ldownloadnet);
	}

	slup=lnum1;
	sldown=lnum2;

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

/*
void spawn(char * const *arg) {
	if(fork() == 0) {
		setsid();
		execvp(arg[0], arg);
		fprintf(stderr, "dwm: execvp %s", arg[0]);
	}
}*/

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

/* Notifications */
int notifications(char *stat)
{
	int len, lentmp, i, inb[LENGTH(notification)]={0};
	char *buf, *bufsuivre, *statn;
	FILE *infile;
	int bufsize = 1000;

	statn = stat;
	buf = (char *) calloc(bufsize,1);
	infile = fopen(NOTIFILE,"r");
	while(fgets(buf,bufsize,infile))
	{
		for (i = 0; i < LENGTH(notification); i++)
		{
			bufsuivre=strstr(buf,notification[i].cmp);
			if (bufsuivre != NULL)
			{
				inb[i]++;
			}
		}
	}
	for (i = 0; i < LENGTH(notification); i++)
	{
		if ( inb[i] != 0)
		{
			lentmp = sprintf(statn,notification[i].chaine,inb[i]);
			len += lentmp;
			statn = statn + lentmp;
		}
	}
	fclose(infile);

	return len;
}
