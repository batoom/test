#include <stdio.h>
#define LOG_TAG "RzSensD"
#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/Timers.h>
#include <utils/threads.h>
#include <utils/Errors.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

#include <string.h>
#include <stdint.h>
#include <dirent.h>
#ifdef HAVE_INOTIFY
# include <sys/inotify.h>
#endif
#ifdef HAVE_ANDROID_OS
# include <sys/limits.h>        /* not part of Linux */
#endif
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <hardware/sensors.h>
#include <cutils/sockets.h>
#include <map>

#define HMC5883
#ifdef HMC5883
#include <math.h>
extern "C"{
#include <HMSHardIronCal.h>
#include <HMSHeading.h>
}
#define PI 						3.1415926535897932f
#endif

using namespace std;


struct sensor_iio_t {
	const char *classic_name;
	const char *dev_path;
	void (*handle_sensor_event)();
};
struct sensor_info_t {
	const char *classic_name;
	const char *dev_name;
	const char *dev_path;
	int fd;
	void (*handle_sensor_event)(int fd);
};

#define fatal() do {									\
		LOGE("Error: %s %d\n", __FUNCTION__, __LINE__);	\
		exit(-1);										\
	} while (0)




static int g_timer_triggered;
static int g_accl[3];
static int g_lux;
static bool g_lux_updated;

static void handle_acceleration(int fd)
{
	struct input_event iev;
	if (read(fd, &iev, sizeof(iev)) != sizeof(iev)) {
		return;
	}
	if (iev.type == EV_ABS && iev.code < 3) {
		g_accl[iev.code] = iev.value;
	}
}

static void handle_light_sensor();
static void handle_compass_sensor();
sensor_iio_t sensors_iio_info [] = {
	{
		"light",
		"/sys/devices/platform/tegra_i2c.0/i2c-0/0-0044/iio/device0/lux",
		handle_light_sensor,
	},
	{
		//"magnetic-field",
		"orientation",
		"/dev/hmc5883",
		handle_compass_sensor,
	}
};

#define NUM_IIO_SENSORS (sizeof(sensors_iio_info) / sizeof(sensors_iio_info[0]))

static void handle_light_sensor()
{
	static bool got_path = false;
	static const char * lux_path = NULL;

	if (got_path == false) {
		got_path == true;
		for (int i=0; i < NUM_IIO_SENSORS; i++) {
			if (!strcmp(sensors_iio_info[i].classic_name, "light")) {
				lux_path = sensors_iio_info[i].dev_path;
				 break;
			 }
		 }
	 }

	 if (lux_path == NULL) {
		 LOGE("Error: no light sensor config at %s %d\n", __FUNCTION__, __LINE__);
		 return;
	 }

	 FILE* fp = fopen(lux_path, "r");
	 if (fp == NULL) {
		 LOGE("Error: open %s failed at %s %d\n", lux_path, __FUNCTION__, __LINE__);
		 return;
	 }

	 char buff[1024];
	 long int lux = 0;
	 if (fgets(buff, sizeof buff, fp) == NULL) {
		 LOGE("Error: no light sensor data at %s %d\n", __FUNCTION__, __LINE__); 
		 goto final;
	 } 

	 lux = strtol(buff, NULL, 0);
	 g_lux = lux;
	 g_lux_updated = true;
final:
	 fclose(fp);
}

struct compass_data_t {
	short x;
	short y;
	short z;
};

static struct compass_data_t g_compass_data;
static bool g_compass_updated;

