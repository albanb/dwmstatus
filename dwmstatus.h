#define BATT_LOW		15			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL		1			// Sleeps for INTERVAL seconds between updates
#define CPU_HI			80			// Above CPU_HIGH, cpu display turns red
#define	WIRELESS		"wlp3s0"
#define WIRED			"enp10s0"

/* Files to read system information */
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
#define CPU_FILE		"/proc/stat"
#define MEM_FILE		"/proc/meminfo"
#define FILE_UPLOAD		"/sys/class/net/%s/statistics/tx_bytes"
#define FILE_DOWNLOAD		"/sys/class/net/%s/statistics/rx_bytes"
#define FILE_OPER		"/sys/class/net/%s/operstate"
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
