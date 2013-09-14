#pragma once

class Cart;

class CartSuperGames : public CartCommon
{
public:
	CartSuperGames(CrtHeader &crtHeader, CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

protected:
	virtual void UpdateIO();
private:
};
