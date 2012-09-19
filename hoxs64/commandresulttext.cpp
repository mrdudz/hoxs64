#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "bits.h"
#include "mlist.h"
#include "carray.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"

#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"

CommandResultText::CommandResultText(ICommandResult *pCommandResult, LPCTSTR pText)
{
	this->m_pCommandResult  = pCommandResult;	
	if (pText)
		this->m_pCommandResult->AddLine(pText);
}

HRESULT CommandResultText::Run()
{
	return S_OK;
}
