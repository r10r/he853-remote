#include "he853.h"

HE853Controller::HE853Controller()
{
	anban_cnt = 0;
	m_initialized = false;
#if RUN_DRY == 0
	handle = hid_open(0x4d9, 0x1357, NULL);
	if (!handle) {
		printf("Unable to open device\n");
	}
	else
#endif
	{
		this->m_initialized = true;
	}
}

HE853Controller::~HE853Controller()
{
}

bool HE853Controller::sendOutputReport(uint8_t* buf)
{
	uint8_t obuf[9];
	obuf[0] = 0x00; // report id = 0, as it seems to be the only report
#if DEBUG == 1
	printf("%.2X ", obuf[0]);
#endif
	for (int i = 0; i < 8; i++) {
		obuf[i + 1] = buf[i];
#if DEBUG == 1
		printf("%.2X ", obuf[i + 1]);
#endif
	}
#if DEBUG == 1
	printf("\n");
#endif
#if RUN_DRY == 1
	return true;
#else
	return (bool) hid_write(handle, obuf, 9);
#endif
}

bool HE853Controller::readDeviceStatus()
{
	uint8_t buf[8];
	if (!m_initialized) return false;

	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x06;
	buf[1] = 0x01;
#if RUN_DRY == 0
	return sendOutputReport(buf);
#else
	return true;
#endif
}

