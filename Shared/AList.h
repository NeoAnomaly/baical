//******************************************************************************
// Copyright 2010, 2011, 2012  Zheltovskiy Andrey                              *
//                                                                             *
//   Licensed under the Apache License, Version 2.0 (the "License");           *
//   you may not use this file except in compliance with the License.          *
//   You may obtain a copy of the License at                                   *
//                                                                             *
//       http://www.apache.org/licenses/LICENSE-2.0                            *
//                                                                             *
//  Unless  required  by  applicable  law  or  agreed  to in writing, software *
//  distributed  under the License is distributed on an "AS IS" BASIS, WITHOUT *
//  WARRANTIES  OR  CONDITIONS OF ANY KIND, either express or implied. See the *
//  License  for  the  specific language governing permissions and limitations *
//  under the License.                                                         *
//                                                                             *
//******************************************************************************
// This header file contains simple template lists                             *
// 1. List with customizable memory manager (CAList)                           *
// 2. List with pooling of internal elements (CBList)                          *
//******************************************************************************
#ifndef ALIST_H_AZH
#define ALIST_H_AZH

typedef void *pAList_Cell;

////////////////////////////////////////////////////////////////////////////////
//CAList
template <typename tData_Type>
class CAList
{
protected:
    struct tCell
    {
        tData_Type  m_pData;
        tCell      *m_pNext;
        tCell      *m_pPrev;
    };

    typedef tCell* pIndex_Cell;

    tCell        *m_pFirst;
    tCell        *m_pLast;

    tUINT32       m_dwCount;
    //HANDLE        m_hLock_Mutex;

    pIndex_Cell  *m_pIndex;
    tBOOL         m_bBrokenIndex;
    tUINT32       m_dwIndexCount;

    tBOOL         m_bInitialized;

public:
    ////////////////////////////////////////////////////////////////////////////
    //CAList::CAList
    CAList()
    {
        m_bInitialized     = TRUE;
        m_pFirst           = NULL;
        m_pLast            = NULL;
        m_dwCount          = 0;
        m_pIndex           = NULL;
        m_bBrokenIndex     = TRUE;
        m_dwIndexCount     = 0;
        //m_hLock_Mutex      = CreateMutex(NULL, FALSE, NULL);

        //if (NULL == m_hLock_Mutex)
        //{
        //    m_bInitialized = FALSE;
        //}
    }//CAList::AList

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::~CAList
    ~CAList()
    {
        Index_Release();

        //if (m_hLock_Mutex)
        //{
        //    CloseHandle(m_hLock_Mutex);
        //    m_hLock_Mutex = NULL;
        //}

        if (m_dwCount)
        {
#ifdef _DEBUG
    #if defined(_WIN32) || defined(_WIN64)
            OutputDebugStringW(L"Warning: List is not empty\n");
    #endif
#endif
        }
    }//CAList::~CAList
    
   
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_Initialized
    tBOOL Get_Initialized()
    {
        return m_bInitialized;
    }//CAList::Get_Initialized

