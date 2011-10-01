#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>

#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "errormsg.h"

#include "assert.h"
#include "hexconv.h"
#include "cevent.h"
//include "monitor.h"
#include "edln.h"
#include "resource.h"


const TCHAR EdLn::m_szMeasureAddress[] = TEXT("0000");
const TCHAR EdLn::m_szMeasureByte[] = TEXT("00");
const TCHAR EdLn::m_szMeasureCpuFlags[] = TEXT("NV-BDIZC");
const TCHAR EdLn::m_szMeasureDigit[] = TEXT("0");

EdLn::EdLn()
{
	m_iValue = 0;
	m_iTextBufferLength = 0;
	m_TextBuffer = NULL;
	m_pTextExtents = NULL;
	m_iTextExtents = 0;
	m_iNumChars = 0;
	m_pszCaption = 0;
	IsFocused = false;
	m_hRegionHitAll = NULL;
	m_hRegionHitEdit = NULL;
}

EdLn::~EdLn()
{
	Cleanup();
}

void EdLn::Cleanup()
{
	FreeTextBuffer();
	FreeCaption();

	if (m_hRegionHitAll)
	{
		DeleteObject(m_hRegionHitAll);
		m_hRegionHitAll = NULL;
	}
	if (m_hRegionHitEdit)
	{
		DeleteObject(m_hRegionHitEdit);
		m_hRegionHitEdit = NULL;
	}
}

