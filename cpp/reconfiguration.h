#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <math.h>



int Partial_reconf(int *bitdata, int bitsize, int fddrvr, int fdctrl, unsigned long long dcplr_addr, unsigned long long ICAP_addr) {
		int answer, choice;
		
		char temp[10];
		printf("Activating (0) or de-activating (1) will affect 2 PR regions, the chosen and the next\n\r");
		printf("Do you want to activate (0) or de-activate (1) the device? or read (2) the device's status\n\r");
		
	    scanf(" %d",&answer);
		char status[4]="1";
		printf("Initialized status value %d\n\r", atoi(status));

		if (answer == 0) {
			printf("Sending decoupling command 0\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));

		}
		else if (answer == 1) {
			printf("Sending decoupling command 1\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));
		}
		else if (answer == 2) {
			printf("Reading decoupling status\n\r");
			choice=0;
			sprintf(temp, "%d, %llx", choice, dcplr_addr);
			write(fdctrl, temp, 10*sizeof(int));
			
			read(fddrvr, status, 4);
			printf("Decoupling status %d\n\r", atoi(status));
		}
		else {
			printf( "No valid preference - please re-run!!!!!!!\n");
			return 0;
		}

		printf("Do you want to activate (0) or de-activate (1) the device? or read (2) the device's status\n\r");
		scanf(" %d",&answer);

		printf("Initialized status value %d\n\r", atoi(status));

		if (answer == 0) {
			printf("Sending decoupling command 0\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));

		}
		else if (answer == 1) {
			printf("Sending decoupling command 1\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));
		}
		else if (answer == 2) {
			printf("Reading decoupling status\n\r");
			choice=0;
			sprintf(temp, "%d, %llx", choice, dcplr_addr);
			write(fdctrl, temp, 10*sizeof(int));
			
			read(fddrvr, status, 4);
			printf("Decoupling status %d\n\r", atoi(status));
		}
		else {
			printf( "No valid preference - please re-run!!!!!!!\n");
			return 0;
		}

		
		printf("opening and writing to the icap driver\n\r");
		choice =1;
		sprintf(temp, "%d, %llx", choice, ICAP_addr);
		write(fdctrl, temp, 10*sizeof(int));
		write(fddrvr, bitdata, bitsize);   	

		printf("Do you want to activate (0) or de-activate (1) the device? or read (2) the device's status\n\r");
	    	scanf(" %d",&answer);

		printf("Initialized status value %d\n\r", atoi(status));

		if (answer == 0) {
			printf("Sending decoupling command 0\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));

		}
		else if (answer == 1) {
			printf("Sending decoupling command 1\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));

		}
		else if (answer == 2) {
			printf("Reading decoupling status\n\r");
			choice=0;
			sprintf(temp, "%d, %llx", choice, dcplr_addr);
			write(fdctrl, temp, 10*sizeof(int));
			
			read(fddrvr, status, 4);
			printf("Decoupling status %d\n\r", atoi(status));
		}
		else {
			printf( "No valid preference - please re-run!!!!!!!\n");
			return 0;
		}

		printf("Do you want to activate (0) or de-activate (1) the device? or read (2) the device's status\n\r");
	    	scanf(" %d",&answer);

		printf("Initialized status value %d\n\r", atoi(status));

		if (answer == 0) {
			printf("Sending decoupling command 0\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));

		}
		else if (answer == 1) {
			printf("Sending decoupling command 1\n\r");
			sprintf(temp, "%d, %llx", answer, dcplr_addr);
			write(fddrvr, temp, 10*sizeof(int));
			sprintf(temp, "%d, %llx", answer, dcplr_addr+ 0x00010000);
			write(fddrvr, temp, 10*sizeof(int));
		}
		else if (answer == 2) {
			printf("Reading decoupling status\n\r");
			choice=0;
			sprintf(temp, "%d, %llx", choice, dcplr_addr);
			write(fdctrl, temp, 10*sizeof(int));
			
			read(fddrvr, status, 4);
			printf("Decoupling status %d\n\r", atoi(status));
		}
		else {
			printf( "No valid preference - please re-run!!!!!!!\n");
			return 0;
		}	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
