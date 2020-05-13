#ifndef DWMSTATUSDEF
#define DWMSTATUSDEF

//#define DEBUG

/* Files to read system information */
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
#define CPU_FILE		"/proc/stat"
#define MEM_FILE		"/proc/meminfo"
#define FILE_UPLOAD		"/sys/class/net/%s/statistics/tx_bytes"
#define FILE_DOWNLOAD		"/sys/class/net/%s/statistics/rx_bytes"
#define FILE_OPER		"/sys/class/net/%s/operstate"
#define TASK_CMD                "task -DELETED count"
#define MOUNT_DIR		"/media"
#define KERNELOS		"/proc/sys/kernel/osrelease"
#define CAFILE 			"/etc/ssl/certs/ca-certificates.crt"
#define TEMPFILE		"/sys/class/hwmon/hwmon0/temp1_input"
#define VOL_CH			"Master"	// Channel to watch for volume
#define	WIRELESS		"wlp3s0"
#define WIRED			"enp10s0"
#define BATT_LOW		15			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL		1			// Sleeps for INTERVAL seconds between updates
#define CPU_HI			80			// Above CPU_HIGH, cpu display turns red

/* Strings format for information display in the status bar */
#define VOL_STR			"[2]\uE05D[1]%d%%"			// Volume
#define VOL_MUTE_STR		"[3]\uE04F %d%%"			// Volume
#define BAT_STR			"[4]\uE032 %dmin"			// Battery, BAT, above BATT_LOW percentage
#define BAT_LOW_STR		"[3]\uE031 %dmin"			// Battery, BAT, below BATT_LOW percentage
#define BAT_FULL_STR		"[2]\uE033[1]%d%%"			// Battery, full
#define BAT_CHRG_STR		"[5]\uE032 %ld%%"			// Battery, AC
#define CPU_STR			"[2]\uE026[1]%d%%"			// CPU percent
#define CPU_HI_STR		"[3]\uE026 %d%%"			// CPU percent when above CPU_HI%
#define MEM_STR			"[2]\uE020[1]%ld%ciB"		// MEM percent
#define MEM_SWAP_STR		"[4]\uE020 %ld%ciB"		// MEM percent
#define NET_STR			"[2]\uE060[1]%ldKbp/s[2]\uE061[1]%ldKbp/s"	//NET oper string up and down
#define WIFI_STR		"[2]\uE02D \uE060[1]%ldKbp/s[2]\uE061[1]%ldKbp/s"	//NET down string up and down
#define NET_DOWN_STR		"[3]\uE060 %ldKbp/s \uE061 %ldKbp/s"	//NET down string up and down
#define DATE_TIME_STR		"[2]\uE015[1]%a %d %b %Y %H:%M"		// This is a strftime format string which is passed localtime
#define TEMP_STR		"[2]\uE01D[1]%d°C"			// Temperature
#define SIZE_STR		"[2]\uE077[1]/ %d%% \uE10e %d%%"			// Size
#define TASK_STR		"[2]\uE01F[1]%c"			// Task string
#define MOUNT_STR		"[2]\uE00C[1]%d"			// Mount string
#define KERNEL_STR		"[2]\uE00E[1]%s"			// Kernel string
#define MAIL_STR_0		"[2]\uE072"			// Mail notification string
#define MAIL_STR_D		"[3]\uE072 "		// Mail notification string when no mail check
#define MAIL_STR_1		"[1] y%d"			// Mail notification string
#define MAIL_STR_2		"[1] s%d"			// Mail notification string
#define MAIL_STR_3		"[1] f%d"			// Mail notification string
#define MAIL_STR_4		"[1] g%d"			// Mail notification string

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
	{ "/home/alban/.cache/maildir/gmail/INBOX/new" },
};
#endif
