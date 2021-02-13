/*
 * setmac.c
 * compilation: gcc setmac.c -o setmac -s
 *
 *  Created on: Aug 24, 2017
 *      Author: sharikov
 *
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define MAC_REALMAP_OFFSET 0x11a
// ÑÐ¼ÐµÑ‰ÐµÐ½Ð¸Ðµ Ð² _ADAPTER.eeprompriv.efuse_eeprom_data[512]
// Ð² ameba ÑÑ‚Ð¾ ST_MAC. AP_MAC=ST_MAC+1
#define MAC_SIZE 6

#define MAGIC_HEADER 0x8195
uint8_t  eeprom_data[4096];
uint16_t eeprom_data_configured[4096];

/* Ñ‡Ð¸Ñ‚Ð°ÐµÑ‚ 1 Ð·Ð°Ð¿Ð¸ÑÑŒ config
 * Ð½Ð° Ð²Ñ‹Ñ…Ð¾Ð´Ðµ:
 *	-1: ÐºÐ¾Ð½ÐµÑ† Ð¿Ð¾ data_addr
 *	-2: ÐºÐ¾Ð½ÐµÑ† Ð¿Ð¾ data_len
 *	-3: ÐºÐ¾Ð½ÐµÑ† Ð¿Ð¾ data_addr + data_len
 */
int read_one_record(FILE* ifp) {
	int found=0;
	int i;
	uint16_t data_addr, data_len;

	//Ð¿Ñ€Ð¾Ñ‡Ð¸Ñ‚Ð°ÐµÐ¼ Ð·Ð°Ð³Ð¾Ð»Ð¾Ð²Ð¾Ðº
	fread((void*)&data_addr, 1, 2, ifp);
	if (data_addr == 0xffff)
		return -1;
	fread((void*)&data_len, 1, 2, ifp);
	if (data_len == 0xffff)
		return -2;
	if ( (data_addr + data_len) > 4096 )
		return -3;

	//Ð¿Ñ€Ð¾Ñ‡Ð¸Ñ‚Ð°ÐµÐ¼ Ð´Ð°Ð½Ð½Ñ‹Ðµ
	fread((void*)&eeprom_data[data_addr], data_len, 1, ifp);

	// Ð¿Ð¾Ð¼ÐµÑ‚Ð¸Ð¼ Ñ‡Ñ‚Ð¾ Ð´Ð°Ð½Ð½Ñ‹Ðµ ÐºÐ¾Ð½Ñ„Ð¸Ð³ÑƒÑ€Ð°Ñ†Ð¸Ð¸ Ð¾Ð±Ð½Ð¾Ð²Ð»ÐµÐ½Ñ‹
	for (i=0; i<data_len; i++) {
		int idx = data_addr+i;
		eeprom_data_configured[idx]++;
	}
	return found;
}
// Ñ‡Ð¸Ñ‚Ð°ÐµÐ¼ Ð²ÑÑŽ ÐºÐ¾Ð½Ñ„Ð¸Ð³ÑƒÑ€Ð°Ñ†Ð¸ÑŽ
int import_calibration(FILE* ifp) {
	int rcount=0;

	do {
		if (read_one_record(ifp) <0)
			break;
		rcount++;
	}
	while (rcount < 1024);
	return rcount;
}

int write_one_record(FILE* ofp, uint16_t data_addr, uint16_t data_len) {
	//printf("addr=0x%x:\tlen=0x%x\n", data_addr, data_len );
	fwrite((void*)&data_addr, 2, 1, ofp);
	fwrite((void*)&data_len,  2, 1, ofp);
	return fwrite((void*)&eeprom_data[data_addr],  1, data_len, ofp);
}