static void handle_compass_sensor()
{
	static bool got_path = false;
	static const char * compass_path = NULL;

	if (got_path == false) {
		got_path == true;
		for (int i=0; i < NUM_IIO_SENSORS; i++) {
			//			if (!strcmp(sensors_iio_info[i].classic_name, "magnetic-field")) {
			if (!strcmp(sensors_iio_info[i].classic_name, "orientation")) {
				compass_path = sensors_iio_info[i].dev_path;
				 break;
			 }
		 }
	 }

	 if (compass_path == NULL) {
		 LOGE("Error: no light sensor config at %s %d\n", __FUNCTION__, __LINE__);
		 return;
	 }

	 FILE* fp = fopen(compass_path, "r");
	 if (fp == NULL) {
		 LOGE("Error: open %s failed at %s %d\n", compass_path, __FUNCTION__, __LINE__);
		 return;
	 }

	 compass_data_t compass_data;

	 if (fread(&compass_data, sizeof compass_data, 1, fp) != 1) {
		 LOGE("Error: no compass sensor data at %s %d\n", __FUNCTION__, __LINE__); 
		 goto final;
	 } 

	 g_compass_data = compass_data;
	 g_compass_updated = true;
final:
	 fclose(fp);
}

sensor_info_t sensors_info [] = {
	{
		"acceleration",
		"accelerometer_tegra",
		NULL,
		-1,
		handle_acceleration,
	},
};

#define NUM_SENSORS (sizeof(sensors_info)/sizeof(sensors_info[0]))

class dev_obj_t
{
public:
	int fd;
	typedef void (*handle_dev_t)(int fd);
	handle_dev_t handle_dev;
	dev_obj_t(int fd, handle_dev_t handle_dev) {
		this->fd = fd;
		this->handle_dev = handle_dev;
	}
};

static map<int, dev_obj_t*> g_dev_input;
static int open_device(const char *deviceName)
{
    int fd;
    char name[80];

    fd = open(deviceName, O_RDWR);
    if(fd < 0) {
        LOGE("could not open %s, %s\n", deviceName, strerror(errno));
        return -1;
    }

    name[sizeof(name) - 1] = '\0';
    if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
        name[0] = '\0';
    }

	for (unsigned int i=0; i < NUM_SENSORS; i++) {
		if (!strncmp(name, sensors_info[i].dev_name, strlen(sensors_info[i].dev_name))) {
			g_dev_input[fd] = new dev_obj_t(fd, sensors_info[i].handle_sensor_event);
			sensors_info[i].dev_path = strdup(deviceName);
			sensors_info[i].fd = fd;
			return 0;
		}
	}
	close(fd);
    return 0;
}

static int scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        open_device(devname);
    }
    closedir(dir);
    return 0;
}

static void list_sensors(FILE* fp)
{
	for (unsigned int i=0; i < NUM_SENSORS; i++)
	{
		if (sensors_info[i].fd >= 0) {
			fprintf(fp, "%s\n", sensors_info[i].classic_name);
		}
	}
	for (unsigned int i=0; i < NUM_IIO_SENSORS; i++) {
		fprintf(fp, "%s\n", (sensors_iio_info[i].classic_name));
	}
	fprintf(fp, "end-of-sensors\n");
}

class input_obj_t
{
public:
	int ifd;
	int ofd;
	int started;
	FILE* ifile;
	FILE* ofile;
public:
	~input_obj_t();
	input_obj_t(int ifd, FILE* ifile, int ofd, FILE* ofile);
};

class accept_obj_t
{
public:
	int fd;
	accept_obj_t(int fd) {
		this->fd = fd;
	}
};

static map<int, input_obj_t*> g_cmd_input;
static map<int, accept_obj_t*> g_accept_input;

static void init_timer(unsigned int ms)
{
	if (ms < 20) {
		ms = 20;
	}
	//assert(ms < 1000000);
	struct timeval tv = {0, ms * 1000};
	struct itimerval itv = {tv, tv};
	setitimer(ITIMER_REAL, &itv, NULL);
}

//get rid of the NL char at the end of a string
void chomp(char *buff)
{
	for (int i=0; buff[i]; i++) {
		if (buff[i] == '\n') {
			buff[i] = 0;
			break;
		}
	}
}

