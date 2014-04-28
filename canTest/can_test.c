#include "can_test.h"

const char *can_states[CAN_STATE_MAX] = {
	"ERROR-ACTIVE",
	"ERROR-WARNING",
	"ERROR-PASSIVE",
	"BUS-OFF",
	"STOPPED",
	"SLEEPING"
};

#define NEXT_ARG() \
	do { \
		argv++; \
		if (--argc < 0) { \
			fprintf(stderr, "missing parameter for %s\n", *argv); \
			exit(EXIT_FAILURE);\
		}\
	} while(0)
	
static void do_show_bitrate(const char *name)
{
	struct can_bittiming bt;

	if (can_get_bittiming(name, &bt) < 0) {
		fprintf(stderr, "%s: failed to get bitrate\n", name);
		exit(EXIT_FAILURE);
	} else
		fprintf(stdout,
			"%s bitrate: %u, sample-point: %0.3f\n",
			name, bt.bitrate,
			(float)((float)bt.sample_point / 1000));
}

static void do_show_bittiming(const char *name)
{
	struct can_bittiming bt;

	if (can_get_bittiming(name, &bt) < 0) {
		fprintf(stderr, "%s: failed to get bittiming\n", name);
		exit(EXIT_FAILURE);
	} else
		fprintf(stdout, "%s bittiming:\n\t"
			"tq: %u, prop-seq: %u phase-seq1: %u phase-seq2: %u "
			"sjw: %u, brp: %u\n",
			name, bt.tq, bt.prop_seg, bt.phase_seg1, bt.phase_seg2,
			bt.sjw, bt.brp);
}

static void do_show_state(const char *name)
{
	int state;

	if (can_get_state(name, &state) < 0) {
		fprintf(stderr, "%s: failed to get state \n", name);
		exit(EXIT_FAILURE);
	}

	if (state >= 0 && state < CAN_STATE_MAX)
		fprintf(stdout, "%s state: %s\n", name, can_states[state]);
	else
		fprintf(stderr, "%s: unknown state\n", name);
}

static void do_show_restart_ms(const char *name)
{
	__u32 restart_ms;

	if (can_get_restart_ms(name, &restart_ms) < 0) {
		fprintf(stderr, "%s: failed to get restart_ms\n", name);
		exit(EXIT_FAILURE);
	} else
		fprintf(stdout,
			"%s restart-ms: %u\n", name, restart_ms);
}

static void do_show_ctrlmode(const char *name)
{
	struct can_ctrlmode cm;

	if (can_get_ctrlmode(name, &cm) < 0) {
		fprintf(stderr, "%s: failed to get controlmode\n", name);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "%s ctrlmode: ", name);
		//print_ctrlmode(cm.flags);
	}
}