int export_calibration(FILE* ofp) {
	int rcnt=0;
	uint16_t data_addr, data_addr_wr;
	uint16_t data_len_wr=0;
	uint8_t wr=0;

	for (data_addr=0; data_addr<sizeof(eeprom_data); data_addr++) {
		if (wr !=0) {
			// ÑÐ¾Ñ…Ñ€Ð°Ð½ÑÐµÐ¼ ÐµÑÐ»Ð¸ Ð²ÑÑ‚Ñ€ÐµÑ‚Ð¸Ð»Ð¾ÑÑŒ Ð½Ðµ ÑÐºÐ¾Ð½Ñ„Ð¸Ð³ÑƒÑ€Ð¸Ñ€Ð¾Ð²Ð°Ð½Ð½Ð¾Ðµ Ð·Ð½Ð°Ñ‡ÐµÐ½Ð¸Ðµ Ð¸Ð»Ð¸
			// Ð¿Ñ€Ð¸ Ð¿ÐµÑ€ÐµÑ…Ð¾Ð´Ðµ Ð³Ñ€Ð°Ð½Ð¸Ñ†Ñ‹ 16 Ð±Ð°Ð¹Ñ‚
			if ((eeprom_data_configured[data_addr] == 0) ||
					((data_addr & 0xf0) != (data_addr_wr & 0xf0)) ) {
				// Ð·Ð°Ð¿Ð¸ÑÑ‹Ð²Ñ‹Ð°ÐµÐ¼ Ð½Ð° Ð´Ð¸ÑÐº
				if (write_one_record(ofp, data_addr_wr, data_len_wr) != data_len_wr)
					return -1;
				wr = 0;
			}
		}
		if ((eeprom_data_configured[data_addr] != 0)) {
			// Ð½Ð°Ñ‡Ð°Ð»Ð¾ Ð·Ð°Ð¿Ð¸ÑÐ¸ ?
			if (wr==0) {
				data_addr_wr = data_addr;   // ÑÐ¾Ñ…Ñ€Ð°Ð½Ð¸Ð¼ Ð½Ð°Ñ‡Ð°Ð»ÑŒÐ½Ñ‹Ð¹ Ð°Ð´Ñ€ÐµÑ Ð·Ð°Ð¿Ð¸ÑÐ¸
				wr =1;           // Ð½Ð°Ð´Ð¾ Ð·Ð°Ð¿Ð¸ÑÐ°Ñ‚ÑŒ
				data_len_wr=0;
			}
			data_len_wr++;
		}
	}
	// ÐµÑÐ»Ð¸ wr!=0 Ð·Ð½Ð°Ñ‡Ð¸Ñ‚ Ð¿Ð¾ÑÐ»ÐµÐ´Ð½ÑÑ Ð¸Ñ‚ÐµÑ€Ð°Ñ†Ð¸Ñ Ð½Ðµ Ð·Ð°Ð¿Ð¸ÑÐ°Ð½Ð°
	if (wr !=0) {
		if (write_one_record(ofp, data_addr_wr, data_len_wr) != data_len_wr)
			return -1;
	}
	return 0;
}

/*
 * ÑÑ‚Ð°Ð½Ð´Ð°Ñ€Ñ‚Ð½Ñ‹Ð¹ Ñ„Ð¾Ñ€Ð¼Ð°Ñ‚: "XX:XX:XX:XX:XX:XX"
 * Ñ„Ð¾Ñ€Ð¼Ð°Ñ‚ OUI+Ð½Ð¾Ð¼ÐµÑ€  : "XX:XX:XX[:XX]+nnn"
 */