HRESULT EdLn::Init(HFONT hFont, LPCTSTR pszCaption, EditStyle style, bool isEditable, int numChars)
{
HRESULT hr = E_FAIL;
	Cleanup();
	this->m_style = style;
	this->m_bIsEditable = isEditable;
	this->m_iNumChars = numChars;
	this->m_hFont = hFont;
	this->m_MinSizeDone = false;
	IsFocused = false;

	int m = 1;
	switch (m_style)
	{
	case HexAddress:
		m = 4;
		break;
	case HexByte:
		m = 2;
		break;
	case CpuFlags:
		m = 8;
		break;
	case Hex:
	case Dec:
		m = numChars;
		break;
	default:
		m = numChars;
		break;
	}

	if (m==0)
		return E_FAIL;

	hr = AllocTextBuffer(m);
	if (FAILED(hr))
		return hr;
	hr = SetCaption(pszCaption);
	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT EdLn::AllocTextBuffer(int i)
{
HRESULT hr = E_FAIL;
	FreeTextBuffer();
	int n = i + 1;
	do
	{

		m_TextBuffer = (TCHAR *)malloc(n * sizeof(TCHAR));
		if (m_TextBuffer==NULL)
		{
			hr = E_OUTOFMEMORY;
			break;
		}
		m_pTextExtents = (int *)malloc(n * sizeof(TCHAR));
		if (m_pTextExtents==NULL)
		{
			hr = E_OUTOFMEMORY;
			break;
		}
		hr = S_OK;
	} while (false);
	if (FAILED(hr))
	{
		FreeTextBuffer();
		return hr;
	}
	ZeroMemory(m_pTextExtents, n);
	m_iTextBufferLength = n;
	m_iNumChars = i;
	m_iTextExtents = n;
	
	return S_OK;
}

void EdLn::FreeTextBuffer()
{
	if (m_TextBuffer != NULL)
	{
		free(m_TextBuffer);
		m_TextBuffer = NULL;
	}
	if (m_pTextExtents != NULL)
	{
		free(m_pTextExtents);
		m_pTextExtents = NULL;
	}
	m_iTextBufferLength = 0;
	m_iNumChars = 0;
	m_iTextExtents = 0;
}

HRESULT EdLn::SetCaption(LPCTSTR pszCaption)
{
	if (pszCaption)
	{
		int i = lstrlen(pszCaption);
		void *p = malloc((i + 1) * sizeof(TCHAR));
		if (p == NULL)
			return E_OUTOFMEMORY;
		FreeCaption();
		m_pszCaption = (TCHAR *)p;
		lstrcpy(m_pszCaption, pszCaption);
	}
	else
	{
		FreeCaption();
	}
	return S_OK;
}

void EdLn::FreeCaption()
{
	if (m_pszCaption != NULL)
	{
		free(m_pszCaption);
		m_pszCaption = NULL;
	}
}

HRESULT EdLn::CreateDefaultHitRegion(HDC hdc)
{
//int w,h;
HRESULT hr;

	RECT rcAll;
	RECT rcCaption;
	RECT rcEdit;
	hr = GetRects(hdc, &rcCaption, &rcEdit, &rcAll);
	if (FAILED(hr))
		return hr;
	if (m_hRegionHitAll==NULL)
	{
		m_hRegionHitAll = CreateRectRgnIndirect(&rcAll);
		if (m_hRegionHitAll == NULL)
			return E_FAIL;
	}
	else
	{
		SetRectRgn(m_hRegionHitAll, rcAll.left, rcAll.top, rcAll.left, rcAll.bottom);
	}
	if (m_hRegionHitEdit==NULL)
	{
		m_hRegionHitEdit = CreateRectRgnIndirect(&rcEdit);
		if (m_hRegionHitEdit == NULL)
			return E_FAIL;
	}
	else
	{
		SetRectRgn(m_hRegionHitEdit, rcEdit.left, rcEdit.top, rcEdit.left, rcEdit.bottom);
	}
	return S_OK;
}

bool EdLn::IsHitAll(int x, int y)
{
	if (m_hRegionHitAll==NULL)
		return false;
	return 0 != ::PtInRegion(m_hRegionHitAll, x, y);
}

bool EdLn::IsHitEdit(int x, int y)
{
	if (m_hRegionHitEdit==NULL)
		return false;
	return 0 != ::PtInRegion(m_hRegionHitEdit, x, y);
}

int EdLn::GetCharIndex(HDC hdc, int x, int y, POINT *pPos)
{
RECT rcEdit;
HRESULT hr;
	
	if (m_pTextExtents == NULL || m_iNumChars <=0)
		return -1;

	if (!IsHitEdit(x, y))
		return -1;

	hr = GetRects(hdc, NULL, &rcEdit, NULL);
	if (FAILED(hr))
		return -1;

	if (pPos)
	{
		pPos->x = m_posX;
		pPos->y = m_posY;
	}

	if (x <= m_posX)
		return 0;

	BOOL br;
	int k = lstrlen(m_TextBuffer);
	if (k > m_iTextExtents)
		k = m_iTextExtents;

	INT numFit = 0;
	SIZE sizeText;
	br = GetTextExtentExPoint(hdc, m_TextBuffer, k, 0, &numFit, m_pTextExtents, &sizeText);
	if (!br)
		return -1;

	if (numFit <= 1)
		return 0;

	int w = abs(rcEdit.right - rcEdit.left);
	for (int i =numFit - 1; i >=0 ;i--)
	{
		if (x >= m_posX + m_pTextExtents[i])
		{
			if (pPos)
			{
				pPos->x = m_posX + m_pTextExtents[i];
				pPos->y = m_posY;
			}
			return i;
		}
	}
	return 0;
}

//HRESULT EdLn::GetMinWindowSize(HDC hdc, int &w, int &h)
HRESULT EdLn::GetRects(HDC hdc, RECT *prcCaption, RECT *prcEdit, RECT *prcAll)
{
BOOL br;
HRESULT hr = E_FAIL;
	//w = PADDING_LEFT + PADDING_RIGHT;
	//h = MARGIN_TOP + PADDING_TOP + PADDING_BOTTOM;
	RECT rcCaption;
	RECT rcEdit;
	RECT rcAll;
	SetRectEmpty(&rcCaption);
	SetRectEmpty(&rcEdit);
	SetRectEmpty(&rcAll);
	if (hdc != NULL && m_hFont != NULL)
	{
		int prevMapMode = SetMapMode(hdc, MM_TEXT);
		if (prevMapMode)
		{
			HFONT hFontPrev = (HFONT)SelectObject(hdc, m_hFont);
			if (hFontPrev)
			{
				TEXTMETRIC tm;
				br = GetTextMetrics(hdc, &tm);
				if (br)
				{
					const TCHAR *psData;
					int m = 1;
					switch (m_style)
					{
					case HexAddress:
						psData = m_szMeasureAddress;
						m = 4;
						break;
					case HexByte:
						psData = m_szMeasureByte;
						m = 2;
						break;
					case CpuFlags:
						psData = m_szMeasureCpuFlags;
						m = 8;
						break;
					case Hex:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					case Dec:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					default:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					}

					SIZE sizeAll;
					sizeAll.cx = sizeAll.cy = 0;
					SIZE sizeCaption = sizeAll;
					SIZE sizeEdit = sizeAll;
					int tx1;
					int tx2;
					int slen1;
					int slen2;
					if (m_pszCaption != NULL)
						slen1 = lstrlen(m_pszCaption);
					else
						slen1 = 0;
					slen2 = lstrlen(psData);
					tx1 = slen1 * tm.tmAveCharWidth;
					tx2 = m * tm.tmAveCharWidth;
					BOOL brCaption = FALSE;
					BOOL brEdit = FALSE;
					if (slen1 > 0)
					{
						brCaption = GetTextExtentExPoint(hdc, m_pszCaption, slen1, 0, NULL, NULL, &sizeCaption);
						if (brCaption)
						{
							SetRect(&rcCaption, 0, 0, sizeCaption.cx, sizeCaption.cy);
							tx1 = (sizeCaption.cx);
						}
					}
					if (m > 0)
					{
						brEdit = GetTextExtentExPoint(hdc, psData, slen2, 0, NULL, NULL, &sizeEdit);
						if (brEdit)
						{
							SetRect(&rcEdit, 0, 0, sizeEdit.cx, sizeEdit.cy);
							if (tx2 > sizeEdit.cx)
								sizeEdit.cx = tx2;
							if (brCaption)
							{
								OffsetRect(&rcEdit, 0, sizeCaption.cy);
							}
							tx2 = (sizeEdit.cx);
						}
					}
					OffsetRect(&rcCaption, m_posX, m_posY);
					OffsetRect(&rcEdit, m_posX, m_posY);

					UnionRect(&rcAll, &rcCaption, &rcEdit);

					if (prcCaption)
						CopyRect(prcCaption, &rcCaption);
					if (prcEdit)
						CopyRect(prcEdit, &rcEdit);
					if (prcAll)
						CopyRect(prcAll, &rcAll);

					//if (tx2 > tx1)
					//	w += tx2;
					//else
					//	w += tx1;

					hr = S_OK;

					//if (m_pszCaption!=NULL)
					//	h += tm.tmHeight * 2;
					//else
					//	h += tm.tmHeight * 1;
					//m_MinSizeW = w;
					//m_MinSizeH = h;
					//m_MinSizeDone = true;
				}

				SelectObject(hdc, hFontPrev);
			}
			SetMapMode(hdc, prevMapMode);
		}
	}
	return hr;
}

HRESULT EdLn::SetPos(int x, int y)
{
	m_posX = x;
	m_posY = y;
	return S_OK;
}

int EdLn::GetXPos()
{
	return m_posX;
}

int EdLn::GetYPos()
{
	return m_posY;
}

void EdLn::Draw(HDC hdc)
{
	if (m_TextBuffer==NULL)
		return;
	int k = (int)GetString(NULL, 0);
	if (k > m_iNumChars)
		k = m_iNumChars;
	int iHeight = 10;
	TEXTMETRIC tm;
	BOOL br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		iHeight = tm.tmHeight;
	}
	//SIZE sizeText;
	int x = m_posX;
	int y = m_posY;
	if (this->m_pszCaption != NULL)
	{
		int lenCaption = lstrlen(m_pszCaption);
		G::DrawDefText(hdc, x, y, m_pszCaption, lenCaption, NULL, NULL);
		y += iHeight;
	}

	//BOOL brTextExtent = GetTextExtentExPoint(hdc, m_TextBuffer, k, 0, NULL, m_pTextExtents, &sizeText);
	ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, m_TextBuffer, k, NULL);
}