    ////////////////////////////////////////////////////////////////////////////
    //CAList::Del
    tBOOL Del(pAList_Cell i_pCell, tBOOL i_bFree_Data)
    {
        tCell  *l_pCell   = static_cast<tCell*>(i_pCell);
        tBOOL l_bResult = TRUE;

        if (l_pCell)
        {
            l_bResult = Cell_Release(l_pCell, TRUE, i_bFree_Data);
        }

        return l_bResult;
    }//CAList::Del


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Extract
    tBOOL Extract(pAList_Cell i_pCell)
    {
        tCell *l_pCell   = static_cast<tCell*>(i_pCell);
        tBOOL  l_bResult = TRUE;

        if (l_pCell)
        {
            l_bResult = Cell_Release(l_pCell, FALSE, FALSE);
            l_pCell->m_pNext = NULL;
            l_pCell->m_pPrev = NULL;
        }

        return l_bResult;
    }//CAList::Extract


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Add_After
    pAList_Cell Add_After(pAList_Cell i_pCell, tData_Type i_pData)
    {
        tCell * l_pNew_Cell = NULL;
        tCell * l_pList_Cell = static_cast<tCell *>(i_pCell);

        l_pNew_Cell = Cell_Allocate();

        if (l_pNew_Cell)
        {
            memset(l_pNew_Cell, 0, sizeof(tCell));
            l_pNew_Cell->m_pData = i_pData;
            l_pNew_Cell->m_pNext = NULL;
            l_pNew_Cell->m_pPrev = NULL;

            Put_After(l_pList_Cell, l_pNew_Cell);
        }

        return static_cast<pAList_Cell>(l_pNew_Cell);
    }//CAList::Add_After


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Put_After
    tBOOL Put_After(pAList_Cell i_pList_Cell, pAList_Cell i_pExt_Cell)
    {
        tCell   *l_pList_Cell = static_cast<tCell*>(i_pList_Cell);
        tCell   *l_pExt_Cell  = static_cast<tCell*>(i_pExt_Cell);
        tBOOL           l_bResult    = TRUE;

        if (NULL == l_pExt_Cell)
        {
            return FALSE;
        }

        m_dwCount++;

        if (l_pList_Cell)
        {
            l_pExt_Cell->m_pNext = l_pList_Cell->m_pNext;
            l_pExt_Cell->m_pPrev = l_pList_Cell;
            if (l_pList_Cell->m_pNext)
            {
                l_pList_Cell->m_pNext->m_pPrev = l_pExt_Cell;
            }
            l_pList_Cell->m_pNext = l_pExt_Cell;
        }


        if ((NULL == l_pList_Cell) && (m_pFirst))
        {
            m_pFirst->m_pPrev = l_pExt_Cell;
            l_pExt_Cell->m_pNext = m_pFirst;
            m_pFirst = l_pExt_Cell;
        }

        if ((m_pLast) && (m_pLast == l_pList_Cell))
        {
            m_pLast = l_pExt_Cell;
        }

        if (!m_pFirst)
        {
            m_pFirst = l_pExt_Cell;
            m_pLast = l_pExt_Cell;
        }

        m_bBrokenIndex = TRUE;
     
        return l_bResult;
    }//CAList::Put_After


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_Prev
    pAList_Cell Get_Prev(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell*>(i_pCell);
        pAList_Cell  l_pResult = NULL;
        
        if (l_pCell)
        {
            l_pResult = static_cast<pAList_Cell>(l_pCell->m_pPrev);
        }
        else
        {
            l_pResult = static_cast<pAList_Cell>(m_pLast);
        }

        return l_pResult;
    }//CAList::Get_Prev


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_Next
    pAList_Cell Get_Next(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell *>(i_pCell);
        pAList_Cell  l_pResult = NULL;
        
        if (l_pCell)
        {
            l_pResult = static_cast<pAList_Cell>(l_pCell->m_pNext);
        }
        else
        {
            l_pResult = static_cast<pAList_Cell>(m_pFirst);
        }

        return l_pResult;
    }//CAList::Get_Next

    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_Data
    tData_Type Get_Data(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell*>(i_pCell);
        tData_Type   l_pResult = NULL;
        if (l_pCell)
        {
            l_pResult = l_pCell->m_pData;
        }

        return l_pResult;
    }//CAList::Get_Data

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Set_Data
    tBOOL Set_Data(pAList_Cell i_pCell, tBOOL i_bFree_Old_Data, tData_Type i_pData)
    {
        tCell      *l_pCell   = static_cast<tCell*>(i_pCell);
        tData_Type  l_pResult = NULL;
        if (l_pCell)
        {
            if (i_bFree_Old_Data)
            {
                Data_Release(l_pCell->m_pData);
            }

            l_pCell->m_pData = i_pData;
        }

        return TRUE;
    }//CAList::Set_Data


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_ByIndex
    pAList_Cell Get_ByIndex(tUINT32 i_dwIDX)
    {
        tCell *l_pReturn   = NULL;
        tUINT32  l_dwI;

        if (m_bBrokenIndex)
        {
            Index_Build();
        }

        if  (i_dwIDX < m_dwCount) 
        {
            if (    (FALSE == m_bBrokenIndex)
                 && (m_pIndex) 
               )
            {
                l_pReturn = m_pIndex[i_dwIDX];
            }
            else
            {
                l_pReturn = m_pFirst;
                for (l_dwI = 0; l_dwI < i_dwIDX; l_dwI++)
                {
                    if (l_pReturn)
                    {
                        l_pReturn = l_pReturn->m_pNext;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        return l_pReturn;
    }//CAList::Get_ByIndex


    ////////////////////////////////////////////////////////////////////////////
    //CAList::operator[]
    tData_Type operator[](tUINT32 i_dwIDX)
    {
        tCell *l_pReturn = static_cast<tCell*>(Get_ByIndex(i_dwIDX));

        return (l_pReturn) ? l_pReturn->m_pData : NULL;
    }//CAList::operator[]


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Put_Data
    tBOOL Put_Data(pAList_Cell i_pCell, tData_Type i_pData, tBOOL i_bFree_Old_Data)
    { 
        tBOOL   l_bResult = TRUE;
        tCell *l_pCell   = static_cast<tCell*>(i_pCell);
        if (l_pCell)
        {
            if ( i_bFree_Old_Data )
            {
                Data_Release(l_pCell->m_pData);
                l_pCell->m_pData = NULL;
            }

            l_pCell->m_pData = i_pData;
        }
        else
        {
            l_bResult = FALSE;
        }


        return l_bResult;
    }//CAList::Put_Data

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Lock
    //void Lock()
    //{
    //    WaitForSingleObject(m_hLock_Mutex,  INFINITE);        
    //}//CAList::Lock


    ////////////////////////////////////////////////////////////////////////////
    //CAList::UnLock
    //void UnLock()
    //{
    //    ReleaseMutex(m_hLock_Mutex);
    //}//CAList::UnLock


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Clear
    void Clear(tBOOL i_bClearData)
    {
        while (NULL != m_pFirst)
        {
            if (TRUE != Cell_Release(m_pFirst, TRUE, i_bClearData))
            {
                break;
            }
        }
    }//CAList::Clear


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_First
    pAList_Cell Get_First()
    {
        return static_cast<pAList_Cell>(m_pFirst);
    }//CAList::Get_First


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Get_Last
    pAList_Cell Get_Last()
    {
        return static_cast<pAList_Cell>(m_pLast);
    }//CAList::Get_Last


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Count
    tUINT32 Count()
    {
        return m_dwCount;
    }//CAList::Count

private:
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Cell_Release
    tBOOL Cell_Release(tCell *i_pCell, tBOOL i_bFree_Cell, tBOOL i_bFree_Data)
    {
        tBOOL l_bResult = TRUE;

        if (NULL == i_pCell)       
        {
            return FALSE;
        }

        if ((NULL != i_pCell->m_pPrev) && (NULL != i_pCell->m_pNext)) 
        {
            i_pCell->m_pPrev->m_pNext = i_pCell->m_pNext;
            i_pCell->m_pNext->m_pPrev = i_pCell->m_pPrev;
        }
        else
        { 
            l_bResult = FALSE;

            if (i_pCell == m_pFirst)
            {
                m_pFirst = i_pCell->m_pNext;

                if (m_pFirst != 0)
                {
                    m_pFirst->m_pPrev = 0;
                }

                l_bResult = TRUE;  
            }

            if (i_pCell == m_pLast)
            {
                m_pLast = i_pCell->m_pPrev;
                if (m_pLast != 0) 
                {
                    m_pLast->m_pNext = 0; 
                }

                l_bResult = TRUE;  
            }
        }

        if (TRUE == l_bResult) 
        {
            if (i_bFree_Data)
            {
                Data_Release(i_pCell->m_pData);
                i_pCell->m_pData = NULL;
            }

            if (i_bFree_Cell)
            {
                Cell_Free(i_pCell);
                i_pCell = NULL;
            }
        }

        m_bBrokenIndex = TRUE;

        m_dwCount--;

        return l_bResult;
    }//CAList::Cell_Release


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Index_Build
    tBOOL Index_Build()
    {
        tBOOL l_bResult = TRUE;

        if ( (m_pIndex) && (m_dwCount > m_dwIndexCount) )
        {
            Index_Release();
        }

        if (NULL == m_pIndex)
        {
            m_dwIndexCount = m_dwCount + 128;
            m_pIndex = (pIndex_Cell*)this->MemAlloc(sizeof(pIndex_Cell) * m_dwIndexCount);
        }

        if (NULL == m_pIndex)
        {
            l_bResult = FALSE;
            m_dwIndexCount = 0;
        }

        if (TRUE == l_bResult)
        {
            pAList_Cell l_pEl = NULL;

            memset(m_pIndex, 0, m_dwIndexCount * sizeof(pIndex_Cell));
            
            pIndex_Cell *l_pIndexLocal = m_pIndex;

            while ((l_pEl = Get_Next(l_pEl)) != NULL)
            {
               *l_pIndexLocal = static_cast<tCell*>(l_pEl);
               l_pIndexLocal ++;
            }

            m_bBrokenIndex = FALSE;
        }

        return l_bResult;
    }//CAList::Index_Build


    ////////////////////////////////////////////////////////////////////////////
    //CAList::Index_Release
    tBOOL Index_Release()
    {
        if (m_pIndex)
        {
            this->MemFree(m_pIndex);
            m_pIndex = NULL;
        }

        m_dwIndexCount = 0;
        m_bBrokenIndex = TRUE;
        return TRUE;
    }//CAList::Index_Release

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Data_Release
    // override this function in derived class to implement specific data 
    // destructor(structures as example)
    virtual tBOOL Data_Release(tData_Type i_pData)
    {
        if (NULL == i_pData)
        {
            return FALSE;
        }

        delete i_pData;
        return TRUE;
    }//CAList::Data_Release

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::MemAlloc
    //override this function in derived class to implement own memory allocation
    //mechanism, it will be used everywhere in this class
    virtual void *MemAlloc(size_t i_szSize)
    {
        return new tUINT8[i_szSize];
    }//CAList::MemAlloc

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::MemFree
    //override this function in derived class to implement own memory deallocation
    //mechanism, it will be used everywhere in this class
    virtual void MemFree(void * i_pMemory)
    {
        delete [] ((tUINT8*)i_pMemory);
    }//CAList::MemFree

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Cell_Allocate
    virtual tCell *Cell_Allocate()
    {
        return (tCell*)this->MemAlloc(sizeof(tCell));
    }//Cell_Allocate

    
    ////////////////////////////////////////////////////////////////////////////
    //CAList::Cell_Free
    virtual void Cell_Free(tCell *i_pCell)
    {
        this->MemFree(i_pCell);
    }//Cell_Free
};


////////////////////////////////////////////////////////////////////////////////
//This list has pool for list's cells, this technology speedup allocation and 
//deallocation of new list's elements (6-10 times).
//List intended for situation when you has very intensive adding & deleting 
//process, but it will use a lot of memory if you will add 1 million items and
//then delete 999 thousands, after that all 999 thousands internal list cells
//will be stored in internal pool, for the time being mechanism of pool packing
//is not developed.
//N.B.: be careful with list, use it only when you absolutely sure in what are 
//      you doing.
template <typename tData_Type>
class CBList:
    public CAList<tData_Type>
{
private:
  
#define TCELL typename CAList<tData_Type>::tCell
    
    struct tPool_Segment
    {
        TCELL         *m_pCells;
        tUINT32        m_dwCount;
        tPool_Segment *m_pNext;
    };

    tPool_Segment  *m_pPool_First;
    TCELL          *m_pPool_Cell_First;

    //count of cell allocated for every pool segment
    tUINT32                             m_dwPool_Size; 
public:
    ////////////////////////////////////////////////////////////////////////////
    //CBList::CBList
    CBList(tUINT32 i_dwPool_Size = 128)
       : CAList<tData_Type>()
       , m_dwPool_Size(i_dwPool_Size)
    {
        m_pPool_First      = NULL;
        m_pPool_Cell_First = NULL;
    }//CBList::CBList


    ////////////////////////////////////////////////////////////////////////////
    //CBList::~CBList
    ~CBList()
    {
        tPool_Segment *l_pPool_Tmp = NULL;
        while (m_pPool_First)
        {
            l_pPool_Tmp = m_pPool_First;
            m_pPool_First = m_pPool_First->m_pNext;

            Free_Pool_Segment(l_pPool_Tmp);
        }
    }//CBList::~CBList

private:
    ////////////////////////////////////////////////////////////////////////////
    //CBList::Create_Pool_Segment
    tBOOL Create_Pool_Segment()
    {
        tBOOL l_bReturn = FALSE;

        tPool_Segment *l_pPool = (tPool_Segment*)CAList<tData_Type>::MemAlloc(sizeof(tPool_Segment));
        
        if (l_pPool)
        {
            memset(l_pPool, 0, sizeof(tPool_Segment));
            l_pPool->m_dwCount = m_dwPool_Size;
            l_pPool->m_pCells  = (TCELL*)this->MemAlloc(sizeof(TCELL) * l_pPool->m_dwCount);
            
            if (l_pPool->m_pCells)
            {
                TCELL *l_pCell = NULL;

                memset(l_pPool->m_pCells, 
                       0, 
                       sizeof(tPool_Segment) * l_pPool->m_dwCount
                      );

                l_pCell = l_pPool->m_pCells;

                for (tUINT32 l_dwIDX = 1; l_dwIDX < l_pPool->m_dwCount; l_dwIDX++)
                {
                    l_pCell->m_pNext = (l_pCell + 1);
                    l_pCell ++;
                }

                l_bReturn          = TRUE;
                l_pPool->m_pNext   = m_pPool_First;
                m_pPool_First      = l_pPool;
                l_pCell->m_pNext   = m_pPool_Cell_First;
                m_pPool_Cell_First = l_pPool->m_pCells;
            }
        }

        if (FALSE == l_bReturn)
        {
            Free_Pool_Segment(l_pPool);
        }

        return l_bReturn;
    }//CBList::Create_Pool_Segment


    ////////////////////////////////////////////////////////////////////////////
    //CBList::Free_Pool_Segment
    void Free_Pool_Segment(tPool_Segment *io_pPool)
    {
        if (io_pPool)
        {
            if (io_pPool->m_pCells)
            {
                this->MemFree(io_pPool->m_pCells);
                io_pPool->m_pCells = NULL;
            }
            this->MemFree(io_pPool);
        }
    }//CBList::Free_Pool_Segment

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CBList::Cell_Allocate
    virtual TCELL *Cell_Allocate()
    {
        TCELL *l_pReturn = NULL;
        if (NULL == m_pPool_Cell_First)
        {
            Create_Pool_Segment();
        }

        l_pReturn = m_pPool_Cell_First;

        if (m_pPool_Cell_First)
        {
            m_pPool_Cell_First = m_pPool_Cell_First->m_pNext;
        }

        return l_pReturn;
    }//CBList::Cell_Allocate

    
    ////////////////////////////////////////////////////////////////////////////
    //CBList::Cell_Free
    virtual void Cell_Free(TCELL *i_pCell)
    {
        memset(i_pCell, 0, sizeof(TCELL));
        i_pCell->m_pNext   = m_pPool_Cell_First;
        m_pPool_Cell_First = i_pCell;
    }//CBList::Cell_Free
};

#endif //ALIST_H_AZH
