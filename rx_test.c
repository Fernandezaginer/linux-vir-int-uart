

#include <stdio.h>
#include <uinstd.h>
#include <fcntl.h>
#include <termios.h>
#include <wiringPi.h>

#define DEVICE_UART "/dev/serial0"
#define SPEED_UART 9600


int main(void){

  if (wiringPiSetup() == -1){
	printf("Error wiringPiSetup() \n");
	return -1;
  }

  int fd;
  fd = serialOpen(DEVICE_UART, SPEED_UART);
  int i = 0;
  
  while(1){

	  if (serialDataAvail(fd)){
		if (i == 0){
		  printf("\n RX: ");
		}
		printf("%d", serialGetchar(fd));
		i++;
		if (i > 90){
		   i = 0;	
		}
	  }	    
  }
  return 0;
}