bool HE853Controller::sendRfData_AnBan(uint16_t deviceCode, uint8_t cmd)
{
	uint8_t rfCmdBuf[32];

	unsigned int tb_fx[16] = { 0x609, 0x306, 0x803, 0xa08,
				   0x00a, 0x200, 0xc02, 0x40c,
				   0xe04, 0x70e, 0x507, 0x105,
				   0xf01, 0xb0f, 0xd0b, 0x90d };
	
	uint8_t gbuf[7];
	uint8_t kbuf[7];
	uint8_t cbuf[7];
	uint8_t idx;

	uint32_t temp;


	anban_cnt++;
	gbuf[0] = 1;
	gbuf[1] = (anban_cnt << 2) & 15;
	if (cmd > 0) { // if not off
		gbuf[1] |= 2;
	}
	gbuf[2] = deviceCode & 15;
	gbuf[3] = (deviceCode >> 4) & 15;
	gbuf[4] = (deviceCode >> 8) & 15;
	gbuf[5] = (deviceCode >> 12) & 15;
	if (cmd >= 9 || cmd == 0) {
		gbuf[6] = 0;
	} else {
		gbuf[6] = cmd - 1;
		gbuf[6] |= 8;
	}

	idx = gbuf[0];
	kbuf[0] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[1] ^ kbuf[0];
	kbuf[1] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[2] ^ kbuf[1];
	kbuf[2] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[3] ^ kbuf[2];
	kbuf[3] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[4] ^ kbuf[3];
	kbuf[4] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[5] ^ kbuf[4];
	kbuf[5] = (uint8_t) (tb_fx[idx] >> 8);
	kbuf[6] = (uint8_t) gbuf[6];
    
	idx = kbuf[0];
	cbuf[0] = (uint8_t) tb_fx[idx];
	idx = kbuf[1] ^ cbuf[0];
	cbuf[1] = (uint8_t) tb_fx[idx];
	idx = kbuf[2] ^ cbuf[1];
	cbuf[2] = (uint8_t) tb_fx[idx];
	idx = kbuf[3] ^ cbuf[2];
	cbuf[3] = (uint8_t) tb_fx[idx];
	idx = kbuf[4] ^ cbuf[3];
	cbuf[4] = (uint8_t) tb_fx[idx];
	idx = kbuf[5] ^ cbuf[4];
	cbuf[5] = (uint8_t) tb_fx[idx];
	cbuf[6] = (uint8_t) (kbuf[6] ^ 9);

	temp = (cbuf[6] << 0x18) | (cbuf[5] << 0x14) |
	       (cbuf[4] << 0x10) | (cbuf[3] << 0x0c) |
	       (cbuf[2] << 0x08) | (cbuf[1] << 0x04) | cbuf[0];
	temp = (temp >> 2) | ((temp & 3) << 0x1a);
	
	rfCmdBuf[0*8+0] = 0x01;
	// StartBit_HTime
	rfCmdBuf[0*8+1] = (uint8_t) ((320 / 10) >> 8);
	rfCmdBuf[0*8+2] = (uint8_t) (320 / 10);
	// StartBit_LTime
	rfCmdBuf[0*8+3] = (uint8_t) ((4800 / 10) >> 8);
	rfCmdBuf[0*8+4] = (uint8_t) (4800 / 10);
	// EndBit_HTime
	rfCmdBuf[0*8+5] = 0x00;
	rfCmdBuf[0*8+6] = 0x00;
	// EndBit_LTime
	rfCmdBuf[0*8+7] = 0x00;
	rfCmdBuf[1*8+0] = 0x02;
	// EndBit_LTime
	rfCmdBuf[1*8+1] = 0x00;
	// DataBit0_HTime
	rfCmdBuf[1*8+2] = (uint8_t) (320 / 10);
	// DataBit0_LTime
	rfCmdBuf[1*8+3] = (uint8_t) (960 / 10);
	// DataBit1_HTime
	rfCmdBuf[1*8+4] = (uint8_t) (960 / 10);
	// DataBit1_LTime
	rfCmdBuf[1*8+5] = (uint8_t) (320 / 10);
	// DataBit_Count
	rfCmdBuf[1*8+6] = (uint8_t) 28;
	// Frame_Count
	rfCmdBuf[1*8+7] = (uint8_t) 7;

	rfCmdBuf[2*8+0] = 0x03;
	rfCmdBuf[2*8+1] = (uint8_t) (temp >> 20);
	rfCmdBuf[2*8+2] = (uint8_t) (temp >> 12);
	rfCmdBuf[2*8+3] = (uint8_t) (temp >> 4);
	rfCmdBuf[2*8+4] = (uint8_t) (temp << 4);
	rfCmdBuf[2*8+5] = 0x00;
	rfCmdBuf[2*8+6] = 0x00;
	rfCmdBuf[2*8+7] = 0x00;

	rfCmdBuf[3*8+0] = 0x04;
	rfCmdBuf[3*8+1] = 0x00;
	rfCmdBuf[3*8+2] = 0x00;
	rfCmdBuf[3*8+3] = 0x00;
	rfCmdBuf[3*8+4] = 0x00;
	rfCmdBuf[3*8+5] = 0x00;
	rfCmdBuf[3*8+6] = 0x00;
	rfCmdBuf[3*8+7] = 0x00;

	return sendOutputReport(rfCmdBuf)
		&& sendOutputReport(rfCmdBuf+8)
		&& sendOutputReport(rfCmdBuf+16)
		&& sendOutputReport(rfCmdBuf+24);
}

