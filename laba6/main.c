#include "headers.h"

int main(int argc, char *argv[])
{
	struct Data data;
	FILE *fp;
	char command[10] = { "eject " };

	if (argc == 1)
	{
		createProccessToPrintDevices(argv[0], &data);

		while (1)
		{
			switch (_getch())
			{
			case 'e': 
				{
					char sda[20], temp[50] = { '\0' }; 
				
					printf("\nEnter /dev/sdX: ");
					fflush(stdin);
					gets(sda);
				
					strcat(temp, command);
					strcat(temp, sda);
					fp = popen(temp, "r"); 
					printf("\n");

					pclose(fp);
				}
				
				break;

			case 'u':
				data.udev = udev_new();

				if (!(data.udev)) {
					fprintf(stderr, "udev_new() failed\n");
					exit(1);
				}
				enumerateUSBDevices(&data);
				break;
			case 'q':
				closePrintProcess(&data);
				return 0;
				break;
			}
		}
	}
	else
		printUSBDevices(argv[1], &data);

	return 0;
}
