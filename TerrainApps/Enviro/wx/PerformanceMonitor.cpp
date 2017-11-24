/////////////////////////////////////////////////////////////////////////////
// Name:        PerformanceMonitor.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "PerformanceMonitor.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "PerformanceMonitor.h"

#if NVPERFSDK_FOUND
#include "NVPerfSDK.h"
#endif

#include "vtlib/vtlib.h"
class vtStructInstance;
#include "EnviroFrame.h"
#include "wx/valgen.h"

#if NVPERFSDK_FOUND
typedef struct _CounterInfo
{
    UINT CounterIndex;
    NVPMCOUNTERTYPE CounterType;
    NVPMCOUNTERVALUE CounterValue;
    NVPMCOUNTERDISPLAYHINT CounterDisplayHint;
    UINT64 CounterMax;
} CounterInfo;
#endif

// WDR: class implementations

//----------------------------------------------------------------------------
// CPerformanceMonitorDialog
//----------------------------------------------------------------------------

bool CPerformanceMonitorDialog::m_NVPMInitialised = false;



CPerformanceMonitorDialog::CPerformanceMonitorDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    PerformanceMonitorDlgBase( parent, id, title, position, size, style )
{
#if NVPERFSDK_FOUND

    UINT NumCounters;

    // WDR: dialog function PerformanceMonitorDialogFunc for CPerformanceMonitorDialog
    PerformanceMonitorDialogFunc( this, TRUE );

    wxListCtrl *pList = GetPmListctrl();

    pList->InsertColumn(0, _T("Value"));
    pList->InsertColumn(1, _T("Status"));
    pList->InsertColumn(2, _T("Description"));
    pList->SetColumnWidth(2, wxLIST_AUTOSIZE);

    if (m_NVPMInitialised)
    {
        if (NVPM_OK == NVPMGetNumCounters(&NumCounters))
        {
            UINT CounterIndex;
            UINT StringBufferLength = 1023;
            char StringBuffer[1024];
            UINT64 AttributeValue;
            for (CounterIndex = 0; CounterIndex < NumCounters; CounterIndex++)
            {
                StringBufferLength = 1023;
                if (NVPM_OK == NVPMGetCounterAttribute(CounterIndex, NVPMA_COUNTER_TYPE, &AttributeValue))
                {
                    if ((NVPM_CT_GPU == AttributeValue) || (NVPM_CT_OGL == AttributeValue))
                    {
                        if (NVPM_OK == NVPMGetCounterDescription(CounterIndex, StringBuffer, &StringBufferLength))
                        {
                            // Put this counter in the List
                            // Cosmetic memory leak here !!
                            CounterInfo *pInfo = new CounterInfo;
                            pInfo->CounterType = (NVPMCOUNTERTYPE)AttributeValue;
                            pInfo->CounterIndex = CounterIndex;
                            NVPMGetCounterAttribute(CounterIndex, NVPMA_COUNTER_VALUE, (UINT64*)&pInfo->CounterValue);
                            NVPMGetCounterAttribute(CounterIndex, NVPMA_COUNTER_DISPLAY_HINT, (UINT64*)&pInfo->CounterDisplayHint);
                            NVPMGetCounterAttribute(CounterIndex, NVPMA_COUNTER_MAX, &pInfo->CounterMax);
                            wxString Description(StringBuffer, wxConvUTF8);
                            wxListItem ListItem;
                            ListItem.SetMask(wxLIST_MASK_TEXT);
                            ListItem.SetId(pList->GetItemCount());
                            ListItem.SetColumn(2);
                            ListItem.SetText(Description);
                            pList->InsertItem(ListItem);
                            pList->SetItemPtrData(ListItem.GetId(), (wxUIntPtr)pInfo);
                            ListItem.SetColumn(1);
                            ListItem.SetText(wxString(_T("Off")));
                            pList->SetItem(ListItem);
                        }
                    }
                }
            }
            if (NumCounters > 0)
                pList->SetColumnWidth(2, wxLIST_AUTOSIZE);
        }
    }

#endif
}

void CPerformanceMonitorDialog::NVPM_init()
{
#if NVPERFSDK_FOUND

    if (NVPM_OK != NVPMInit())
        return;
    m_NVPMInitialised = true;

#endif
}