static void handle_commands(const int fd)
{

	LOGD("%s: %d: fd is %d", __FUNCTION__, __LINE__, fd);
	char buff[1024];
	if (fgets(buff, 1024, g_cmd_input[fd]->ifile) == NULL) {
		delete g_cmd_input[fd];
		g_cmd_input.erase(fd);

		if (g_cmd_input.empty() && g_accept_input.empty()) {
			LOGD("exit: no more input! %s: %d\n", __FUNCTION__, __LINE__); 
			exit(0);
		}
		return;
	}

	chomp(buff);
	
	LOGD("%s: %d: got cmd: %s", __FUNCTION__, __LINE__, buff); 
	int msec;
	if (sscanf(buff, "set-delay:%d", &msec) == 1) {
		if (msec < 20) {
			msec = 20;
		}
		init_timer(msec);
	} else if (!strcmp(buff, "list-sensors")) {
		FILE* fp = g_cmd_input[fd]->ofile;
		list_sensors(fp);
	} else if (!strcmp(buff, "start")) {
			g_cmd_input[fd]->started = 1;
	} else {
		LOGE("unknown command: %s %d", __FUNCTION__, __LINE__);
	}
}


static void alarm_handler(int host_signum)
{
	g_timer_triggered = 1;
}

static int64_t
data__now_ms(void)
{
    struct timespec  ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (int64_t)ts.tv_sec * 1000000 + ts.tv_nsec/1000;
}

int g_daemon_mode;
#define CONTROL_SOCKET_NAME "sensor_daemon"

static void setup_signal()
{
	struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = alarm_handler;
    sigaction(SIGALRM, &act, NULL);
}

struct pollfds_t {
	int nfds;
	int capcity;
	struct pollfd* pfds;
};

static int
fd_accept(int  fd)
{
    struct sockaddr  from;
    socklen_t        fromlen = sizeof(from);
    int              ret;

    do {
        ret = accept(fd, &from, &fromlen);
    } while (ret < 0 && errno == EINTR);

	LOGD("%s: %d ret is %d", __FUNCTION__, __LINE__, ret); 
    return ret;
}

input_obj_t::input_obj_t(int ifd, FILE* ifile, int ofd, FILE* ofile)
{
	this->ifd = ifd;
	this->ifile = ifile;
	this->ofd = ofd;
	this->ofile = ofile;
	this->started = 0;
}

input_obj_t::~input_obj_t()
{
	fclose(ifile);
	if (ofile != ifile) {
		fclose(ofile);
	}

	// just in case
	close(ifd);
	close(ofd);
}

template <class keytype, class valtype>
bool contains(const map<keytype, valtype>& map_obj, const keytype& key) 
{
	return map_obj.find(key) != map_obj.end();
}


void handle_dev(const int fd)
{
	g_dev_input[fd]->handle_dev(fd);
}

void handle_poll_result(const struct pollfd& pollfd)
{
	if (contains(g_accept_input, pollfd.fd)) {

		int ret = fd_accept(pollfd.fd);
		if (ret < 0) {
			LOGE("%s %d: %s", __FUNCTION__, __LINE__, strerror(errno));
			return;
		} else {
			FILE* fp = fdopen(ret, "r+");
			if (fp == NULL) {
				fatal();
			}
			setlinebuf(fp);
			g_cmd_input[ret] = new input_obj_t(ret, fp, ret, fp);
		}
	} else if (contains(g_cmd_input, pollfd.fd)) {
		handle_commands(pollfd.fd);
	} else if (contains(g_dev_input, pollfd.fd)) {
		handle_dev(pollfd.fd);
	} else {
		LOGE("%s %d: eh?", __FUNCTION__, __LINE__);
	}
}

int fill_write_fds(pollfds_t& pollfds) {
	int nfds = g_cmd_input.size();

	if (pollfds.capcity < nfds) {
		pollfds.capcity = nfds + 5;
		pollfds.pfds = (struct pollfd*) realloc(pollfds.pfds, pollfds.capcity * sizeof(struct pollfd));
		if (pollfds.pfds == NULL) {
			fatal();
		}
	}
	
	pollfds.nfds = nfds;
	int i = 0;
	for (map<int, input_obj_t*>::const_iterator it = g_cmd_input.begin();
		 it != g_cmd_input.end();
		 it ++) {
		pollfds.pfds[i].fd = it->second->ofd;
		pollfds.pfds[i].events = POLLOUT;
		pollfds.pfds[i].revents = 0;
		i++;
	}
	return 0;
}

