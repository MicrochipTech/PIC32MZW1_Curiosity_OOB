#ifndef MCHP_PIC32MZW1_H
#define MCHP_PIC32MZW1_H

typedef struct Timer Timer;

struct Timer {
	unsigned long systick_period;
	unsigned long end_time;
};

typedef struct Network Network;

struct Network
{
	int socket;
	int hostIP;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

void TimerInit(Timer*);

int pic32mzw1_read(Network*, unsigned char*, int, int);
int pic32mzw1_write(Network*, unsigned char*, int, int);
void pic32mzw1_disconnect(Network*);
void NetworkInit(Network* n);

int ConnectNetwork(Network*, char*, int, int);

#endif //MCHP_PIC32MZW1_H