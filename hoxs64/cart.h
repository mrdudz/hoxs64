#ifndef __CART_H__
#define __CART_H__

#define APPWARN_UNKNOWNCARTTYPE MAKE_HRESULT(0, 0xa00, 2)

class ICartInterface : public IRegister
{
public:

	//virtual void Reset(ICLK sysclock)=0;
	//virtual void ExecuteCycle(ICLK sysclock)=0;
	//virtual bit8 ReadRegister(bit16 address, ICLK sysclock)=0;
	//virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data)=0;
	//virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock)=0;

	virtual bit8 Get_GAME()=0;
	virtual bit8 Get_EXROM()=0;

	virtual bit8 ReadROML(bit16 address)=0;
	virtual bit8 ReadROMH(bit16 address)=0;
	virtual bit8 ReadUltimaxROML(bit16 address)=0;
	virtual bit8 ReadUltimaxROMH(bit16 address)=0;

	virtual void WriteROML(bit16 address, bit8 data)=0;
	virtual void WriteROMH(bit16 address, bit8 data)=0;
	virtual void WriteUltimaxROML(bit16 address, bit8 data)=0;
	virtual void WriteUltimaxROMH(bit16 address, bit8 data)=0;

	virtual bit8 MonReadROML(bit16 address)=0;
	virtual bit8 MonReadROMH(bit16 address)=0;
	virtual bit8 MonReadUltimaxROML(bit16 address)=0;
	virtual bit8 MonReadUltimaxROMH(bit16 address)=0;

	virtual void MonWriteROML(bit16 address, bit8 data)=0;
	virtual void MonWriteROMH(bit16 address, bit8 data)=0;
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data)=0;
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data)=0;

	virtual bool IsUltimax()=0;
	virtual bool IsCartIOActive()=0;
	virtual bool IsCartAttached()=0;

	virtual void CartFreeze()=0;
	virtual void CartReset()=0;
	virtual void CheckForCartFreeze()=0;

	virtual void AttachCart()=0;
	virtual void DetachCart()=0;
	virtual void ConfigureMemoryMap()=0;
};

class Cart;

# pragma pack (1)

struct CrtHeader
{
	bit8 Signature[16];
	bit32 FileHeaderLength;
	bit16 Version;
	bit16 HardwareType;
	bit8 EXROM;
	bit8 GAME;
	bit8 Reserved[6];
	bit8 CartridgeName[32];
};

struct CrtChip
{
	bit8 Signature[4];
	bit32 TotalPacketLength;
	bit16 ChipType;
	bit16 BankLocation;
	bit16 LoadAddressRange;
	bit16 ROMImageSize;
};

# pragma pack ()

struct CrtChipAndData
{
	CrtChipAndData();
	CrtChip chip;
	bit8 *pData;
	bit16 allocatedSize;
	bit16 romOffset;
	__int64 iFileIndex;
};

struct CrtBank
{
	CrtBank();
	virtual ~CrtBank();
	bit16 bank;
	CrtChipAndData chipAndDataLow;
	CrtChipAndData chipAndDataHigh;
};

typedef shared_ptr<CrtBank> Sp_CrtBank;

//typedef shared_ptr<CrtChipAndData> Sp_CrtChipAndData;
//struct LessChipAndDataBank
//{
//	bool operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const;
//};

typedef std::vector<Sp_CrtBank> CrtBankList;
typedef std::vector<Sp_CrtBank>::iterator CrtBankListIter;
typedef std::vector<Sp_CrtBank>::const_iterator CrtBankListConstIter;

class CartCommon : public ICartInterface
{
public:
	CartCommon(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory);
	virtual ~CartCommon();

	void InitReset(ICLK sysclock);

	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	virtual bit8 Get_GAME();
	virtual bit8 Get_EXROM();

	virtual bit8 ReadROML(bit16 address);
	virtual bit8 ReadROMH(bit16 address);
	virtual bit8 ReadUltimaxROML(bit16 address);
	virtual bit8 ReadUltimaxROMH(bit16 address);

	virtual void WriteROML(bit16 address, bit8 data);
	virtual void WriteROMH(bit16 address, bit8 data);
	virtual void WriteUltimaxROML(bit16 address, bit8 data);
	virtual void WriteUltimaxROMH(bit16 address, bit8 data);

	virtual bit8 MonReadROML(bit16 address);
	virtual bit8 MonReadROMH(bit16 address);
	virtual bit8 MonReadUltimaxROML(bit16 address);
	virtual bit8 MonReadUltimaxROMH(bit16 address);

	virtual void MonWriteROML(bit16 address, bit8 data);
	virtual void MonWriteROMH(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data);

	virtual bool IsUltimax();
	virtual bool IsCartIOActive();
	virtual bool IsCartAttached();

	virtual void CartFreeze();
	virtual void CartReset();
	virtual void CheckForCartFreeze();

	virtual void AttachCart();
	virtual void DetachCart();
	virtual void ConfigureMemoryMap();

protected:
	virtual void UpdateIO()=0;
	void BankRom();

