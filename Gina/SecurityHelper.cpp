#include "SecurityHelper.h"
#include "Utils.h"

#define LOGON_PROCESS_NAME "GINA Sample"

// excerpt from a DDK header file that we can't easily include here
#define STATUS_PASSWORD_EXPIRED         ((NTSTATUS)0xC0000071L)

static int _stringLenInBytes(const wchar_t * s) {
    if (!s) {
        return 0;
    } else {
        return (lstrlen(s) + 1) * sizeof(*s);
    }
}

static void _initUnicodeString(UNICODE_STRING * target, wchar_t * source, USHORT cbMax) {
    target->Length        = cbMax - sizeof(*source);
    target->MaximumLength = cbMax;
    target->Buffer        = source;
}

static MSV1_0_INTERACTIVE_LOGON * _allocLogonRequest(const wchar_t * domain, const wchar_t * user, const wchar_t * pass, DWORD * pcbRequest) {
    const DWORD cbHeader = sizeof(MSV1_0_INTERACTIVE_LOGON);
    const DWORD cbDom    = _stringLenInBytes(domain);
    const DWORD cbUser   = _stringLenInBytes(user);
    const DWORD cbPass   = _stringLenInBytes(pass);

    // sanity check string lengths
    if (cbDom > USHRT_MAX || cbUser > USHRT_MAX || cbPass > USHRT_MAX) {
        Utils::Output(L"_allocLogonRequest - Input string was too long");
        return 0;
    }

    *pcbRequest = cbHeader + cbDom + cbUser + cbPass;

    MSV1_0_INTERACTIVE_LOGON * pRequest = (MSV1_0_INTERACTIVE_LOGON *)new char[*pcbRequest];
    if (!pRequest) {
        Utils::Output(L"_allocLogonRequest pRequest: Out Of Memory");
        return 0;
    }

    pRequest->MessageType = MsV1_0InteractiveLogon;

    char * p = (char *)(pRequest + 1); // point past MSV1_0_INTERACTIVE_LOGON header

    wchar_t * pDom  = (wchar_t *)(p);
    wchar_t * pUser = (wchar_t *)(p + cbDom);
    wchar_t * pPass = (wchar_t *)(p + cbDom + cbUser);

    CopyMemory(pDom,  domain, cbDom);
    CopyMemory(pUser, user,   cbUser);
    CopyMemory(pPass, pass,   cbPass);

    _initUnicodeString(&pRequest->LogonDomainName, pDom,  (USHORT)cbDom);
    _initUnicodeString(&pRequest->UserName,        pUser, (USHORT)cbUser);
    _initUnicodeString(&pRequest->Password,        pPass, (USHORT)cbPass);

    return pRequest;
}

static bool _newLsaString(LSA_STRING * target, const char * source) {
    if (0 == source) {
        return false;
    }

    const int cch = lstrlenA(source);
    const int cchWithNullTerminator = cch + 1;

    // UNICODE_STRINGs have a size limit
    if (cchWithNullTerminator * sizeof(*source) > USHRT_MAX) {
        return false;
    }

    char * newStr = new char[cchWithNullTerminator];
    if (!newStr) {
        Utils::Output(L"_newLsaString newStr: Out Of Memory");
        return false;
    }

    CopyMemory(newStr, source, cchWithNullTerminator * sizeof(*newStr));

    target->Length        = (USHORT)cch                   * sizeof(*newStr);
    target->MaximumLength = (USHORT)cchWithNullTerminator * sizeof(*newStr);
    target->Buffer        = newStr;

    return true;
}

static void _deleteLsaString(LSA_STRING * target) {
    delete target->Buffer;
    target->Buffer = 0;
}

bool SecurityHelper::SetSeTcbPrivilege() {
    TOKEN_PRIVILEGES tp;
    LUID luid;
    HANDLE hProcessToken;

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken)) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::SetSeTcbPrivilege OpenProcessToken Error: %s", Utils::GetLastErrorString().c_str()));
        return false;
    }

    if (!::LookupPrivilegeValue(NULL, SE_TCB_NAME, &luid)) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::SetSeTcbPrivilege LookupPrivilegeValue Error: %s", Utils::GetLastErrorString().c_str()));
        ::CloseHandle(hProcessToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!::AdjustTokenPrivileges(hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::SetSeTcbPrivilege AdjustTokenPrivileges Error: %s", Utils::GetLastErrorString().c_str()));
        ::CloseHandle(hProcessToken);
        return false;
    }

    ::CloseHandle(hProcessToken);
    return true;
}