int decode_macstr_from_user(char* macstr, char* newmac) {
	int cc;
	char *numptr;
	int res;
	int val[MAC_SIZE] = { -1, -1, -1, -1, -1, -1 };

	numptr = strchr(macstr, '+');
	if (numptr) {
		// OUI+Ð½Ð¾Ð¼ÐµÑ€
		res = sscanf(macstr, "%02x:%02x:%02x+%d", &val[0], &val[1], &val[2], &val[3]);
		if (res <0) {
			printf("Invalid MAC string %s\n", macstr);
			return -1;
		}
		// Ð¿Ñ€Ð¾Ð²ÐµÑ€Ð¸Ð¼ Ð½Ð¾Ð¼ÐµÑ€
		if ((val[3] <0) || (val[3] > 0xffffff)) {
			printf("MAC (OUI+num format) error: invalid number %d\n", val[3]);
			return -1;
		}
		// ÐºÐ¾Ð¿Ð¸Ñ€ÑƒÐµÐ¼ OUI
		for (cc=0; cc<3; cc++)
			newmac[cc] = val[cc];
		// Ð´Ð¾Ð±Ð°Ð²Ð»ÑÐµÐ¼ Ð½Ð¾Ð¼ÐµÑ€
		newmac[3] = (val[3] >> 16) & 255;
		newmac[4] = (val[3] >>  8) & 255;
		newmac[5] =  val[3]        & 255;
	}
	else {
		// ÑÑ‚Ð°Ð½Ð´Ð°Ñ€Ñ‚Ð½Ñ‹Ð¹ Ñ„Ð¾Ñ€Ð¼Ð°Ñ‚
		res = sscanf(macstr, MACSTR,
				&val[0], &val[1], &val[2], &val[3], &val[4], &val[5]);
		if (res != MAC_SIZE) {
			printf("Invalid MAC (standart format)\n");
			return -1;
		}

		// Ñ„Ð¾Ñ€Ð¼Ð¸Ñ€ÑƒÐµÐ¼ MAC
		for (cc=0; cc < MAC_SIZE; cc++)
			newmac[cc] = val[cc];
	}
	// Ð² Ameba Ð¼Ð»Ð°Ð´ÑˆÐ¸Ð¹ Ð±Ð°Ð¹Ñ‚ MAC Ð½Ðµ Ð¼Ð¾Ð¶ÐµÑ‚ Ð±Ñ‹Ñ‚ÑŒ Ñ€Ð°Ð²ÐµÐ½ ff
	// Ð¿Ñ€Ð¾Ð¸Ð·Ð¾Ð¹Ð´ÐµÑ‚ Ð¿ÐµÑ€ÐµÐ¿Ð¾Ð»Ð½ÐµÐ½Ð¸Ðµ AP_MAC
	// ((char)AP_MAC[5]=(char)ST_MAC[5]+1 --> 0xff+1 = 0)
	if (newmac[5] == (char)255) {
		printf("Bad MAC! MAC[5] = 0xff\n");
		return -1;
	}

	// Ð¿Ñ€Ð¾Ð²ÐµÑ€Ð¸Ð¼ ÑÑ„Ð¾Ñ€Ð¼Ð¸Ñ€Ð¾Ð²Ð°Ð½Ð½Ñ‹Ð¹ MAC Ð½Ð° 0 Ð¸ ff
	res=0;
	for (cc=0; cc < MAC_SIZE; cc++) {
		res |= newmac[cc];
	}
	if (res==0) {
		printf ("Error! MAC = 00:00:00:00:00:00 \n");
		return -1;
	}
	res=0xff;
	for (cc=0; cc < MAC_SIZE; cc++) {
		res &= newmac[cc];
	}
	if (res==0xff) {
		printf ("Error! MAC = FF:FF:FF:FF:FF:FF \n");
		return -1;
	}

	return 0;
}

void print_usage() {
	printf("Usage: setmac -i oldcalibration [-o updatedcalibration] [-v] [-n new_mac]\n");
	printf(" oldcalibration - current calibration data (binary)\n");
	printf(" updatedcalibration - updated calibration data (binary)\n");
	printf(" new_mac - new MAC\n");
	printf(" supported MAC format:\n");
	printf("  standart  : XX:XX:XX:XX:XX:XX\n");
	printf("  OUI+serial: XX:XX:XX[:XX:XX:XX]+nnn\n");
}