bool HE853Controller::sendRfData_GER(uint16_t deviceCode, bool cmd)
{
	uint8_t rfCmdBuf[32];
	int i = 0;

	uint8_t tb_fx[16] = { 0x07, 0x0b, 0x0d, 0x0e,
    		  	      0x13, 0x15, 0x16, 0x19,
			      0x1a, 0x1c, 0x03, 0x05,
			      0x06, 0x09, 0x0a, 0x0c };

	uint8_t buf[4] = { 0x00,
			(uint8_t) ((deviceCode >> 8) & 0xff), 
			(uint8_t) (deviceCode & 0xff), 
						0x00 };

	if (cmd == true)
		buf[3] |= 0x10;
	
	uint8_t gbuf[8] = { (uint8_t) (buf[0] >> 4),
 			    (uint8_t) (buf[0] & 15),
			    (uint8_t) (buf[1] >> 4),
			    (uint8_t) (buf[1] & 15),
			    (uint8_t) (buf[2] >> 4),
			    (uint8_t) (buf[2] & 15),
			    (uint8_t) (buf[3] >> 4),
			    (uint8_t) (buf[3] & 15) };

	uint8_t kbuf[8];
	for (i = 0; i < 8; i++) {
		kbuf[i] = (uint8_t) ((tb_fx[gbuf[i]] | 0x40) & 0x7f);
	}
	kbuf[0] |= 0x80;

	uint64_t t64 = 0;
	t64 = kbuf[0];
	for (i = 1; i < 8; i++)
	{
		t64 = (t64 << 7) | kbuf[i];
	}
	t64 = t64 << 7;
	rfCmdBuf[0*8+0] = 0x01;
	// StartBit_HTime
	rfCmdBuf[0*8+1] = (uint8_t) ((260 / 10) >> 8);
	rfCmdBuf[0*8+2] = (uint8_t) (260 / 10);
	// StartBit_LTime
	rfCmdBuf[0*8+3] = (uint8_t) ((8600 / 10) >> 8);
	rfCmdBuf[0*8+4] = (uint8_t) (8600 / 10);
	// EndBit_HTime
	rfCmdBuf[0*8+5] = 0x00;
	rfCmdBuf[0*8+6] = 0x00;
	// EndBit_LTime
	rfCmdBuf[0*8+7] = 0x00;
	rfCmdBuf[1*8+0] = 0x02;
	// EndBit_LTime
	rfCmdBuf[1*8+1] = 0x00;
	// DataBit0_HTime
	rfCmdBuf[1*8+2] = (uint8_t) (260 / 10);
	// DataBit0_LTime
	rfCmdBuf[1*8+3] = (uint8_t) (260 / 10);
	// DataBit1_HTime
	rfCmdBuf[1*8+4] = (uint8_t) (260 / 10);
	// DataBit1_LTimeur	
	rfCmdBuf[1*8+5] = (uint8_t) (1300 / 10);
	// DataBit_Count
	rfCmdBuf[1*8+6] = (uint8_t) 57;
	// Frame_Count
	rfCmdBuf[1*8+7] = (uint8_t) 7;
	rfCmdBuf[2*8+0] = 0x03;
	rfCmdBuf[2*8+1] = (uint8_t) (t64 >> 56);
	rfCmdBuf[2*8+2] = (uint8_t) (t64 >> 48);
	rfCmdBuf[2*8+3] = (uint8_t) (t64 >> 40);
	rfCmdBuf[2*8+4] = (uint8_t) (t64 >> 32);
	rfCmdBuf[2*8+5] = (uint8_t) (t64 >> 24);
	rfCmdBuf[2*8+6] = (uint8_t) (t64 >> 16);
	rfCmdBuf[2*8+7] = (uint8_t) (t64 >> 8);

	rfCmdBuf[3*8+0] = 0x04;
	rfCmdBuf[3*8+1] = (uint8_t) t64;
	rfCmdBuf[3*8+2] = 0x00;
	rfCmdBuf[3*8+3] = 0x00;
	rfCmdBuf[3*8+4] = 0x00;
	rfCmdBuf[3*8+5] = 0x00;
	rfCmdBuf[3*8+6] = 0x00;
	rfCmdBuf[3*8+7] = 0x00;

	return sendOutputReport(rfCmdBuf)
		&& sendOutputReport(rfCmdBuf+8)
		&& sendOutputReport(rfCmdBuf+16)
		&& sendOutputReport(rfCmdBuf+24);
}

