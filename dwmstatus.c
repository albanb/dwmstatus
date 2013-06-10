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
#include <dwmstatus.h>

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
static int notifications(char *stat);
static int runevery(time_t *ltime, int sec);

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
		len=sprintf(stat,MEM_SWAP_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M',lnum1/1024,'M');
	else
		len=sprintf(stat,MEM_STR,(lnum1-(lnum2+lnum3+lnum4))/1024,'M',lnum1/1024,'M');

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