int main(int argc, char *argv[])
{
	char* ifn = NULL;
	char* ofn=NULL;
	char* macstr=NULL;
	char  newmac[6];
	FILE *ifp = NULL;
	FILE *ofp = NULL;
	int  retcode = EXIT_SUCCESS;
	uint16_t val16;
	int rcount=0;
	int option = 0;

	while ((option = getopt(argc, argv,"i:o:m:")) != -1) {
		switch (option) {
		case 'i' :
			ifn = optarg;
			break;
		case 'o' :
			ofn = optarg;
			break;
		case 'm' :
			macstr=optarg;
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}
	if (ifn==NULL) {
		print_usage();
		exit(EXIT_FAILURE);
	}
	if (ofn && macstr==NULL) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (macstr) {
		if (decode_macstr_from_user(macstr, newmac) <0)
			exit(EXIT_FAILURE);

	}

	memset(eeprom_data_configured, 0, sizeof(eeprom_data_configured));
	memset(eeprom_data, 0xff, sizeof(eeprom_data));


	ifp = fopen(ifn, "rb+");
	if (!ifp) {
		retcode = 2;
		goto exit_lbl;
	}
	if (ofn) {
		ofp = fopen(ofn, "wb+");
		if (!ofp) {
			retcode = 2;
			goto exit_lbl;
		}
	}

	// Ð¿Ñ€Ð¾Ð²ÐµÑ€Ð¸Ð¼ Ð·Ð°Ð³Ð¾Ð»Ð¾Ð²Ð¾Ðº Ñ„Ð°Ð¹Ð»Ð°
	fread((void*)&val16, 1, 2, ifp);
	if (val16 != MAGIC_HEADER) {
		printf("Magic Header mismatch! (0x%x)\n", val16);
		retcode = -1;
		goto exit_lbl;
	}

	rcount = import_calibration(ifp);

	if (rcount ==0) {
		retcode = EXIT_FAILURE;
		printf("No calibration data\n");
		goto exit_lbl;
	}

	else {
		if (	(eeprom_data_configured[MAC_REALMAP_OFFSET+0] !=0) ||
				(eeprom_data_configured[MAC_REALMAP_OFFSET+1] !=0) ||
				(eeprom_data_configured[MAC_REALMAP_OFFSET+2] !=0) ||
				(eeprom_data_configured[MAC_REALMAP_OFFSET+3] !=0) ||
				(eeprom_data_configured[MAC_REALMAP_OFFSET+4] !=0) ||
				(eeprom_data_configured[MAC_REALMAP_OFFSET+5] !=0) ) {
			printf("oldMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
					MAC2STR(&eeprom_data[MAC_REALMAP_OFFSET]));
		}
		else {
			printf("MAC not found\n");
			retcode = EXIT_FAILURE;
			goto exit_lbl;
		}
	}

	if (ofp) {
		//  Ð²ÑÑ‚Ð°Ð²Ð¸Ð¼ Ð½Ð¾Ð²Ñ‹Ð¹ MAC
		for (val16=0; val16 < MAC_SIZE; val16++) {
			eeprom_data[MAC_REALMAP_OFFSET + val16]=newmac[val16];
			eeprom_data_configured[MAC_REALMAP_OFFSET+val16]++;
		}
		printf("newMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
				MAC2STR(&eeprom_data[MAC_REALMAP_OFFSET]));
		// ÑÐ¾Ð·Ð´Ð°Ð´Ð¸Ð¼ Ð½Ð¾Ð²Ñ‹Ð¹ ÐºÐ°Ð»Ð¸Ð±Ñ€Ð¾Ð²Ð¾Ñ‡Ð½Ñ‹Ð¹ Ñ„Ð°Ð¹Ð»
		val16 = MAGIC_HEADER;
		if (fwrite((void*)&val16, 2, 1, ofp)!=1) {
			retcode = 2;
			goto exit_lbl;
		}

		if (export_calibration(ofp) !=0) {
			printf("Write error!\n");
			retcode = 2;
		}
	}


	exit_lbl:
	if (ifp) {
		fclose(ifp);
		ifp=NULL;
	}
	if (ofp) {
		fclose(ofp);
		ofp=NULL;
	}
	return retcode;
}