static void do_show_clockfreq(const char *name)
{
	struct can_clock clock;

	memset(&clock, 0, sizeof(struct can_clock));
	if (can_get_clock(name, &clock) < 0) {
		fprintf(stderr, "%s: failed to get clock parameters\n",
				name);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%s clock freq: %u\n", name, clock.freq);
}

static void do_show_bittiming_const(const char *name)
{
	struct can_bittiming_const btc;

	if (can_get_bittiming_const(name, &btc) < 0) {
		fprintf(stderr, "%s: failed to get bittiming_const\n", name);
		exit(EXIT_FAILURE);
	} else
		fprintf(stdout, "%s bittiming-constants: name %s,\n\t"
			"tseg1-min: %u, tseg1-max: %u, "
			"tseg2-min: %u, tseg2-max: %u,\n\t"
			"sjw-max %u, brp-min: %u, brp-max: %u, brp-inc: %u,\n",
			name, btc.name, btc.tseg1_min, btc.tseg1_max,
			btc.tseg2_min, btc.tseg2_max, btc.sjw_max,
			btc.brp_min, btc.brp_max, btc.brp_inc);
}

void cmd_show_interface(const char *name)
{
	do_show_bitrate(name);
	do_show_bittiming(name);
	do_show_state(name);
	do_show_restart_ms(name);
	do_show_ctrlmode(name);
	do_show_clockfreq(name);
	do_show_bittiming_const(name);

	exit(EXIT_SUCCESS);
}

static void do_stop(const char *name)
{
	if (can_do_stop(name) < 0) {
		fprintf(stderr, "%s: failed to stop\n", name);
		exit(EXIT_FAILURE);
	} else {
		do_show_state(name);
	}
}


static void do_start(const char *name)
{
	if (can_do_start(name) < 0) {
		fprintf(stderr, "%s: failed to start\n", name);
		exit(EXIT_FAILURE);
	} else {
		do_show_state(name);
	}
}

static void do_set_bitrate(u32 bitrate, const char *name)
{
	u32 sample_point = 0;
	int err;

	if (sample_point)
		err = can_set_bitrate_samplepoint(name, bitrate, sample_point);
	else
		err = can_set_bitrate(name, bitrate);

	if (err < 0) {
		fprintf(stderr, "failed to set bitrate of %s to %u\n",
			name, bitrate);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){
	const char* name = "can0";
	u32 bitrate[] = {1000000, 800000, 500000, 250000, 125000, 100000, 50000, 20000};
	int sock_fd = 0;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
	int dlc = 0, extended = 0, rtr = 0;
	int ret = 0;
	int i,cnt = 0;
	u32 canidTmp = 0;
	int testno = 0;
	char s[200];

	struct can_bittiming bt;
	struct can_frame frame = {
		.can_id = 1,
	};
	struct ifreq ifr;
	struct sockaddr_can addr;
	
	if (argc < 2){
		cmd_show_interface(name);
		return 0;
	}
	
	name = argv[1];

test_start:
	do_stop(name);
	do_set_bitrate(bitrate[testno], name);
	do_show_bitrate(name);
	do_start(name);

	sleep(1);

	sock_fd= socket(family, type, proto);
	if (sock_fd < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr)) {
		perror("ioctl");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}
	frame.can_id = 0x012;
	can_get_bittiming(name, &bt);
	logPrintTime();
	snprintf(s, sizeof(s), "%s bitrate: %u, sample-point: %0.3f\n", name, bt.bitrate, (float)((float)bt.sample_point / 1000));
	logPrint(s);
	snprintf(s, sizeof(s), "press anykey to start test:bitrate[%u]\n", bt.bitrate);
	logPrint(s);
	getchar();

	logPrintTime();
	for(i = 0; i < 4; i++){
		cnt = 0;
		dlc = 0;
		switch(i){
		case 0:
			extended = 0;
			rtr = 0;
			break;
		case 1:
			extended = 1;
			rtr = 0;
			break;
		case 2:
			extended = 0;
			rtr = 1;
			break;
		case 3:
			extended = 1;
			rtr = 1;
			break;
		}

		frame.can_id++;
		
		while(1) {
			frame.data[dlc] = 0x55;
			dlc++;
			if (dlc == 8)
				break;
		}
		frame.can_dlc = dlc;
		if (extended) {
			frame.can_id &= CAN_EFF_MASK;
			frame.can_id |= CAN_EFF_FLAG;
		} else {
			frame.can_id &= CAN_SFF_MASK;
		}

		if (rtr)
			frame.can_id |= CAN_RTR_FLAG;

		while(cnt < 100000){
			ret = write(sock_fd, &frame, sizeof(frame));
			if (ret == -1) {
				//perror("write");
				continue;
			}
			cnt++; 
		}
		snprintf(s, sizeof(s), "write:frame count:%d  extendflg:%d  rtrflg:%d\n", cnt, extended, rtr);
		logPrintTime();
		logPrint(s);
	}

	sleep(1);
	frame.can_id = 0x111;
	frame.can_id &= CAN_SFF_MASK;
	ret = write(sock_fd, &frame, sizeof(frame));
	if (ret == -1) {
		perror("write");
	}

	canidTmp = 0;
	logPrintTime();
	int j;
	for(j = 0; j < 4; j++){
		cnt = 0;
		do{
			ret = read(sock_fd, &frame, sizeof(struct can_frame));
			if (ret < 0){
	 			perror("read");
				continue;
			}
/*
			else {
			char buf[BUF_SIZ];
			int n = 0;
			if (frame.can_id & CAN_EFF_FLAG)
				n = snprintf(buf, BUF_SIZ, "<0x%08x> ", frame.can_id & CAN_EFF_MASK);
			else
				n = snprintf(buf, BUF_SIZ, "<0x%03x> ", frame.can_id & CAN_SFF_MASK);

			n += snprintf(buf + n, BUF_SIZ - n, "[%d] ", frame.can_dlc);
			for (i = 0; i < frame.can_dlc; i++) {
				n += snprintf(buf + n, BUF_SIZ - n, "%02x ", frame.data[i]);
			}
			if (frame.can_id & CAN_RTR_FLAG)
				n += snprintf(buf + n, BUF_SIZ - n, "remote request");

			fprintf(stdout, "%s\n", buf);

			n = 0;
			}
*/
			cnt++;
		}while(cnt < 100000);
			int n = 0;
			if (frame.can_id & CAN_EFF_FLAG)
				n = snprintf(s, sizeof(s), "<0x%08x> ", frame.can_id & CAN_EFF_MASK);
			else
				n = snprintf(s, sizeof(s), "<0x%03x> ", frame.can_id & CAN_SFF_MASK);

			n += snprintf(s + n, sizeof(s) - n, "[%d] ", frame.can_dlc);
			for (i = 0; i < frame.can_dlc; i++) {
				n += snprintf(s + n, sizeof(s) - n, "%02x ", frame.data[i]);
			}
			if (frame.can_id & CAN_RTR_FLAG)
				n += snprintf(s + n, sizeof(s) - n, "remote request");
			snprintf(s + n, sizeof(s) - n, "  count=%d\n", cnt);
			logPrintTime();
			logPrint(s);
	}
	
	close(sock_fd);

	testno++;
	if(testno < 8)
		goto test_start;

	return 0;
}

//打印系统时间
void logPrintTime(void){
	char s[100];
	time_t sec = time(NULL);
	struct tm *tt = localtime(&sec);
	snprintf(s, sizeof(s), "%02d-%02d-%02d %02d:%02d:%02d\n",
		tt->tm_year+1900,
		tt->tm_mon+1,
		tt->tm_mday,
		tt->tm_hour,
		tt->tm_min,
		tt->tm_sec);
	logPrint(s);
}

//打印自定义信息
void logPrint(char *s){
	FILE *fp = fopen("logCan.txt", "a+");
	if(NULL == fp){
		perror("fopen");
	}
	usleep(1);
	fputs(s,fp);
	puts(s);
	fflush(fp);
	fclose(fp);
	fp = NULL;
}