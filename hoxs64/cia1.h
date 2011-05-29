#ifndef __CIA1_H__
#define __CIA1_H__

class IAutoLoad
{
public:
	virtual void AutoLoadHandler(ICLK sysclock)=0;
};

class VIC6569;
class C64;
class Tape64;

class CIA1 : public CIA , public ErrorMsg
{
public:

	CIA1();

	HRESULT Init(CConfig *cfg, CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, VIC6569 *vic, Tape64 *tape64, CDX9 *dx, IAutoLoad *pAutoLoad);

	//IRegister
	virtual void Reset(ICLK sysclock);

	bit8 ReadPortA();
	bit8 ReadPortB();
	void WritePortA();
	void WritePortB();
	void SetSystemInterrupt();
	void ClearSystemInterrupt();

	void ExecuteDevices(ICLK sysclock);

	void LightPen();
	bit8 keyboard_matrix[8];
	bit8 keyboard_rmatrix[8];
	bit8 joyport1,joyport2;

	void ReadKeyboard();
	void ResetKeyboard();
	void SetKeyMatrixDown(bit8 row, bit8 col);

	unsigned char c64KeyMap[256];
	ICLK nextKeyboardScanClock;
	virtual void SetWakeUpClock();

	CPU6510 *cpu;
	VIC6569 *vic;
	Tape64 *tape64;
	IC64 *pIC64;

	IAutoLoad *pIAutoLoad;
protected:
	bool m_bAltLatch;
private:
	CConfig *cfg;
	CAppStatus *appStatus;
	CDX9 *dx;
};

#endif