	Cart *m_pCart;
	CrtHeader& m_crtHeader;
	CrtBankList& m_lstBank;
	bit8 *m_pCartData;
	bit8 *m_pZeroBankData;
	bit8 *m_ipROML_8000;
	bit8 *m_ipROMH_A000;
	bit8 *m_ipROMH_E000;

	bit8 reg1;
	bit8 reg2;

	bit8 GAME;
	bit8 EXROM;
	bool m_bIsCartAttached;
	bool m_bIsCartIOActive;
	bool m_bIsCartRegActive;
	bit8 m_iSelectedBank;

	bool m_bEnableRAM;
	bool m_bAllowBank;
	bool m_bREUcompatible;
	bit8 m_bFreezePending;
	bit8 m_bFreezeDone;
	bit16 m_iRamBankOffsetIO;
	bit16 m_iRamBankOffsetRomL;
	
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;
	bool m_bEffects;
};

class Cart : public ICartInterface, public ErrorMsg
{
public:
	class CartType
	{
	public:
		enum ECartType
		{
			Normal_Cartridge = 0,
			Action_Replay = 1,//AR5 + AR6 + AR4.x
			Final_Cartridge_III = 3,
			Simons_Basic = 4,
			Ocean_1 = 5,
			Fun_Play = 7,
			Super_Games = 8,
			System_3 = 15,
			Dinamic = 17,
			Zaxxon = 18,
			Magic_Desk = 19,
			Action_Replay_4 = 30,
			EasyFlash = 32,
			Action_Replay_3 = 35,	
			Retro_Replay = 36,
			Action_Replay_2 = 50,
		};
	};
	class ChipType
	{
	public:
		enum EChipType
		{
			ROM=0,
			RAM=1,
			EPROM=2,
		};
	};
	Cart();
	~Cart();
	void Init(IC6510 *pCpu, bit8 *pC64RamMemory);
	HRESULT LoadCrtFile(LPCTSTR filename);
	shared_ptr<ICartInterface> CreateCartInterface(IC6510 *pCpu, bit8 *pC64RamMemory);
	int GetTotalCartMemoryRequirement();
	static bool IsSupported(CartType::ECartType hardwareType);
	bool IsSupported();

	void Reset(ICLK sysclock);
	void ExecuteCycle(ICLK sysclock);
	bit8 ReadRegister(bit16 address, ICLK sysclock);
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	bit8 Get_GAME();
	bit8 Get_EXROM();

	bit8 ReadROML(bit16 address);
	bit8 ReadROMH(bit16 address);
	bit8 ReadUltimaxROML(bit16 address);
	bit8 ReadUltimaxROMH(bit16 address);

	void WriteROML(bit16 address, bit8 data);
	void WriteROMH(bit16 address, bit8 data);
	void WriteUltimaxROML(bit16 address, bit8 data);
	void WriteUltimaxROMH(bit16 address, bit8 data);

	bit8 MonReadROML(bit16 address);
	bit8 MonReadROMH(bit16 address);
	bit8 MonReadUltimaxROML(bit16 address);
	bit8 MonReadUltimaxROMH(bit16 address);

	void MonWriteROML(bit16 address, bit8 data);
	void MonWriteROMH(bit16 address, bit8 data);
	void MonWriteUltimaxROML(bit16 address, bit8 data);
	void MonWriteUltimaxROMH(bit16 address, bit8 data);

	bool IsUltimax();
	bool IsCartIOActive();
	bool IsCartAttached();
	
	void CartFreeze();
	void CartReset();
	void CheckForCartFreeze();

	void AttachCart();
	void DetachCart();
	void ConfigureMemoryMap();


	CrtHeader m_crtHeader;//Cart head information
	CrtBankList m_lstBank;//A list of cart ROM/EPROM banks that index the cart memory data.
	bit8 *m_pCartData; //Pointer to the cart memory data. This a block of RAM followed by 1 or more ROM/EPROM banks.
	bit8 *m_pZeroBankData; //An offset into the cart memory data that points past the initial block of RAM to the first ROM/EPROM bank.

	bool m_bIsCartDataLoaded;

	bit8 *m_ipROML_8000;
	bit8 *m_ipROMH_A000;
	bit8 *m_ipROMH_E000;
protected:
	
private:
	static const int RAMRESERVEDSIZE;
	static const int ZEROBANKOFFSET;
	void CleanUp();
	int GetTotalCartMemoryRequirement(CrtBankList lstBank);
	void BankRom();
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;

	shared_ptr<ICartInterface> m_spCurrentCart;
	friend CartCommon;
};

#include "cart_normal_cartridge.h"
#include "cart_easyflash.h"
#include "cart_retro_replay.h"
#include "cart_action_replay.h"
#include "cart_action_replay_mk4.h"
#include "cart_action_replay_mk3.h"
#include "cart_action_replay_mk2.h"
#include "cart_final_cartridge_iii.h"
#include "cart_simons_basic.h"
#include "cart_ocean1.h"
#include "cart_fun_play.h"
#include "cart_super_games.h"
#include "cart_system_3.h"
#include "cart_dinamic.h"
#include "cart_zaxxon.h"
#include "cart_magic_desk.h"
#endif