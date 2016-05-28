/*Include libraries*/
#include <unistd.h>
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#undef _POSIX_C_SOURCE
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/statvfs.h>
#include <sys/types.h>

/* Macros definition */
#define LENGTH(X) (sizeof X / sizeof X[0])
#define BUF_SIZE (2048)

/* structures declaration*/
typedef struct {
	int (*func)(char *);
	int refresh;
	char * const *cmd;
} Stbar;

typedef struct
{
	char mailPath[58];
} Maildir;

/* Functions declaration */
static int alsa_sound(char *stat);
static int bat(char *stat);
static int mktimes(char *stat);
static int proc(char *stat);
static int mem(char *stat);
static int is_up(char *device);
static int net(char *stat);
static int mount(char *stat);
static int os_kernel(char *stat);
static int temp(char *stat);
static int runevery(time_t *ltime, int sec);
static int get_freespace(char *mntpt);
static int disk(char *stat);
static int mailCount(char *stat);
static int task(char *stat);
static pid_t proc_find(const char* name);

/*Your configuration*/
#include <dwmstbar.h>

/*Main function*/
int main() {
	Display *dpy;
	Window root;
	char statnext[100]; 
	char status[300];
	unsigned int i;
	time_t lastrefresh[LENGTH(stbar)]={0};
	char statbck[LENGTH(stbar)][100]; 
	for (i=0; i < LENGTH(stbar); i++)
	{
		statbck[i][0]='\0';
	}

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
				if (stbar[i].func(statnext) >= 0)
				{
					strcat(status,statnext);
					statbck[i][0]='\0';
					strcat(statbck[i],statnext);
					#ifdef DEBUG
						printf("line %i : %s",i,(char *)statnext);
					#endif
					statnext[0] = '\0';
				}
//				else
//					strcat(status,statbck[i]);
			}
			else
			{
				strcat(status,statbck[i]);
				#ifdef DEBUG
					printf("line %i : %s",i,(char *)statbck[i]);
				#endif
			}
		}
		#ifdef DEBUG
			printf("status : %s", (char *)status);
		#endif
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
		return 1;
	}
	else
		return 0;
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
        {
		realvol = (vol*100)/max;
		len = sprintf(stat, VOL_STR, realvol);
        }

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
	static long lnum[5][2] = {{0}};
	static int id = 0, batime = 0;
	long lmaxbat;
	int len;
	char stattmp[50];

	stattmp[0] = '\0';
	/* Power / Battery */
	infile = fopen(BATT_NOW,"r");
	fscanf(infile,"%ld\n",&lnum[(id+1)%5][0]);
	fclose(infile);
	infile = fopen(BATT_STAT,"r");
	fscanf(infile,"%s\n",stattmp); 
	fclose(infile);
	infile = fopen(BATT_FULL,"r");
	fscanf(infile,"%ld\n",&lmaxbat);
	fclose(infile);
	if (lnum[id][0] != lnum[(id+1)%5][0])
	{
		batime = (lnum[0][1]+lnum[1][1]+lnum[2][1]+lnum[3][1]+lnum[4][1]+1) * lnum[(id+1)%5][0]/(60*(lnum[(id+2)%5][0]-lnum[(id+1)%5][0]));
		id = (id+1)%5;
		lnum[(id+1)%5][1] = 0;
	}
	else
		lnum[(id+1)%5][1]++;
	if (strncmp(stattmp,"Charging",8) == 0) 
	{
		len = sprintf(stat,BAT_CHRG_STR,100*lnum[(id+1)%5][0]/lmaxbat);
	}
	else if (strncmp(stattmp,"Full",8) == 0) 
	{
		stat[0] = '\0';
		len = 0;
	}
	else 
	{
		if (batime <  BATT_LOW)
			len = sprintf(stat,BAT_LOW_STR,batime);
		else
			len = sprintf(stat,BAT_STR,batime);
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
	long lnum1,lnum2,lnum3,lnum4,lnum5,lnum6,spare;

	infile=fopen(MEM_FILE,"r");
	fscanf(infile,"MemTotal: %ld kB\nMemFree: %ld kB\nMemAvailable:	%ld kB\nBuffers: %ld kB\nCached: %ld kB\n",&lnum1,&lnum2,&spare,&lnum3,&lnum4);
	fseek(infile,392,SEEK_SET);
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

/* Set task number */
int task(char *stat)
{
	int len;
	char *buf;
	int bufsize = 10;
	FILE *cmd;

	buf= (char *) calloc(bufsize,3);
	
        /* Execute tasks -DELETE count to know the number of undeleted tasks */
	cmd=popen(TASK_CMD,"r");
        /* Check if there is no fail in the popen call */
        if (cmd == NULL)
        {
            return 0;
        }
	if (fgets(buf,bufsize-1,cmd) == NULL)
        {
            return 0;
        }
	pclose(cmd);
        len=sprintf(stat,TASK_STR, buf[0]);
	free(buf);

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

	if (dir != NULL)
        {
            /*Read media directory to check mounted media */
            while ((rf=readdir(dir))!=NULL)
	    {
		/*Ignore . and .. directories */
		if (strcmp(rf->d_name,".") != 0 && strcmp(rf->d_name,"..") != 0)
			imount++;
	    }

	    /* Close media */
	    closedir(dir);
        }

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

/* Current temperature */
int temp(char *stat)
{
	int len;
	int temp;
	FILE *infile;

	infile=fopen(TEMPFILE,"r");
	fscanf(infile,"%d\n",&temp);
	fclose(infile);
	len=sprintf(stat,TEMP_STR,temp/1000);

	return len;

}

int get_freespace(char *mntpt)
{
    struct statvfs data;
    double total, used = 0;

    if ( (statvfs(mntpt, &data)) < 0)
    {
		fprintf(stderr, "can't get info on disk.\n");
		return 0;
    }
    total = (data.f_blocks * data.f_frsize);
    used = (data.f_blocks - data.f_bfree) * data.f_frsize ;
    return used/total*100;
}

int disk(char *stat)
{
	int len;

	len=sprintf(stat,SIZE_STR,get_freespace("/"),get_freespace("/home"));

	return len;
}

/*Mail notification through maildir*/
int mailCount(char *stat)
{
	int newMail, nbMaildir, len = 0, envl = 0;
	DIR* dir = NULL;
	struct dirent* rf = NULL;
	pid_t lpid;
	
	for (nbMaildir =0; nbMaildir < LENGTH(maildirs); nbMaildir++ )
	{
		newMail = 0;
		dir = opendir(maildirs[nbMaildir].mailPath); /* try to open directory */
		if (dir == NULL)
		{
			perror("No maildir directory");
		}
		while ((rf = readdir(dir)) != NULL) /*count number of file*/
		{
			if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0)
			{
				newMail++;
			}
		}
		closedir(dir);
		
		/* Check if offlineimap is running */
		lpid = proc_find("offlineimap");
		/* If not running, display the mail in red */
		if (lpid < 0 && envl == 0)
		{
			len = sprintf (stat+len, MAIL_STR_D);
			envl = 1;
		}
	
		if (newMail > 0)
		{
			if (envl == 0)
			{
				len = sprintf (stat+len, MAIL_STR_0);
				envl = 1;
			}
			switch (nbMaildir)
			{
				case 0:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_1, newMail);
					break;
				case 1:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_2, newMail);
					break;
				case 2:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_3, newMail);
					break;
				case 3:
					len = strlen (stat);
					len = sprintf (stat+len, MAIL_STR_4, newMail);
					break;
			}
		}
	}
	return len;
}

pid_t proc_find(const char* name) 
{
	DIR* dir;
	FILE *fp;
	struct dirent* ent;
	char *endptr, *first;
	char buf[512];
	long lpid;
    
	if (!(dir = opendir("/proc"))) 
	{
		perror("can't open /proc");
		return -1;
	}

	while((ent = readdir(dir)) != NULL) 
	{
		/* if endptr is not a null character, the directory is not entirely numeric, so ignore it */
		lpid = strtol(ent->d_name, &endptr, 10);
		if (*endptr != '\0') 
		{
			continue;
		}

		/* try to open the cmdline file */
		sprintf(buf, "/proc/%ld/comm", lpid);
		fp = fopen(buf, "r");

		if (fp)
		{
			if (fgets(buf, sizeof(buf), fp) != NULL)
			{
				/* check the first token in the file, the program name */
				first = strtok(buf,"\n");    
				if (!strcmp(first, name))
				{
					fclose(fp);
					closedir(dir);
					return (pid_t)lpid;
				}
			}
			fclose(fp);
		}
	}
	closedir(dir);
	return -1;
}
