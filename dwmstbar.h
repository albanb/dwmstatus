#ifndef DWMSTATUSDEF
#define DWMSTATUSDEF

//#define DEBUG

/* Files to read system information */
#define BATT_NOW		    "/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		    "/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		    "/sys/class/power_supply/BAT0/status"
#define CPU_FILE    		"/proc/stat"
#define MEM_FILE    		"/proc/meminfo"
#define FILE_UPLOAD	    	"/sys/class/net/%s/statistics/tx_bytes"
#define FILE_DOWNLOAD   	"/sys/class/net/%s/statistics/rx_bytes"
#define FILE_OPER	    	"/sys/class/net/%s/operstate"
#define TASK_CMD            "todo.sh -d /home/alban/.config/todo/config count"
#define MOUNT_DIR	    	"/run/media/alban"
#define KERNELOS	    	"/proc/sys/kernel/osrelease"
#define CAFILE 		    	"/etc/ssl/certs/ca-certificates.crt"
#define TEMPFILE	    	"/sys/class/hwmon/hwmon0/temp1_input"
#define VOL_CH		    	"Master"	// Channel to watch for volume
#define	WIRELESS	     	"wlp3s0"
#define WIRED		    	"enp10s0"
#define BATT_LOW	    	15			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL	    	1			// Sleeps for INTERVAL seconds between updates
#define CPU_HI			    80			// Above CPU_HIGH, cpu display turns red

/* Strings format for information display in the status bar */
#define VOL_STR			    "\x02\uE05D\x01%d%%"			// Volume
#define VOL_MUTE_STR	    "\x04\uE04F %d%%"			// Volume
#define BAT_STR			    "\x03\uE032 %dmin"			// Battery, BAT, above BATT_LOW percentage
#define BAT_LOW_STR		    "\x04\uE031 %dmin"			// Battery, BAT, below BATT_LOW percentage
#define BAT_FULL_STR		"\x02\uE033\x01%d%%"			// Battery, full
#define BAT_CHRG_STR		"\x05\uE032 %ld%%"			// Battery, AC
#define CPU_STR			    "\x02\uE026\x01%d%%"			// CPU percent
#define CPU_HI_STR		    "\x04\uE026 %d%%"			// CPU percent when above CPU_HI%
#define MEM_STR			    "\x02\uE020\x01%ld%ciB"		// MEM percent
#define MEM_SWAP_STR		"\x03\uE020 %ld%ciB"		// MEM percent
#define NET_STR			    "\x02\uE060\x01%ldKbp/s\x02\uE061\x01%ldKbp/s"	//NET oper string up and down
#define WIFI_STR		    "\x02\uE02D \uE060\x01%ldKbp/s\x02\uE061\x01%ldKbp/s"	//NET down string up and down
#define NET_DOWN_STR		"\x04\uE060 %ldKbp/s \uE061 %ldKbp/s"	//NET down string up and down
#define DATE_TIME_STR		"\x02\uE015\x01%a %d %b %Y %H:%M"		// This is a strftime format string which is passed localtime
#define TEMP_STR	    	"\x02\uE01D\x01%d°C"			// Temperature
#define SIZE_STR	    	"\x02\uE077\x01/ %d%% \uE10e %d%%"			// Size
#define TASK_STR	    	"\x02\uE01F\x01%c"			// Task string
#define MOUNT_STR	    	"\x02\uE00C\x01%d"			// Mount string
#define KERNEL_STR	    	"\x02\uE00E\x01%s"			// Kernel string
#define MAIL_STR_0	    	"\x02\uE072"			// Mail notification string
#define MAIL_STR_D	    	"\x03\uE072 "		// Mail notification string when no mail check
#define MAIL_STR_1	    	"\x01 y%d"			// Mail notification string
#define MAIL_STR_2	    	"\x01 s%d"			// Mail notification string
#define MAIL_STR_3	    	"\x01 f%d"			// Mail notification string
#define MAIL_STR_4	    	"\x01 g%d"			// Mail notification string

static Stbar stbar[] = {
	{ mount, 1, NULL },
	{ mailCount, 10, NULL },
	{ task, 60, NULL },
	{ os_kernel, 60, NULL },
	{ disk, 1, NULL },
	{ bat, 1, NULL },
	{ proc, 1, NULL },
	{ mem, 1, NULL },
	{ temp, 1, NULL },
	{ net, 1, NULL },
	{ alsa_sound, 1, NULL },
	{ mktimes, 1, NULL },
};

Maildir maildirs[] = {
	{ "/home/alban/.cache/maildir/yahooalban/Inbox/new" },
	{ "/home/alban/.cache/maildir/freebinmail/INBOX/new" },
	{ "/home/alban/.cache/maildir/freealban/INBOX/new" },
};
#endif