HRESULT EdLn::GetValue(int& v)
{
HRESULT hr=E_FAIL;
	if (m_TextBuffer == NULL)
		return E_FAIL;

	switch (m_style)
	{
	case HexAddress:
		hr = GetHex(v);
		break;
	case HexByte:
		hr = GetHex(v);
		break;
	case CpuFlags:
		hr = GetFlags(v);
		break;
	case Hex:
		hr = GetHex(v);
		break;
	case Dec:
		hr = GetDec(v);
		break;
	}
	return hr;
}


void EdLn::SetValue(int v)
{
	switch (m_style)
	{
	case HexAddress:
		SetHexAddress(v);
		break;
	case HexByte:
		SetHexByte(v);
		break;
	case CpuFlags:
		SetFlags(v);
		break;
	case Hex:
		SetHex(v);
		break;
	case Dec:
		SetDec(v);
		break;
	}
}

HRESULT EdLn::GetFlags(int& v)
{
const int NUMDIGITS = 8;
	if (m_TextBuffer == NULL)
		return E_FAIL;
	int i;
	int k;
	v = 0;
	for (i=0, k=0x80 ; i < m_iNumChars ; i++, k>>=1)
	{
		TCHAR c = m_TextBuffer[i];
		if (c == 0)
			break;
		if (c != ' ' && c != '0')
		{
			v = v | k;
		}
	}
	return S_OK;
}

