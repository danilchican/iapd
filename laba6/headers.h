#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <libudev.h>
#include <signal.h>

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#define BLOCK_SIZE 512
#define SUBSYSTEM "block"
	
struct Data 
{
	struct udev *udev;
	struct udev_enumerate *enumerate;

	struct udev_list_entry *devices;
	struct udev_list_entry *entry;

	pid_t pid;	
	key_t key;
	int semid;
	struct sembuf mybuff, mybuff1;
};

char _getch();

int has_usb(char *devpath);
void createProccessToPrintDevices(char *path, struct Data *data);
void closePrintProcess(struct Data *data);

void printDevice(struct udev_device *dev, struct Data *data);
void processDevice(struct udev_device *dev, struct Data *data);
void enumerateUSBDevices(struct Data *data);
void enumeratePhoneDevices(struct Data *data);

void monitorUSBDevices(struct Data *data);
void printUSBDevices(char *param, struct Data *data);
