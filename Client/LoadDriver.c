#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")   // SCM 函数都在 advapi32.dll

/*  功能：把指定路径的 .sys 注册并启动
    参数：szSvcName  服务（驱动）内部名
          szSysPath  驱动完整路径，如 C:\\MyDriver.sys
    返回：TRUE  成功（已启动）
          FALSE 失败（打印 LastError） */
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

int wmain(int argc, WCHAR *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %S <driver.sys>\n", argv[0]);
        return 0;
    }

    WCHAR szSvc[256] = { 0 };
    _wsplitpath_s(argv[1], NULL, 0, NULL, 0, szSvc, 256, NULL, 0);

    if (LoadDriver(szSvc, argv[1]))
        printf("Load OK\n");
    else
        printf("Load failed\n");

    return 0;
}