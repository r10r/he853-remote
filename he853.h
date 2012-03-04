#ifndef HE853_H__
#define HE853_H__

/* Timings

FName StartBit_HTime StartBit_LTime EndBit_HTime EndBit_LTime DataBit0_HTime DataBit0_LTime 
AnBan  320            4800           0            0            320            960           
UK     320            9700           0            0            320            960           
GER    260            8600           0            0            260            260           


FName  DataBit1_HTime DataBit1_LTime DataBit_Count Frame_Count Remark
AnBan  960            320            28            7           (建议6Frame)康泰3合1无线协议标准V1.2
UK     960            320            24            18          (建议16Frame)
GER    260            1300           57            7           (建议6Frame)

*/
#include <stdio.h>
#include <wchar.h>
#include <string.h> 
#include <stdlib.h>
#include <stdint.h>
#include "hidapi.h" // http://www.signal11.us/oss/hidapi/

#define RUN_DRY 0
#define DEBUG   0

class HE853Controller {
private:
	hid_device *handle;
	uint8_t anban_cnt;
    	bool m_initialized;

public:
	HE853Controller();
	~HE853Controller();

private:
	bool sendOutputReport(uint8_t* buf);
	bool readDeviceStatus();
	bool sendRfData_AnBan(uint16_t deviceCode, uint8_t cmd);
	bool sendRfData_GER(uint16_t deviceCode, bool cmd);
	bool sendRfData_UK(uint16_t deviceCode, bool cmd);
	bool execRfCommand();

public:
	bool getDeviceStatus(void);
	bool sendAnBan(uint16_t deviceId, uint8_t command);
	bool sendUK(uint16_t deviceId, bool command);
	bool sendEU(uint16_t deviceId, bool command);
	bool sendAll(uint16_t deviceId, uint8_t command);
};

#endif 