void EdLn::SetFlags(int v)
{
TCHAR szBitByte[9];
const int NUMDIGITS = 8;

	if (m_TextBuffer == NULL || m_iNumChars < NUMDIGITS)
		return ;
	int i = 0;
	m_TextBuffer[0] = 0;
	ZeroMemory(szBitByte, _countof(szBitByte) * sizeof(TCHAR));
	for (i=0; i < _countof(szBitByte) && i < NUMDIGITS; i++)
	{
		int k = (((unsigned int)v & (1UL << (7-i))) != 0);
		szBitByte[i] = TEXT('0') + k;
	}
	_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, szBitByte, _TRUNCATE);
}

HRESULT EdLn::GetHex(int& v)
{
HRESULT hr = E_FAIL;

	if (m_TextBuffer == NULL || m_iNumChars <= 0)
		return E_FAIL;

	int r = _stscanf_s(m_TextBuffer, TEXT(" %x"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}
	return S_OK;
}

void EdLn::SetHex(int v)
{
TCHAR szTemp[9];
const int NUMDIGITS = 8;
	if (m_TextBuffer == NULL)
		return ;
	HexConv::long_to_hex((bit32)v, szTemp, m_iNumChars);
	_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexAddress(int v)
{
TCHAR szTemp[5];
const int NUMDIGITS = 4;
	if (m_TextBuffer == NULL || m_iNumChars < NUMDIGITS)
		return ;
	HexConv::long_to_hex(v & 0xffff, szTemp, NUMDIGITS);
	_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexByte(int v)
{
TCHAR szTemp[3];
const int NUMDIGITS = 2;
	if (m_TextBuffer == NULL)
		return ;
	HexConv::long_to_hex(v & 0xff, szTemp, NUMDIGITS);
	_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}


HRESULT EdLn::GetDec(int& v)
{
HRESULT hr = E_FAIL;

	if (m_TextBuffer == NULL || m_iNumChars <= 0)
		return E_FAIL;

	int r = _stscanf_s(m_TextBuffer, TEXT(" %d"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}
	return S_OK;
}

void EdLn::SetDec(int v)
{
	_sntprintf_s(m_TextBuffer, m_iTextBufferLength, _TRUNCATE, TEXT("%d"), v);
}

size_t EdLn::GetString(TCHAR buffer[], int bufferSize)
{
	if (m_TextBuffer == NULL)
		return E_FAIL;
	size_t k;
	if (buffer == NULL || bufferSize == 0)
	{
		k = _tcsnlen(m_TextBuffer, m_iTextBufferLength);
		return k;
	}
	else
	{
		k = _tcsnlen(m_TextBuffer, m_iTextBufferLength);
		if (k == 0)
		{
			buffer[0];
		}
		else if (k < m_iTextBufferLength)
		{
			//NULL terminated m_TextBuffer
			_tcsncpy_s(buffer, bufferSize, m_TextBuffer, _TRUNCATE);
		}
		else 
		{
			//m_TextBuffer is not NULL terminated.
			if (k < bufferSize)
			{
				_tcsncpy_s(buffer, bufferSize, m_TextBuffer, k);
			}
			else
			{
				_tcsncpy_s(buffer, bufferSize, m_TextBuffer, bufferSize - 1);
			}
		}
		return k;
	}
}

void EdLn::SetString(const TCHAR *data, int count)
{
	if (m_TextBuffer == NULL || m_iTextBufferLength <= 0)
		return;
	if (data == NULL || count == 0)
	{
		m_TextBuffer[0] = 0;
		return;
	}

	size_t k = _tcsnlen(data, count);
	if (k == 0)
	{
		m_TextBuffer[0] = 0;
	}
	else if (k < count)
	{
		//NULL terminated data
		_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, data, _TRUNCATE);
	}
	else 
	{
		//data is not NULL terminated.
		if (k < m_iTextBufferLength)
		{
			_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, data, k);
		}
		else
		{
			_tcsncpy_s(m_TextBuffer, m_iTextBufferLength, data, m_iTextBufferLength - 1);
		}
	}
}