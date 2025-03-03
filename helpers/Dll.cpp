//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Standard dll required functions and class factory implementation.

#include "Header.h"
#include <windows.h>
#include <unknwn.h>
#include "Dll.h"
#include "helpers.h"
#include "FileLog.h"
#include "Utils.h"

static LONG g_cRef = 0;   // global dll reference count
HINSTANCE g_hinst = NULL; // global dll hinstance

static Utils::CFileLog g_log;

extern HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);
extern HRESULT CFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv);
EXTERN_C GUID CLSID_CSample;

class CClassFactory : public IClassFactory
{
public:
    CClassFactory() : _cRef(1) 
    {
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv)
    {
        static const QITAB qit[] = 
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(__in IUnknown* pUnkOuter, __in REFIID riid, __deref_out void **ppv)
    {
        HRESULT hr;
        if (!pUnkOuter)
        {
			if (IID_ICredentialProvider == riid) {
				hr = CSample_CreateInstance(riid, ppv);
			}
			else if (IID_ICredentialProviderFilter == riid) {
				hr = CFilter_CreateInstance(riid, ppv);
			}
			else {
				*ppv = NULL;
				hr = CLASS_E_NOAGGREGATION;
			}
        }
        else
        {
            *ppv = NULL;
            hr = CLASS_E_NOAGGREGATION;
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(__in BOOL bLock)
    {
        if (bLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory()
    {
    }
    long _cRef;
};

HRESULT CClassFactory_CreateInstance(__in REFCLSID rclsid, __in REFIID riid, __deref_out void **ppv)
{
    *ppv = NULL;

    HRESULT hr;

    if (CLSID_CSample == rclsid)
    {
        CClassFactory* pcf = new CClassFactory();
        if (pcf)
        {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hr;
}

void DllAddRef()
{
    InterlockedIncrement(&g_cRef);
    Utils::Output(Utils::StringFormat(L"DllAddRef g_cRef: %d", g_cRef));
}

void DllRelease()
{
    InterlockedDecrement(&g_cRef);
    Utils::Output(Utils::StringFormat(L"DllRelease g_cRef: %d", g_cRef));
}

STDAPI DllCanUnloadNow()
{
    HRESULT hr = (g_cRef > 0) ? S_FALSE : S_OK;
    if (SUCCEEDED(hr)) {
        Utils::Output(L"DllCanUnloadNow - S_OK");
        Utils::SetLog(NULL);
    } else {
        Utils::Output(L"DllCanUnloadNow - S_FALSE");
    }
    return hr;
}

STDAPI DllGetClassObject(__in REFCLSID rclsid, __in REFIID riid, __deref_out void** ppv)
{
    Utils::SetLog(&g_log);
    Utils::Output(L"DllGetClassObject");
    return CClassFactory_CreateInstance(rclsid, riid, ppv);
}

STDAPI_(BOOL) DllMain(__in HINSTANCE hinstDll, __in DWORD dwReason, __in void *)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDll);
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    
    g_hinst = hinstDll;
    return TRUE;
}