bool SecurityHelper::RegisterLogonProcess(HANDLE * phLsa) {
    *phLsa = 0;

    LSA_STRING name;
    if (!_newLsaString(&name, LOGON_PROCESS_NAME)) {
        return false;
    }

    LSA_OPERATIONAL_MODE unused;
    NTSTATUS status = ::LsaRegisterLogonProcess(&name, phLsa, &unused);

    _deleteLsaString(&name);

    if (status) {
        *phLsa = 0;
		Utils::Output(Utils::StringFormat(L"LsaRegisterLogonProcess Error: %s", Utils::GetErrorString(::LsaNtStatusToWinError(status)).c_str()));
        return false;
    }

    return true;
}

bool SecurityHelper::CallLsaLogonUser(HANDLE hLsa, const wchar_t * domain, const wchar_t * user, const wchar_t * pass, SECURITY_LOGON_TYPE logonType, LUID * pLogonSessionId, HANDLE * phToken, MSV1_0_INTERACTIVE_PROFILE * * ppProfile, DWORD * pWin32Error) {
    bool result      = false;
    DWORD win32Error = 0;
    *phToken         = 0;

    LUID ignoredLogonSessionId;

    // optional arguments
    if (ppProfile)        *ppProfile   = 0;
    if (pWin32Error)      *pWin32Error = 0;
    if (!pLogonSessionId) pLogonSessionId = &ignoredLogonSessionId;

    LSA_STRING logonProcessName             = { 0 };
    TOKEN_SOURCE sourceContext              = { 's', 'a', 'm', 'p', 'l', 'e' };
    MSV1_0_INTERACTIVE_PROFILE* pProfile    = 0;
    ULONG cbProfile                         = 0;
    QUOTA_LIMITS quotaLimits;
    NTSTATUS substatus;
    DWORD cbLogonRequest;

    MSV1_0_INTERACTIVE_LOGON* pLogonRequest = _allocLogonRequest(domain, user, pass, &cbLogonRequest);
    if (!pLogonRequest) {
        win32Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if (!_newLsaString(&logonProcessName, LOGON_PROCESS_NAME)) {
        win32Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    // LsaLogonUser - the function from hell
    NTSTATUS status = ::LsaLogonUser(
        hLsa,
        &logonProcessName,          // we use our logon process name for the "origin name"
        logonType,
        LOGON32_PROVIDER_DEFAULT,   // we use MSV1_0 or Kerb, whichever is supported
        pLogonRequest,
        cbLogonRequest,
        0,                          // we do not add any group SIDs
        &sourceContext,
        (void**)&pProfile,          // caller must free this via LsaFreeReturnBuffer 
        &cbProfile,
        pLogonSessionId,
        phToken,
        &quotaLimits,               // we ignore this, but must pass in anyway
        &substatus);

    if (status) {
        win32Error = ::LsaNtStatusToWinError(status);

        if ((ERROR_ACCOUNT_RESTRICTION == win32Error && STATUS_PASSWORD_EXPIRED == substatus)) {
            win32Error = ERROR_PASSWORD_EXPIRED;
        }

        *phToken = 0;
        pProfile = 0;
        Utils::Output(Utils::StringFormat(L"SecurityHelper::CallLsaLogonUser LsaLogonUser Error: %u, 0x%08X", win32Error, substatus));
        goto cleanup;
    }

    if (ppProfile) {
        *ppProfile = (MSV1_0_INTERACTIVE_PROFILE *)pProfile;
        pProfile = 0;
    }

    result = true;

cleanup:
    // if caller cares about the details, pass them on
    if (pWin32Error) {
        *pWin32Error = win32Error;
    }

    if (pLogonRequest) {
        SecureZeroMemory(pLogonRequest, cbLogonRequest);
        delete pLogonRequest;
    }

    if (pProfile) {
        ::LsaFreeReturnBuffer(pProfile);
    }

    _deleteLsaString(&logonProcessName);

    return result;
}

bool SecurityHelper::GetLogonSid(HANDLE htok, void * psid, DWORD cbMax) {
    DWORD cb;
    ::GetTokenInformation(htok, TokenGroups, 0, 0, &cb);
    TOKEN_GROUPS * ptg = (TOKEN_GROUPS *)::LocalAlloc(LMEM_FIXED, cb);
    if (!ptg) {
        Utils::Output(L"SecurityHelper::GetLogonSid ptg: Out Of Memory");
        return false;
    }

    bool success = false;
    if (::GetTokenInformation(htok, TokenGroups, ptg, cb, &cb)) {
        // search for the logon SID
		DWORD i;
        for (i = 0; i < ptg->GroupCount; ++i) {
            if (ptg->Groups[i].Attributes & SE_GROUP_LOGON_ID) {
                void* logonSid = ptg->Groups[i].Sid;
                const DWORD cb = ::GetLengthSid(logonSid);
                if (cbMax < cb) {
                    return false; // sanity check caller's buffer size
                }
                if (!::CopySid(cb, psid, logonSid)) {
                    Utils::Output(Utils::StringFormat(L"SecurityHelper::GetLogonSid CopySid Error: %s", Utils::GetLastErrorString().c_str()));
                    break;
                }
                success = true;
                break;
            }
        }
        if (i == ptg->GroupCount) {
            Utils::Output(L"SecurityHelper::GetLogonSid Failed to find a logon SID in the user's access token!");
        }
    } else {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::GetLogonSid GetTokenInformation Error: %s", Utils::GetLastErrorString().c_str()));
    }

    ::LocalFree(ptg);

    return success;
}

bool SecurityHelper::GetLogonSessionId(HANDLE htok, LUID * pluid) {
    TOKEN_STATISTICS stats;
    DWORD cb = sizeof stats;
    if (::GetTokenInformation(htok, TokenStatistics, &stats, cb, &cb)) {
        *pluid = stats.AuthenticationId;
        return true;
    } else {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::GetLogonSessionId GetTokenInformation Error: %s", Utils::GetLastErrorString().c_str()));
        return false;
    }
}

// caller must free *ppProfilePath using LocalFree
bool SecurityHelper::ExtractProfilePath(wchar_t * * ppProfilePath, MSV1_0_INTERACTIVE_PROFILE * pProfile) {
    *ppProfilePath = 0;
    if (0 == pProfile->ProfilePath.Length) {
        // no profile path was specified, so return a null pointer to WinLogon
        // to indicate that *it* should figure out the appropriate path
        return true;
    }

    bool result = false;

    const int cch = pProfile->ProfilePath.Length / sizeof(wchar_t);
    wchar_t * profilePath = (wchar_t *)::LocalAlloc(LMEM_FIXED, sizeof(wchar_t) * (cch + 1)); // I never assume a UNICODE_STRING is null terminated
    if (profilePath) {
        // copy the string data and manually null terminate it
        CopyMemory(profilePath, pProfile->ProfilePath.Buffer, pProfile->ProfilePath.Length);
        profilePath[cch] = L'\0';

        *ppProfilePath = profilePath;
    } else {
        Utils::Output(L"SecurityHelper::ExtractProfilePath profilePath: Out Of Memory");
    }

    return result;
}

bool SecurityHelper::AllocWinLogonProfile(WLX_PROFILE_V1_0 * * ppWinLogonProfile, const wchar_t * profilePath) {
    *ppWinLogonProfile = 0;
    if (!profilePath) {
        // no profile path was specified, so return a null pointer to WinLogon
        // to indicate that *it* should figure out the appropriate path
        return true;
    }

    bool result = false;

    // must use LocalAlloc for this - WinLogon will use LocalFree
    WLX_PROFILE_V1_0 * profile = (WLX_PROFILE_V1_0 *)::LocalAlloc(LMEM_FIXED, sizeof(WLX_PROFILE_V1_0));
    if (profile) {
        profile->dwType = WLX_PROFILE_TYPE_V1_0;

        const int cch = lstrlen(profilePath) + 1;
        
        wchar_t * newProfilePath = (wchar_t *)::LocalAlloc(LMEM_FIXED, cch * sizeof(*newProfilePath));
        if (newProfilePath) {
            // copy the string data and manually null terminate it
            CopyMemory(newProfilePath, profilePath, cch * sizeof(*newProfilePath));

            profile->pszProfile = newProfilePath;
            *ppWinLogonProfile = profile;

            result = true;
        } else {
            Utils::Output(L"SecurityHelper::AllocWinLogonProfile newProfilePath: Out Of Memory");
        }
    } else {
        Utils::Output(L"SecurityHelper::AllocWinLogonProfile profile: Out Of Memory");
    }

    return result;
}

bool SecurityHelper::CreateProcessAsUserOnDesktop(HANDLE hToken, wchar_t * programImage, wchar_t * desktop, void * env) {
    // impersonate the user to ensure that they are allowed
    // to execute the program in the first place
    if (!::ImpersonateLoggedOnUser(hToken)) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::CreateProcessAsUserOnDesktop - ImpersonateLoggedOnUser Error: %s", Utils::GetLastErrorString().c_str()));
        return false;
    }

    STARTUPINFO si = { sizeof(si), 0, desktop };
    PROCESS_INFORMATION pi;
    if (!::CreateProcessAsUser(hToken, programImage, programImage, 0, 0, FALSE, CREATE_UNICODE_ENVIRONMENT, env, 0, &si, &pi)) {
        ::RevertToSelf();
        Utils::Output(Utils::StringFormat(L"SecurityHelper::CreateProcessAsUserOnDesktop - CreateProcessAsUser Error: %s, programImage: %s", Utils::GetLastErrorString().c_str(), programImage));
        return false;
    }
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    ::RevertToSelf();

    Utils::Output(Utils::StringFormat(L"SecurityHelper::CreateProcessAsUserOnDesktop - Successfully, programImage: %s", programImage));
    return true;
}

bool SecurityHelper::ImpersonateAndGetUserName(HANDLE hToken, wchar_t * name, int cch) {
    bool result = false;
    if (::ImpersonateLoggedOnUser(hToken)) {
        DWORD cchName = cch;
        if (::GetUserName(name, &cchName)) {
            result = true;
        } else {
            Utils::Output(Utils::StringFormat(L"SecurityHelper::ImpersonateAndGetUserName - GetUserName Error: %s", Utils::GetLastErrorString().c_str()));
        }
        ::RevertToSelf();
    } else {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::ImpersonateAndGetUserName - ImpersonateLoggedOnUser Error: %s", Utils::GetLastErrorString().c_str()));
    }

    return result;
}

// checks user SID in both tokens for equality
bool SecurityHelper::IsSameUser(HANDLE hToken1, HANDLE hToken2, bool * pbIsSameUser) {
    *pbIsSameUser = false;
    bool result = false;

    const DWORD bufSize = sizeof(TOKEN_USER) + SECURITY_MAX_SID_SIZE;
    char buf1[bufSize];
    char buf2[bufSize];

    DWORD cb;
    if (::GetTokenInformation(hToken1, TokenUser, buf1, bufSize, &cb) &&
        ::GetTokenInformation(hToken2, TokenUser, buf2, bufSize, &cb)) {
        *pbIsSameUser = EqualSid(((TOKEN_USER *)buf1)->User.Sid, ((TOKEN_USER *)buf2)->User.Sid) ? true : false;
        result = true;
    } else {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::IsSameUser - GetTokenInformation Error: %s", Utils::GetLastErrorString().c_str()));
    }

    return result;
}

static void * _administratorsAlias() {
    const int subAuthCount = 2;
    static char sid[sizeof(SID) + subAuthCount * sizeof(DWORD)];
    
    SID * psid = (SID *)sid;
    if (0 == psid->Revision) {
        // first time called, initialize the sid
        psid->IdentifierAuthority.Value[5] = 5; // NT Authority
        psid->SubAuthorityCount = subAuthCount;
        psid->SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
        psid->SubAuthority[1] = DOMAIN_ALIAS_RID_ADMINS;
        psid->Revision = 1;
    }
    return sid;
}

// checks for presence of local admin group (must have TOKEN_DUPLICATE perms on hToken)
bool SecurityHelper::IsAdmin(HANDLE hToken) {
    // we can pretty much assume all tokens will be primary tokens in this application
    // and CheckTokenMembership requires an impersonation token (which is really annoying)
    // so we'll just duplicate any token we get into an impersonation token before continuing...
    bool isAdmin = false;
    HANDLE hImpToken;
    if (::DuplicateTokenEx(hToken, TOKEN_QUERY, 0, SecurityIdentification, TokenImpersonation, &hImpToken)) {
        BOOL isMember;
        if (::CheckTokenMembership(hImpToken, _administratorsAlias(), &isMember) && isMember) {
            isAdmin = true;
        } else {
            Utils::Output(Utils::StringFormat(L"SecurityHelper::IsAdmin - CheckTokenMembership Error: %s", Utils::GetLastErrorString().c_str()));
        }
        ::CloseHandle(hImpToken);
    } else {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::IsAdmin - DuplicateTokenEx Error: %s", Utils::GetLastErrorString().c_str()));
    }

    return isAdmin;
}

wchar_t * SecurityHelper::LocalAllocString(const wchar_t * s) {
    if (!s) {
        return 0;
    }

    const int cb = (1 + lstrlen(s)) * sizeof(*s);
    wchar_t * newStr = (wchar_t *)::LocalAlloc(LMEM_FIXED, cb);
    if (newStr) {
        CopyMemory(newStr, s, cb);
    } else {
        Utils::Output(L"SecurityHelper::LocalAllocString newStr: Out Of Memory");
    }
    return newStr;
}

static wchar_t * _dupStringRange(const wchar_t * begin, const wchar_t * end) {
    const int cch = (int)(end - begin);
    wchar_t * newStr = new wchar_t[cch + 1];
    if (newStr) {
        CopyMemory(newStr, begin, cch * sizeof(*newStr));
        newStr[cch] = L'\0';
    } else {
        Utils::Output(L"_dupStringRange newStr: Out Of Memory");
    }
    return newStr;
}

static bool _parseUserInitRegistryValue(const wchar_t * userinitValue, wchar_t * * * programList, int * count) {
    // the Userinit value is a comma-separated list of program files to be executed by Gina
    const wchar_t COMMA = L',';

    // quickly overestimate how many programs are in the list
    const wchar_t * it = userinitValue;
    int stringCountEstimate = 1;
    while (*it) {
        if (COMMA == *it++) {
            ++stringCountEstimate;
        }
    }

    // allocate the list
    *programList = new wchar_t *[stringCountEstimate];
    if (!*programList) {
        Utils::Output(L"_parseUserInitRegistryValue *programList: Out Of Memory");
        return false;
    }

    ZeroMemory(*programList, stringCountEstimate * sizeof(**programList));

    // parse the strings
    it = userinitValue;
    const wchar_t * begin = 0;

    *count = 0;
    while (true) {
        if (!begin) {
            begin = it;
        }

        if (COMMA == *it || 0 == *it) {
            const wchar_t * end = it;

            if (begin != end) {
                wchar_t * newStr = _dupStringRange(begin, end);
                if (!newStr) {
                    return false; // system is melting
                }

                (*programList)[*count] = newStr;
                ++*count;
            }

            if (0 == *it) {
                break;
            }

            begin = 0;
        }

        ++it;
    }

    return true;
}

bool SecurityHelper::ReadUserInitProgramList(wchar_t * * * programList, int * count) {
    HKEY hkey;
    LONG status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon", 0, KEY_QUERY_VALUE, &hkey);
    if (status) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::ReadUserInitProgramList - Failed to open WinLogon registry key: %ld", status));
        return false;
    }

    BYTE * userinitValue = 0;
    DWORD cb = 0;
    status = ::RegQueryValueEx(hkey, L"Userinit", 0, 0, 0, &cb);
    if (0 == status) {
        userinitValue = (BYTE *)::LocalAlloc(LMEM_FIXED, cb);
        if (!userinitValue) {
            Utils::Output(L"SecurityHelper::ReadUserInitProgramList userinitValue: Out Of Memory");
            return false;
        }

        status = ::RegQueryValueEx(hkey, L"Userinit", 0, 0, userinitValue, &cb);
    }
    ::RegCloseKey(hkey);

    if (status) {
        Utils::Output(Utils::StringFormat(L"SecurityHelper::ReadUserInitProgramList - Failed to read Userinit registry value: %ld", status));
        return false;
    }

    bool result = _parseUserInitRegistryValue((wchar_t *)userinitValue, programList, count);
    ::LocalFree(userinitValue);
    return result;
}

void SecurityHelper::FreeUserInitProgramList(wchar_t * * programList, int count) {
    wchar_t * * end = programList + count;
    for (wchar_t * * it = programList; it != end; ++it) {
        delete *it;
    }
    delete programList;
}
