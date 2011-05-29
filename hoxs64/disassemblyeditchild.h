#ifndef __DISSASSEMBLYEDITCHILD_H__
#define __DISSASSEMBLYEDITCHILD_H__

class CDisassemblyEditChild : public CVirWindow
{
public:
	static const int WIDTH_LEFTBAR = 16;
	int WIDTH_LEFTBAR2;
	int LINE_HEIGHT;
	static const int PADDING_LEFT = 4;
	static const int PADDING_RIGHT = 4;
	static const int PADDING_TOP = 4;
	static const int PADDING_BOTTOM = 4;
	static const int MARGIN_TOP = 5;
	static const int TAB_ADDRESS = 0;
	static const int TAB_BYTES = 6;
	static const int TAB_MNEMONIC = 6+10;

	struct AssemblyLineBuffer
	{
		bit16 Address;
		TCHAR AddressText[Monitor::BUFSIZEADDRESSTEXT];
		TCHAR BytesText[Monitor::BUFSIZEINSTRUCTIONBYTESTEXT];
		TCHAR MnemonicText[Monitor::BUFSIZEMNEMONICTEXT];
		bool IsUnDoc;
		bool IsPC;
		int InstructionCycle;
		bool IsValid;
		AssemblyLineBuffer();
		void Clear();
		void WriteDisassemblyString(TCHAR *pszBuffer, int cchBuffer);
		bool IsEqual(AssemblyLineBuffer& other);
	};

	CDisassemblyEditChild();
	virtual ~CDisassemblyEditChild();

	static TCHAR ClassName[];

	HRESULT Init(CVirWindow *parent, IMonitorEvent *monitorEvent, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont);
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID);

	void InvalidateBuffer();
	void UpdateDisplay(bool bSeekPC);
	void UpdateBuffer(bool bEnsurePC);
	void SetTopAddress(bit16 address);
	bit16 GetNearestTopAddress(bit16 address);
	bit16 GetNthAddress(bit16 startaddress, int linenumber);
	bit16 GetTopAddress();
	bit16 GetBottomAddress();
	bit16 GetBottomAddress(int offset);
	bit16 GetNextAddress();
	bit16 GetPrevAddress();
	void GetMinWindowSize(int &w, int &h);
	static const int BUFFER_WIDTH = 50;
	static const int MAX_BUFFER_HEIGHT = 200;
private:
	CVirWindow *m_pParent;
	IMonitorCpu *m_cpu;
	IMonitorVic *vic;
	HFONT m_hFont;
	bit16 m_FirstAddress;
	int m_NumLines;
	Monitor m_mon;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;
	IMonitorEvent *m_monitorEvent;
	HBITMAP m_hBmpBreak;
	bool m_bHasLastDrawText;
	RECT m_rcLastDrawText;
	
	AssemblyLineBuffer *m_pFrontTextBuffer;
	AssemblyLineBuffer *m_pBackTextBuffer;
	HRESULT AllocTextBuffer();
	void FreeTextBuffer();
	void ClearBuffer();
	void FlipBuffer();
	void UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, int numLines, bool bEnsurePC);
	void UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, bit16 address, int startLine, int numLines, int& lineOfPC);
	void InvalidateRectChanges();
	int GetLineFromYPos(int y);

	HRESULT OnCreate(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void DrawDisplay(HWND hWnd, HDC hdc);
	void DrawDisplay2(HWND hWnd, HDC hdc);
	int GetNumberOfLines(RECT& rc, int lineHeight);
	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
