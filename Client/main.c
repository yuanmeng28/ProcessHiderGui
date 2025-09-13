#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define IOCTL_GET_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL GetSysFullPath(const WCHAR * szSysName, WCHAR * bufOut);
BOOL LoadDriver(const WCHAR *szSvcName, const WCHAR *szSysPath);
BOOL UnloadDriver(const WCHAR * szSvcName);

typedef struct _DATA_PID {
    HANDLE pid;
} DATA_PID, *PDATA_PID;

int wmain()
{
    WCHAR driverPath[MAX_PATH] = { 0 };
    // L"ProcessHiderCore.sys"
    if (!GetSysFullPath(L"ProcessHiderCore.sys", driverPath))
    {
        printf("获取驱动路径失败，error=%lu\n", GetLastError());
        return 0;
    }

    printf("驱动路径: %ws\n\n", driverPath);

    printf("请选择你要执行的操作：\n");
    printf("1.加载&启动驱动 2.隐藏进程 3.停止&卸载驱动\n");

    int choice;
    scanf_s("%d", &choice);

    switch (choice)
    {
    case 1:
    {
        WCHAR serviceName[] = L"ProcessHiderCore";
        if (LoadDriver(serviceName, driverPath))
            printf("驱动加载并启动成功。\n");
        else
            printf("驱动加载或启动失败。\n");
        break;
    }
    case 2:
        // 隐藏进程的代码将在后续部分实现
        break;
    case 3:
    {
        WCHAR serviceName[] = L"ProcessHiderCore";
        if (UnloadDriver(serviceName))
            printf("驱动停止并卸载成功。\n");
        else
            printf("驱动停止或卸载失败。\n");
        break;
    }
    default:
        break;
    }

    HANDLE hDev = NULL;
    DATA_PID pid = { 0 };
    DWORD  bytesReturned = 0;
    BOOL   ok;

    hDev = CreateFile(L"\\\\.\\ProcessHiderCore",
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL
    );
    if (hDev == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile 失败，error=%lu\n", GetLastError());
        printf("----驱动未成功加载----\n");
        system("pause");
        return 0;
    }

    int nPid;
    printf("请输入要隐藏的进程 PID：");
    scanf_s("%u", &nPid);
    pid.pid = (HANDLE)(ULONG_PTR)nPid;

    ok = DeviceIoControl(hDev,
        IOCTL_GET_PID,
        &pid, sizeof(pid),
        &pid, sizeof(pid),
        &bytesReturned, NULL
    );

    if (!ok)
    {
        printf("DeviceIoControl 失败，error=%lu\n", GetLastError());
    }

    CloseHandle(hDev);
    system("pause");
    return 0;
}


BOOL GetSysFullPath(LPCWSTR szSysName, LPWSTR bufOut)
{
    WCHAR szDir[MAX_PATH];

    if (!GetModuleFileNameW(NULL, szDir, MAX_PATH))
        return FALSE;
    PathRemoveFileSpecW(szDir);

    return PathCombineW(bufOut, szDir, szSysName) != NULL;
}

BOOL LoadDriver(const WCHAR *szSvcName, const WCHAR *szSysPath)
{
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hSvc = NULL;
    BOOL      bOk = FALSE;
    DWORD     err;

    hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        printf("OpenSCManager failed: %lu\n", GetLastError());
        return FALSE;
    }

    hSvc = CreateServiceW(
        hSCM,
        szSvcName, szSvcName, SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL, szSysPath,
        NULL, NULL, NULL, NULL, NULL
    );

    err = GetLastError();
    if (hSvc == NULL && err == ERROR_SERVICE_EXISTS)
    {
        hSvc = OpenServiceW(hSCM, szSvcName, SERVICE_ALL_ACCESS);
        err = hSvc ? ERROR_SUCCESS : GetLastError();
    }

    if (hSvc == NULL)
    {
        printf("CreateService/OpenService failed: %lu\n", err);
        goto Cleanup;
    }

    if (!StartServiceW(hSvc, 0, NULL))
    {
        err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING)
        {
            printf("Driver already running.\n");
            bOk = TRUE;
        }
        else
            printf("StartService failed: %lu\n", err);
    }
    else
    {
        printf("Driver started successfully.\n");
        bOk = TRUE;
    }

Cleanup:
    if (hSvc) CloseServiceHandle(hSvc);
    if (hSCM) CloseServiceHandle(hSCM);
    return bOk;
}

BOOL UnloadDriver(const WCHAR *szSvcName)
{
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) return FALSE;

    SC_HANDLE hSvc = OpenServiceW(hSCM, szSvcName, SERVICE_ALL_ACCESS);
    if (hSvc)
    {
        SERVICE_STATUS ss;
        ControlService(hSvc, SERVICE_CONTROL_STOP, &ss);
        DeleteService(hSvc);
        CloseServiceHandle(hSvc);
    }
    CloseServiceHandle(hSCM);
    return TRUE;
}