void CPerformanceMonitorDialog::NVPM_shutdown()
{
#if NVPERFSDK_FOUND

    m_NVPMInitialised = false;
    NVPMShutdown();

#endif
}

void CPerformanceMonitorDialog::NVPM_frame()
{
#if NVPERFSDK_FOUND

    if (m_NVPMInitialised)
    {
        CPerformanceMonitorDialog *pPM = GetFrame()->m_pPerformanceMonitorDlg;

        if (NULL != pPM)
        {
            pPM->UpdateCounters();
        }
    }

#endif
}

void CPerformanceMonitorDialog::UpdateCounters()
{
#if NVPERFSDK_FOUND

    NVPMRESULT Result = NVPM_OK;
    if (m_NVPMInitialised)
    {
        wxListCtrl *pList = GetPmListctrl();
        UINT Count = pList->GetItemCount();
#ifdef _MSC_VER
		// Stupid broken compiler
		NVPMSampleValue *Values = new NVPMSampleValue[Count];
#else
        NVPMSampleValue Values[Count];
#endif
        if (NVPM_OK == (Result = NVPMSample(Values, &Count)))
        {
            UINT Index;

            for (Index = 0; Index < Count; Index++)
            {
                long ListIndex;
                long ItemCount = pList->GetItemCount();
                // This could probably be made quicker with a wrap search
                CounterInfo *pInfo;
                UINT TargetCounterIndex = Values[Index].unCounterIndex;
                for (ListIndex = 0; ListIndex < ItemCount; ListIndex++)
                {
                    pInfo = (CounterInfo*)pList->GetItemData(ListIndex);
                    CounterInfo TestInfo = *pInfo;
                    if (pInfo->CounterIndex == TargetCounterIndex)
                        break;
                }
                if (ListIndex < ItemCount)
                {
                    wxListItem ListItem;
                    wxString Value;
                    switch(pInfo->CounterValue)
                    {
                        case NVPM_CV_PERCENT:
                            switch(pInfo->CounterDisplayHint)
                            {
                                case NVPM_CDH_PERCENT:
                                    Value << Values[Index].ulValue << _T("%");
                                    break;
                                case NVPM_CDH_RAW:
                                    Value << Values[Index].ulValue << _T("%");
                                    break;
                            }
                            break;
                        case NVPM_CV_RAW:
                            switch(pInfo->CounterDisplayHint)
                            {
                                case NVPM_CDH_PERCENT:
                                    if (0 == Values[Index].ulCycles)
                                        Values[Index].ulCycles = 1;
                                    Value << Values[Index].ulValue * 100 / Values[Index].ulCycles << _T("%");
                                    break;
                                case NVPM_CDH_RAW:
                                    Value << Values[Index].ulValue;
                                    break;
                            }
                            break;
                    }

                    ListItem.SetMask(wxLIST_MASK_TEXT);
                    ListItem.SetId(ListIndex);
                    ListItem.SetColumn(0);
                    ListItem.SetText(Value);
                    pList->SetItem(ListItem);
                }
            }
        }
#ifdef _MSC_VER
		// Stupid broken compiler
		delete Values;
#endif
    }

#endif
}

// WDR: handler implementations for CPerformanceMonitorDialog

void CPerformanceMonitorDialog::OnListItemRightClick( wxListEvent &event )
{
#if NVPERFSDK_FOUND

    NVPMRESULT Result = NVPM_OK;
    if (m_NVPMInitialised)
    {
        wxListCtrl *pList = GetPmListctrl();
        wxListItem ListItem;
        UINT CounterIndex = ((CounterInfo*)pList->GetItemData(event.GetIndex()))->CounterIndex;

        ListItem.SetId(event.GetIndex());
        ListItem.SetColumn(1);
        ListItem.SetMask(wxLIST_MASK_TEXT);
        pList->GetItem(ListItem);
        if (ListItem.GetText().IsSameAs(_T("On")))
        {
            ListItem.SetText(_T("Off"));
            pList->SetItem(ListItem);
            ListItem.SetColumn(0);
            ListItem.SetText(_T(""));
            pList->SetItem(ListItem);
            Result = NVPMRemoveCounter(CounterIndex);
        }
        else
        {

            ListItem.SetText(_T("On"));
            pList->SetItem(ListItem);
            Result = NVPMAddCounter(CounterIndex);
        }
    }

#endif
}