int fill_pollfds(pollfds_t& pollfds)
{
	int nfds = g_accept_input.size() + g_cmd_input.size() + g_dev_input.size();
	if (nfds == 0) {
		fatal();
	}

	if (pollfds.capcity < nfds) {
		pollfds.capcity = nfds + 5;
		pollfds.pfds = (struct pollfd*) realloc(pollfds.pfds, pollfds.capcity * sizeof(struct pollfd));
		if (pollfds.pfds == NULL) {
			fatal();
		}
	}
	pollfds.nfds = nfds;
	int i = 0;
	for (map<int, accept_obj_t*>::const_iterator it = g_accept_input.begin(); 
		 it != g_accept_input.end();
		 it ++) {
		pollfds.pfds[i].fd = it->first;
		pollfds.pfds[i].events = POLLIN;
		pollfds.pfds[i].revents = 0;
		i++;
	}

	for (map<int, input_obj_t*>::const_iterator it = g_cmd_input.begin();
		 it != g_cmd_input.end();
		 it ++) {
		pollfds.pfds[i].fd = it->first;
		pollfds.pfds[i].events = POLLIN;
		pollfds.pfds[i].revents = 0;
		i++;
	}

	for (map<int, dev_obj_t*>::const_iterator it = g_dev_input.begin();
		 it != g_dev_input.end();
		 it ++) {
		pollfds.pfds[i].fd = it->first;
		pollfds.pfds[i].events = POLLIN;
		pollfds.pfds[i].revents = 0;
		i++;
	}
	return 0;
}

static int fprintf_if_started(FILE* fp, const char* fmt, ...)
{
	for (map<int, input_obj_t*>::const_iterator it = g_cmd_input.begin();
		 it != g_cmd_input.end();
		 it ++) {
		if (it->second->ofile == fp && !it->second->started) {
			return 0;
		}
	}

    va_list ap;

    va_start(ap, fmt);
	int ret = vfprintf(fp, fmt, ap);
    va_end(ap);

    return ret;
}
//
static int compass_data[3];
static float orietation_data[3];
static float sensor_angle[3];

static int Calculate_Sensor_Angle(void)
{
	float ipitch, iroll;
	float tmp = 0;
	float x, y, z;

	x = orietation_data[0];
	y = orietation_data[1];
	z = orietation_data[2];

	//ipitch
	if (y != 0)
		{
			tmp = z/y;
			ipitch = atan(tmp)*180/PI;
		}
    else
		ipitch = (z>0)?90:-90;
	
	if (z <= 0)
		{
			if (y >= 0)
				ipitch = 90 + ipitch;
			else
				ipitch = ipitch - 90;//180
		}
    else
		{
			if (y >= 0)
				ipitch = ipitch - 90;
			else
				ipitch = 90 + ipitch;
		}
	sensor_angle[1] = ((int)(ipitch*10 + 0.5))/10.0;

	// roll
	if (x!=0)
		{
			tmp = z/x;
			iroll = atan(tmp)*180/PI;
		}
	else
		iroll = (z>0)?90:-90;

	if (z <= 0)
		{
			if (x >= 0)			
				iroll = iroll + 90;			
			else
				iroll = iroll - 90;
		}
	else
		{
			if (x >= 0)
				iroll = iroll + 90 ;
			else
				iroll = iroll - 90 ;
		}

	sensor_angle[2] = ((int)(iroll*10 + 0.5))/10.0;

	//	DBG("cruise  Calculate_Sensor_Angle  : ******** [Y]:%f [Z]:%f ******\n", sensor_angle[1], sensor_angle[2]);
	return 0;
}

