#define MAX_VAL_ULONG                               ((ULONG)~((ULONG)0))
#define MAX_VAL_LONG                                ((LONG)(MAX_VAL_ULONG >> 1))
#define DNS_RESOLVE_TIME_OUT_MIN                    (50)
#define DNS_RESOLVE_TIME_OUT_MAX                    (500)

#include "AList.h"
#include "RBTree.h"

////////////////////////////////////////////////////////////////////////////////
//CDNS_Item
class CDNS_Item
{                            
    wchar_t         *m_pName;      //host name 
    wchar_t          m_pIP[96];
    size_t           m_szName_Max; //host name max length
    size_t           m_szName;     //host name current length
    sockaddr_storage m_sSAS;       //host Socket address V4/V6
    HANDLE           m_hEvent;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CDNS_Item
    CDNS_Item(sockaddr *i_pSA)
        : m_pName(NULL)
        , m_szName_Max(0)
        , m_szName(0)
        , m_hEvent(NULL)
    {
        DWORD l_dwSA_Size = 0;
        DWORD l_dwIP_Len  = LENGTH(m_pIP);

        if (AF_INET == i_pSA->sa_family)
        {
            memcpy(&m_sSAS, i_pSA, sizeof(sockaddr_in));
            ((sockaddr_in*)&m_sSAS)->sin_port = 0;
            l_dwSA_Size = sizeof(sockaddr_in);
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            memcpy(&m_sSAS, i_pSA, sizeof(sockaddr_in6));
            ((sockaddr_in6*)&m_sSAS)->sin6_port = 0;
            l_dwSA_Size = sizeof(sockaddr_in6);
        }

        if (0 == WSAAddressToStringW((sockaddr*)&m_sSAS,
                                     l_dwSA_Size,
                                     NULL,
                                     m_pIP,
                                     (LPDWORD)&l_dwIP_Len
                                    )
           )
        {
            size_t l_szIP = wcslen(m_pIP);

            for (size_t l_szI = 0; l_szI < l_szIP; l_szI ++)
            {
                if (L':' == m_pIP[l_szI])
                {
                    m_pIP[l_szI] = L'x';
                }
            }
        }
        else
        {
            wcscpy_s(m_pIP, LENGTH(m_pIP), L"Unknown"); 
        }

        m_hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    }//CDNS_Item

