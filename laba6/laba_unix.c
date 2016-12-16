#include "headers.h"

char _getch()
{
	struct termios old, new;
	char ch;
	tcgetattr(0, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &new);
	ch = getchar();

	tcsetattr(0, TCSANOW, &old);
	return ch;
}

int has_usb(char *devpath) 
{
	char *command = (char *)malloc(strlen(devpath) + 1);
	strcpy(command, devpath);

	char *pch = strtok(command, "/");
	pch = strtok(NULL, "/");

	while (pch != NULL)	{
		pch = strtok(NULL, "/");	
		
		if (pch != NULL) {
			if(!strcmp(pch, "usb1")) {	
				return 1;
			}
		}
	}
	
	return 0;
}

void printDevice(struct udev_device *dev, struct Data *data)
{
	const char *path, *tmp, *devnode, *devpath;

	unsigned long long disk_size = 0;
	unsigned short int block_size = BLOCK_SIZE;

	path = udev_list_entry_get_name(data->entry);
	dev = udev_device_new_from_syspath(data->udev, path);
	devnode = udev_device_get_devnode(dev);

	devpath = udev_device_get_devpath(dev);


	if(has_usb(devpath)) {
		tmp = udev_device_get_sysattr_value(dev, "size");

		if (tmp)
			disk_size = strtoull(tmp, NULL, 10);

		tmp = udev_device_get_sysattr_value(dev, "queue/logical_block_size");
		if (tmp)
			block_size = atoi(tmp);

		FILE *fp;
		char line[300] = {"df -H | grep "};

		strcat(line,devnode);
		fp = popen(line, "r"); 

		while (fgets( line, sizeof line, fp)) 
			printf("%s", line);
		
		printf("\n");

		pclose(fp);
	}
}

void processDevice(struct udev_device *dev, struct Data *data)
{
    if (dev) {
	if (strncmp(udev_device_get_devtype(dev), "partition", 9) == 0) {
		if (udev_device_get_devnode(dev)) {
		    printDevice(dev, data);
		}
	}

        udev_device_unref(dev);
    }
}

void enumeratePhoneDevices(struct Data *data)
{
	data->enumerate = udev_enumerate_new(data->udev);

	udev_enumerate_add_match_subsystem(data->enumerate, SUBSYSTEM);
	udev_enumerate_scan_devices(data->enumerate);

	data->devices = udev_enumerate_get_list_entry(data->enumerate);
	
	udev_list_entry_foreach(data->entry, data->devices) {
		const char* path = udev_list_entry_get_name(data->entry);
		struct udev_device* dev = udev_device_new_from_syspath(data->udev, path);
		processDevice(dev, data);
	}

	udev_enumerate_unref(data->enumerate);
}

void enumerateUSBDevices(struct Data *data)
{
	data->enumerate = udev_enumerate_new(data->udev);

	udev_enumerate_add_match_subsystem(data->enumerate, SUBSYSTEM);
	udev_enumerate_scan_devices(data->enumerate);

	data->devices = udev_enumerate_get_list_entry(data->enumerate);
	
	udev_list_entry_foreach(data->entry, data->devices) {
		const char* path = udev_list_entry_get_name(data->entry);
		struct udev_device* dev = udev_device_new_from_syspath(data->udev, path);
		processDevice(dev, data);
	}

	udev_enumerate_unref(data->enumerate);
}


void monitorUSBDevices(struct Data *data)
{
     struct udev_monitor* mon = udev_monitor_new_from_netlink(data->udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);


    while (1) {
	fd_set fds;

	struct timeval tv;

	int ret;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	ret = select(fd+1, &fds, NULL, NULL, NULL);

        if (ret > 0 && FD_ISSET(fd, &fds)) 
	{
		   	struct udev_device* dev = udev_monitor_receive_device(mon);

			const char* idVendor = udev_device_get_sysattr_value(dev, "idVendor");
			const char* idProduct = udev_device_get_sysattr_value(dev, "idProduct");
		
			if (dev) {	
				const char* product = udev_device_get_sysattr_value(dev, "iProduct");
			    	const char* action = udev_device_get_action(dev);

				    if (!action)
					action = "exists";

				if (idVendor) {
					char line[300] = {"lsusb -d "};
					FILE *fp;

					strcat(line, idVendor);
					strcat(line, ":");
					strcat(line, idProduct);
					
					strcat(line, " -v 2> /dev/null | grep -E '\\<(iProduct)' 2> /dev/null");

					printf("Action: %s", action);

					if(!strcmp(action, "add")) {
						printf("ed new device. ");
					}

					fp = popen(line, "r"); 

					while (fgets(line, sizeof line, fp)) {
						char *command = (char *)malloc(strlen(line) + 1);
						strcpy(command, line);
						
						char *pch = strtok(command, " ");
						pch = strtok(NULL, " ");
						pch = strtok(NULL, " ");
						pch = strtok(NULL, " ");

						if((pch == NULL)) {
							char line[300] = {"lsusb -d "};
							FILE *fp;

							strcat(line, idVendor);
							strcat(line, ":");
							strcat(line, idProduct);
					
							strcat(line, " -v 2> /dev/null | grep -E '\\<(idVendor)' 2> /dev/null");

							printf("Action: %s", action);

							if(!strcmp(action, "add")) {
								printf("ed new device. ");
							}
							break;
						} else {
							do {
								printf("%s ", pch);
								pch = strtok(NULL, " ");
							} while(pch != NULL);
						printf("\n");
							break;
						}
					}
		
					printf("\n");

					pclose(fp);

					
				}
				udev_device_unref(dev);
			}
		
        } 

    }
	
	udev_monitor_unref(mon);
	udev_unref(data->udev);
}

void printUSBDevices(char *param, struct Data *data)
{
	data->udev = udev_new();

	if (!(data->udev)) {
		fprintf(stderr, "udev_new() failed\n");
		exit(1);
	}

	enumerateUSBDevices(data);
	//enumeratePhoneDevices(data);
        monitorUSBDevices(data);
}



void closePrintProcess(struct Data *data)
{
	semop(data->semid, &(data->mybuff), 1);
	kill(data->pid,SIGKILL);
}

void createProccessToPrintDevices(char *path, struct Data *data)
{

	data->key = ftok(path, 0);
	data->semid = semget(data->key, 1, 0666 | IPC_CREAT);
	semctl(data->semid, 0, SETVAL, (int)0);
	data->mybuff.sem_num = 0;

	data->mybuff1.sem_num = 0;
	data->mybuff1.sem_op = 1;

	switch(data->pid = fork())
	{
		case -1:
			perror("fork error...");
			exit(1);
		case 0:	
		{
			char *cmd[3] = {path, " child", 0};
			execv(path, cmd);
		}	
		default:
			break;
	}
}
