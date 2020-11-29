#include "stdafx.h"
#include "TaskManager.h"

TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

bool TaskManager::ExistTaskJob(LPCTSTR wszTaskName)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	ITaskFolder* pRootFolder = NULL;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	IRegisteredTask* pRegisteredTask = NULL;
	hr = pRootFolder->GetTask(_bstr_t(wszTaskName), &pRegisteredTask);
	bool exists = (hr >= 0);
	pRootFolder->Release();
	CoUninitialize();
	return exists;
}

bool TaskManager::CreateTaskJob(LPCWSTR wszTaskName, wchar_t* nFullPath, wchar_t* nArguments)
{
	wchar_t pathName[MAX_PATH] = { 0 };
	wcscpy(pathName, nFullPath);
	wchar_t* pos = wcsrchr(pathName, '\\');
	*pos = '\0';

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	ITaskFolder* pRootFolder = NULL;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

	ITaskDefinition* pTask = NULL;
	hr = pService->NewTask(0, &pTask);
	pService->Release();

	IRegistrationInfo* pRegInfo = NULL;
	hr = pTask->get_RegistrationInfo(&pRegInfo);

	IPrincipal* pPrincipal = NULL;
	pTask->get_Principal(&pPrincipal);
	pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);

	hr = pRegInfo->put_Author(L"GPURAMDRIVE");
	pRegInfo->Release();

	ITaskSettings* pSettings = NULL;
	hr = pTask->get_Settings(&pSettings);

	pSettings->put_AllowDemandStart(VARIANT_FALSE);
	pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
	pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
	pSettings->put_MultipleInstances(TASK_INSTANCES_IGNORE_NEW);
	pSettings->put_ExecutionTimeLimit(_bstr_t(L"PT0S"));
	pSettings->put_StartWhenAvailable(VARIANT_TRUE);
	pSettings->Release();

	ITriggerCollection* pTriggerCollection = NULL;
	hr = pTask->get_Triggers(&pTriggerCollection);

	ITrigger* pTrigger = NULL;
	hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
	pTriggerCollection->Release();

	ILogonTrigger* pLogonTrigger = NULL;
	hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
	pTrigger->Release();

	hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
	//hr = pLogonTrigger->put_StartBoundary(_bstr_t(L"2020-01-01T12:05:00"));
	//hr = pLogonTrigger->put_EndBoundary(_bstr_t(L"2020-05-02T08:00:00"));
	//hr = pLogonTrigger->put_UserId(_bstr_t(L"DOMAIN\\UserName"));
	pLogonTrigger->Release();

	IActionCollection* pActionCollection = NULL;
	hr = pTask->get_Actions(&pActionCollection);

	IAction* pAction = NULL;
	hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	pActionCollection->Release();

	IExecAction* pExecAction = NULL;
	//  QI for the executable task pointer.
	hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
	pAction->Release();

	hr = pExecAction->put_Path(_bstr_t(nFullPath));
	pExecAction->put_Arguments(_bstr_t(nArguments));
	pExecAction->put_WorkingDirectory(_bstr_t(pathName));
	pExecAction->Release();

	IRegisteredTask* pRegisteredTask = NULL;

	hr = pRootFolder->RegisterTaskDefinition(
		_bstr_t(wszTaskName),
		pTask,
		TASK_CREATE_OR_UPDATE,
		_variant_t(L"S-1-5-32-544"),
		_variant_t(),
		TASK_LOGON_GROUP,
		_variant_t(L""),
		&pRegisteredTask);

	pRootFolder->Release();
	pTask->Release();
	pRegisteredTask->Release();
	CoUninitialize();

	return true;
}

bool TaskManager::DeleteTaskJob(LPCWSTR wszTaskName)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	ITaskFolder* pRootFolder = NULL;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

	pRootFolder->Release();
	CoUninitialize();

	return true;
}