    ////////////////////////////////////////////////////////////////////////////
    //~CDNS_Item
    ~CDNS_Item()
    {
        if (m_pName)
        {
            delete [] m_pName;
            m_pName = NULL;
        }

        if (m_hEvent)
        {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //Get_SA
    const sockaddr *Get_SA()
    {
        return (sockaddr*)&m_sSAS;
    }//Get_SA


    ////////////////////////////////////////////////////////////////////////////
    //Get_Event
    HANDLE Get_Event()
    {
        return m_hEvent;
    }//Get_Event

    ////////////////////////////////////////////////////////////////////////////
    //Update
    tBOOL Update(wchar_t *i_pName)
    {
        wchar_t *l_pName  = (i_pName) ? i_pName : m_pIP;
        size_t   l_szName = wcslen(l_pName);

        if (l_szName > m_szName_Max)
        {
            if (m_pName)
            {
                delete [] m_pName;
                m_pName = NULL;
            }

            m_szName_Max = (l_szName + 1 + 15) & (~15UL);
            m_pName      = new wchar_t[m_szName_Max];
        }

        if (NULL != m_pName)
        {
            m_szName = l_szName;
            wcscpy_s(m_pName, m_szName_Max, l_pName);
        }
        else
        {
            m_szName     = 0;
            m_szName_Max = 0;
        }

        return (m_pName) ? TRUE : FALSE;
    }//Update


    ////////////////////////////////////////////////////////////////////////////
    //Get_Name()
    tBOOL Get_Name(wchar_t *o_pName, size_t i_szLength_Max)
    {
        if (    (o_pName)
             && (1 < i_szLength_Max)
           )
        {
            if (m_szName >= i_szLength_Max)
            {
                wcsncpy_s(o_pName, i_szLength_Max, m_pName, i_szLength_Max - 1);
            }
            else
            {
                wcscpy_s(o_pName, i_szLength_Max, m_pName);
            }

            return TRUE;
        }

        return FALSE;
    }//Get_Name()

    ////////////////////////////////////////////////////////////////////////////
    //Is_Greater -> (m_sSA > i_pSA) == TRUE
    tBOOL Is_Greater(sockaddr *i_pSA)
    {
        if (m_sSAS.ss_family > i_pSA->sa_family)
        {
            return TRUE;
        }
        else if (m_sSAS.ss_family < i_pSA->sa_family)
        {
            return FALSE;
        }

        if (AF_INET  == i_pSA->sa_family)
        {
            return (((sockaddr_in*)&m_sSAS)->sin_addr.S_un.S_addr > ((sockaddr_in*)i_pSA)->sin_addr.S_un.S_addr);
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            return (0 < memcmp(((sockaddr_in6*)&m_sSAS)->sin6_addr.u.Byte, 
                               ((sockaddr_in6*)i_pSA)->sin6_addr.u.Byte, 
                               sizeof(in6_addr)
                              )
                   );
        }

        return FALSE;
    }//Is_Greater

    ////////////////////////////////////////////////////////////////////////////
    //Is_Equal
    tBOOL Is_Equal(sockaddr *i_pSA)
    {
        if (m_sSAS.ss_family == i_pSA->sa_family)
        {
            if (AF_INET  == i_pSA->sa_family)
            {
                return (((sockaddr_in*)&m_sSAS)->sin_addr.S_un.S_addr == ((sockaddr_in*)i_pSA)->sin_addr.S_un.S_addr);
            }
            else if (AF_INET6 == i_pSA->sa_family)
            {
                return (0 == memcmp(((sockaddr_in6*)&m_sSAS)->sin6_addr.u.Byte, 
                                    ((sockaddr_in6*)i_pSA)->sin6_addr.u.Byte, 
                                    sizeof(in6_addr)
                                   )
                       );
            }
        }

        return FALSE;
    }//Is_Equal
};
                              

////////////////////////////////////////////////////////////////////////////////
//CDNS_Resolver
class CDNS_Resolver:
    public CRBTree<CDNS_Item*, sockaddr*>
{
    volatile LONG     m_lTimeOut;
    CRITICAL_SECTION  m_sCS;
    HANDLE            m_hTh_Main;
    HANDLE            m_hEv_Exit;
    HANDLE            m_hSm_Resovle; 
    CBList<CDNS_Item*> m_cRecords;
    CBList<CDNS_Item*> m_cResolving;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CDNS_Resolver()
    CDNS_Resolver()
        : CRBTree<CDNS_Item*, sockaddr*>(32, FALSE)
        , m_hEv_Exit(NULL)
        , m_hTh_Main(NULL)
        , m_cRecords(32)
        , m_hSm_Resovle(NULL)
        , m_lTimeOut(DNS_RESOLVE_TIME_OUT_MAX)
    {
        InitializeCriticalSection(&m_sCS);

        m_hSm_Resovle = CreateSemaphore(NULL, 0, MAX_VAL_LONG, NULL);
        m_hEv_Exit    = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (    (m_hEv_Exit)
             && (m_hSm_Resovle)
           )
        {
            m_hTh_Main = (HANDLE)_beginthreadex( NULL, 0, Routine_Static, this, 0, NULL);
        }
    }//~CDNS_Resolver()

    ////////////////////////////////////////////////////////////////////////////
    //~CDNS_Resolver()
    ~CDNS_Resolver()
    {
        if (m_hTh_Main)
        {
            SetEvent(m_hEv_Exit);

            if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hTh_Main, 30000))
            {
                CloseHandle(m_hTh_Main);
            }

            m_hTh_Main = NULL;
        }

        if (m_hEv_Exit)
        {
            CloseHandle(m_hEv_Exit);
            m_hEv_Exit = NULL;
        }

        if (m_hSm_Resovle)
        {
            CloseHandle(m_hSm_Resovle);
            m_hSm_Resovle = NULL;
        }

        m_cResolving.Clear(FALSE);
        m_cRecords.Clear(TRUE);

        DeleteCriticalSection(&m_sCS);
    }//~CDNS_Resolver()

