#pragma once

#include "Header.h"

namespace Utils {

namespace Protocol {

#define MAX_STRING_SIZE 128

#pragma pack(1)

// Э��ͷ
typedef struct tagCmdHeader {
	BYTE cCmdBeginFlag;	// 0xBF
	WORD wCmd;			// Э���
	WORD wCRC;			// [cCmdBeginFlag | cCmdEndFlag] | [wCmd]
	BYTE cCmdEndFlag;	// 0xEF
} CmdHeader;

#pragma pack()

// Э��ͷ����
class Commond {
public:
	// ��Ȩ����
	static const WORD RequestLogin = 1001;
	// ��Ȩ������Ӧ
	static const WORD ResponseLogin = 2001;
	// ɨ����Ȩ������Ӧ
	static const WORD ResponseQCLogin = 2002;
	static const BYTE CmdBeginFlag = 0xBF;
	static const BYTE CmdEndFlag = 0xEF;
	static const size_t CmdHeaderLen = sizeof(CmdHeader);
	static WORD MakeCrc(WORD wCmd) { return (((CmdBeginFlag << 8) | CmdEndFlag) | wCmd); }
};

}

}
