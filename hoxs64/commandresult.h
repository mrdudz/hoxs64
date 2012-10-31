#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__


#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)

class CommandResult : public ICommandResult
{
public:
	CommandResult(IMonitor *pIMonitor, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex);
	~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	DBGSYM::CliCommandStatus::CliCommandStatus m_status;

	//ICommandResult
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString, int id);
	virtual HRESULT Quit();
	virtual bool IsFinished();
	virtual bool IsQuit();
	virtual DWORD WaitLinesTakenOrQuit(DWORD timeout);
	virtual DWORD WaitFinished(DWORD timeout);
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus();
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status);
	virtual void Reset();
	virtual void AddLine(LPCTSTR pszLine);
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
	virtual size_t CountUnreadLines();
	virtual int GetId();
	virtual IMonitor* GetMonitor();
	virtual CommandToken* GetToken();
	virtual void *GetData();
	virtual void SetData(void *p);
protected:
	HRESULT CreateCliCommandResult(CommandToken *pCommandToken, IRunCommand **ppRunCommand);
	virtual HRESULT Run();
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	void PostFinished();
	void SetFinished();
	size_t line;
	std::vector<LPTSTR> a_lines;
	HWND m_hWnd;
	HANDLE m_hThread;
	HANDLE m_hevtQuit;
	HANDLE m_hevtLineTaken;
	HANDLE m_mux;
	DWORD m_dwThreadId;
	int m_id;
	IMonitor *m_pIMonitor;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
	CommandToken *m_pCommandToken;
	void *m_data;
private:
    CommandResult(CommandResult const &);
    CommandResult & operator=(CommandResult const &);

	std::basic_string<TCHAR> m_sCommandLine;
	bool m_bIsFinished;
	bool m_bIsQuit;
	void InitVars();
	void Cleanup();
};

#endif