    ////////////////////////////////////////////////////////////////////////////
    //Get_Name()
    tBOOL Get_Name(sockaddr *i_pKey, wchar_t *o_pName, size_t i_szName, tBOOL *o_pNew)
    {
        CDNS_Item *l_pRec    = NULL; 
        tBOOL     l_bAdd    = FALSE;

        if (    (NULL == i_pKey)
             || (    (AF_INET  != i_pKey->sa_family)
                  && (AF_INET6 != i_pKey->sa_family)
                )
             || (NULL == o_pName)
             || (16 > i_szName)
           )
        {
            return FALSE;
        }

        EnterCriticalSection(&m_sCS);
        l_pRec = this->Find(i_pKey);
        if (NULL == l_pRec)
        {
            l_pRec = new CDNS_Item(i_pKey);
            if (l_pRec)
            {
                this->Push(l_pRec, i_pKey);
                m_cRecords.Add_After(m_cRecords.Get_Last(), l_pRec);
                l_pRec->Update(NULL);
                l_bAdd = TRUE;
            }

            if (o_pNew)
            {
                *o_pNew = 1;
            }
        }
        LeaveCriticalSection(&m_sCS);

        if (l_bAdd)
        {
            EnterCriticalSection(&m_sCS);
            m_cResolving.Add_After(m_cResolving.Get_Last(), l_pRec);
            ReleaseSemaphore(m_hSm_Resovle, 1, NULL);
            LeaveCriticalSection(&m_sCS);

            WaitForSingleObject(l_pRec->Get_Event(), (DWORD)m_lTimeOut);
        }//if (NULL == l_pRec)

        EnterCriticalSection(&m_sCS);
        if (l_pRec)
        {
            l_pRec->Get_Name(o_pName, i_szName);
        }
        LeaveCriticalSection(&m_sCS);

        return (NULL != l_pRec);
    }//Get_Name()

protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(sockaddr *i_pKey, CDNS_Item *i_pData) 
    {
        return i_pData->Is_Greater(i_pKey);
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(sockaddr *i_pKey, CDNS_Item *i_pData) 
    {
        return i_pData->Is_Equal(i_pKey);
    }

private:
    //////////////////////////////////////////////////////////////////////////// 
    //Routine_Static
    static unsigned int __stdcall Routine_Static(LPVOID i_pContext)
    {
        CDNS_Resolver *l_pThis = static_cast<CDNS_Resolver*>(i_pContext);
        if (l_pThis)
        {
            l_pThis->Routine();
        }

        return 0;
    }//Routine_Static

    //////////////////////////////////////////////////////////////////////////// 
    //Routine
    void Routine()
    {
        wchar_t     l_pName[NI_MAXHOST];
        BOOL        l_bExit     = FALSE;
        pAList_Cell l_pEl       = NULL;
        CDNS_Item   *l_pRec      = NULL; 
        HANDLE      l_hEvents[] = {m_hEv_Exit, m_hSm_Resovle};
        DWORD       l_dwWFMOR   = 0;
        LONG        l_lWaitMax  = -1;
        DWORD       l_dwTick    = 0;

        while (FALSE == l_bExit)
        {
            l_dwWFMOR = WaitForMultipleObjects(LENGTH(l_hEvents), l_hEvents, FALSE, 1000);

            if (WAIT_OBJECT_0 == l_dwWFMOR)
            {
                l_bExit = TRUE;
            }
            else if ((WAIT_OBJECT_0 + 1)  == l_dwWFMOR)
            {
                EnterCriticalSection(&m_sCS);
                l_pRec = m_cResolving.Get_Data(m_cResolving.Get_First());
                m_cResolving.Del(m_cResolving.Get_First(), FALSE);
                LeaveCriticalSection(&m_sCS);

                if (l_pRec)
                {
                    l_dwTick = GetTickCount();
                    if (Resolve_SA(l_pRec->Get_SA(), l_pName, LENGTH(l_pName)))
                    {
                        l_dwTick = CTicks::Difference(GetTickCount(), l_dwTick);
                        EnterCriticalSection(&m_sCS);
                        l_pRec->Update(l_pName);
                        LeaveCriticalSection(&m_sCS);
                    }
                    else
                    {
                        l_dwTick = 0;

                        EnterCriticalSection(&m_sCS);
                        l_pRec->Update(NULL);
                        LeaveCriticalSection(&m_sCS);
                    }
                    SetEvent(l_pRec->Get_Event());
                }

                //Update waiting timeout based on maximum waiting of response 
                //from DNS server
                if ((LONG)l_dwTick > l_lWaitMax)
                {
                    //set timeout with max limit of DNS_RESOLVE_TIME_OUT
                    if (l_dwTick > DNS_RESOLVE_TIME_OUT_MAX)
                    {
                        l_lWaitMax = DNS_RESOLVE_TIME_OUT_MAX;
                    }
                    else if (l_dwTick < DNS_RESOLVE_TIME_OUT_MIN)
                    {
                        l_lWaitMax = DNS_RESOLVE_TIME_OUT_MIN;
                    }
                    else
                    {
                        l_lWaitMax = (LONG)l_dwTick;
                    }
                    
                    if (l_lWaitMax != m_lTimeOut)
                    {
                        InterlockedExchange(&m_lTimeOut, l_lWaitMax);
                    }
                }
            }
            else if (WAIT_TIMEOUT == l_dwWFMOR)
            {
                EnterCriticalSection(&m_sCS);
                l_pEl = m_cRecords.Get_Next(l_pEl);
                l_pRec = m_cRecords.Get_Data(l_pEl);
                LeaveCriticalSection(&m_sCS);

                if (l_pRec)
                {
                    if (Resolve_SA(l_pRec->Get_SA(), l_pName, LENGTH(l_pName)))
                    {
                        EnterCriticalSection(&m_sCS);
                        l_pRec->Update(l_pName);
                        LeaveCriticalSection(&m_sCS);
                    }
                    else
                    {
                        EnterCriticalSection(&m_sCS);
                        l_pRec->Update(NULL);
                        LeaveCriticalSection(&m_sCS);
                    }
                }
            }
        }
    }//Routine

    //////////////////////////////////////////////////////////////////////////// 
    //Resolve_SA
    tBOOL Resolve_SA(const sockaddr *i_pSA, wchar_t *o_pName, tUINT32 i_dwLength)
    {
        tUINT32 l_dwIP_Size = 0;
        tBOOL   l_bReturn   = FALSE;

        if (NULL == i_pSA)
        {
            return l_bReturn;
        }


        if (AF_INET == i_pSA->sa_family)
        {
            l_dwIP_Size = sizeof(sockaddr_in);
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            l_dwIP_Size = sizeof(sockaddr_in6);
        }                                             

        int l_iRes = GetNameInfoW(i_pSA, l_dwIP_Size, 
                                  o_pName, i_dwLength, 
                                  NULL, 0, 
                                  NI_NAMEREQD
                                 );

        if (0 == l_iRes) 
        {
            l_bReturn = TRUE;
        }

        return l_bReturn;
    }//Resolve_SA
};//CDNS_Resolver