static void calculate_compass()
{
	int magX = 0; int magY = 0; int magZ = 0;
	static int count = 0;
	int iResult;
	int offsets[3] = {0};
	double dHeading = 0;
    float accForward;
    float accLeft;



	orietation_data[0] = g_accl[0];
	orietation_data[1] = g_accl[1];
	orietation_data[2] = g_accl[2];

	compass_data[0] = g_compass_data.y;
	compass_data[1] = -g_compass_data.x;
	compass_data[2] = g_compass_data.z;

	magX = compass_data[0];
	magY = compass_data[1];
	magZ = compass_data[2];

	count++;
	if (count < 120){
		CollectDataItem(magX, magY, magZ);
		//LOGE("CollectData.\n");
	}

	iResult = Calibrate(&offsets[0]);
	// Elsewhere in the application, getting magnetic data and correcting out the
	// hard iron disturbances 
	magX -= offsets[0];
	magY -= offsets[1];
	magZ -= offsets[2];
	//Magnetic field data is now ready to be used
	accForward = 0;
	accLeft = 0;	
	dHeading = Heading(-magX,magY,magZ,accForward ,accLeft);

	if (dHeading >0)
		{
			// Use the Heading Value
			g_compass_data.x = dHeading;
		}

	Calculate_Sensor_Angle();
	g_compass_data.y = sensor_angle[1];
	g_compass_data.z = sensor_angle[2];

}

static void handle_write_poll_result(const struct pollfd& pfd)
{
	for (map<int, input_obj_t*>::const_iterator it = g_cmd_input.begin();
		 it != g_cmd_input.end();
		 it ++) {
		if (it->second->ofd == pfd.fd) {
			fprintf_if_started(it->second->ofile, "acceleration:%g:%g:%g\n", 
				   GRAVITY_EARTH * g_accl[1]/1000,
				   GRAVITY_EARTH * g_accl[0]/1000,
				   GRAVITY_EARTH * g_accl[2]/1000);

			if (g_lux_updated) {
				fprintf_if_started(it->second->ofile, "light:%g\n", (float)g_lux);
			}

			if (g_compass_updated) {
				calculate_compass();
				fprintf_if_started(it->second->ofile, "magnetic:%d:%d:%d\n", g_compass_data.x, g_compass_data.y, g_compass_data.z);
			}
			fprintf_if_started(it->second->ofile, "sync:%lld\n", data__now_ms());
		}
	}
	g_lux_updated = false;
}

int main(int argc, char *argv[])
{
	LOGD("%s: %d", __FUNCTION__, __LINE__); 
	int fd = android_get_control_socket(CONTROL_SOCKET_NAME);
    if (fd < 0) {
        LOGE("couldn't get fd for control socket '%s', running as non-daemon", CONTROL_SOCKET_NAME);
		g_cmd_input[0] = new input_obj_t(0, stdin, 1, stdout);
		setlinebuf(stdout);
    } else {
		g_accept_input[fd] = new accept_obj_t(fd);
		listen(fd, 5);
	}

	scan_dir("/dev/input");

	setup_signal(); 
	init_timer(200);
	
	pollfds_t pollfds = {0, 0, NULL};

	pollfds_t write_fds = {0, 0, NULL};

	while (1) {
		fill_pollfds(pollfds);
		
		poll(pollfds.pfds, pollfds.nfds, -1);
		for (int i = 0; i < pollfds.nfds; i++) {
			if (pollfds.pfds[i].revents & POLLIN) {
				handle_poll_result(pollfds.pfds[i]);
			}
		}

		if (g_timer_triggered) {
			g_timer_triggered = 0;
			fill_write_fds(write_fds);
			if (write_fds.nfds == 0) {
				continue;
			}

			for (int i=0; i < NUM_IIO_SENSORS; i++) {
				sensors_iio_info[i].handle_sensor_event();
			}

			poll(write_fds.pfds, write_fds.nfds, 1);
			for (int i = 0; i < write_fds.nfds; i++) {
				if (write_fds.pfds[i].revents & POLLOUT) {
					handle_write_poll_result(write_fds.pfds[i]);
				}
			}
		}
	}
	return 0;
}