bool HE853Controller::sendRfData_UK(uint16_t deviceCode, bool cmd)
{
	uint8_t rfCmdBuf[32];
	int8_t i = 0;
	uint16_t buf[8];

	for (i = 0; i < 8; i++) {
		buf[i] = deviceCode % 3;
		deviceCode /= 3;

		switch (buf[i]) {
			case 0:
				buf[i] = 0;
				break;
			case 1:
				buf[i] = 3;
				break;
		case 2:
                		buf[i] = 1;
		                break;
		}
	}
    
	uint16_t addr = 0;
	for (i = 7; i >= 0; i--) {
		addr = (uint16_t) ((addr << 2) | buf[i]);
	}
    
	uint8_t sbuf = 0x14;
	if (cmd == true)
		sbuf |= 0x01;

	rfCmdBuf[0*8+0] = 0x01;
	// StartBit_HTime
	rfCmdBuf[0*8+1] = (uint8_t) ((320 / 10) >> 8);
	rfCmdBuf[0*8+2] = (uint8_t) (320 / 10);
	// StartBit_LTime
	rfCmdBuf[0*8+3] = (uint8_t) ((9700 / 10) >> 8);
	rfCmdBuf[0*8+4] = (uint8_t) (9700 / 10);
	// EndBit_HTime
	rfCmdBuf[0*8+5] = 0x00;
	rfCmdBuf[0*8+6] = 0x00;
	// EndBit_LTime
	rfCmdBuf[0*8+7] = 0x00;

	rfCmdBuf[1*8+0] = 0x02;
	// EndBit_LTime
	rfCmdBuf[1*8+1] = 0x00;
	// DataBit0_HTime
	rfCmdBuf[1*8+2] = (uint8_t) (320 / 10);
	// DataBit0_LTime
	rfCmdBuf[1*8+3] = (uint8_t) (960 / 10);
	// DataBit1_HTime
	rfCmdBuf[1*8+4] = (uint8_t) (960 / 10);
	// DataBit1_LTime
	rfCmdBuf[1*8+5] = (uint8_t) (320 / 10);
	// DataBit_Count
	rfCmdBuf[1*8+6] = (uint8_t) 24;
	// Frame_Count
	rfCmdBuf[1*8+7] = (uint8_t) 18;

	rfCmdBuf[2*8+0] = 0x03;
	rfCmdBuf[2*8+1] = (uint8_t) (addr >> 8);
	rfCmdBuf[2*8+2] = (uint8_t) addr;
	rfCmdBuf[2*8+3] = sbuf;
	rfCmdBuf[2*8+4] = 0x00;
	rfCmdBuf[2*8+5] = 0x00;
	rfCmdBuf[2*8+6] = 0x00;
	rfCmdBuf[2*8+7] = 0x00;
	
	rfCmdBuf[3*8+0] = 0x04;
	rfCmdBuf[3*8+1] = 0x00;
	rfCmdBuf[3*8+2] = 0x00;
	rfCmdBuf[3*8+3] = 0x00;
	rfCmdBuf[3*8+4] = 0x00;
	rfCmdBuf[3*8+5] = 0x00;
	rfCmdBuf[3*8+6] = 0x00;
	rfCmdBuf[3*8+7] = 0x00;

	return sendOutputReport(rfCmdBuf)
		&& sendOutputReport(rfCmdBuf+8)
		&& sendOutputReport(rfCmdBuf+16)
		&& sendOutputReport(rfCmdBuf+24);
}

bool HE853Controller::execRfCommand()
{
	// execute command
	uint8_t execCmdBuf[8];
	memset(execCmdBuf, 0x00, sizeof(execCmdBuf));
	execCmdBuf[0] = 0x05;
	// send rf command + execute command
	return sendOutputReport(execCmdBuf);
}

bool HE853Controller::getDeviceStatus(void)
{
	return readDeviceStatus();
}

bool HE853Controller::sendAnBan(uint16_t deviceId, uint8_t command)
{
	return sendRfData_AnBan(deviceId, command) &&
		execRfCommand();
}

bool HE853Controller::sendUK(uint16_t deviceId, bool command)
{
	return sendRfData_UK(deviceId, command) &&
		execRfCommand();
}

bool HE853Controller::sendEU(uint16_t deviceId, bool command)
{
	return sendRfData_GER(deviceId, command) &&
		execRfCommand();
}

bool HE853Controller::sendAll(uint16_t deviceId, uint8_t command)
{
	return sendAnBan(deviceId, command) &&
		sendEU(deviceId, (bool)command) &&
		sendUK(deviceId, (bool)command);
}

