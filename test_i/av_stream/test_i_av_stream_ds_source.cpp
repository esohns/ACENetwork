/***************************************************************************
*   Copyright (C) 2009 by Erik Sohns   *
*   erik.sohns@web.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"

#define INITGUID // *NOTE*: this exports DEFINE_GUIDs
                 //         (see: stream_lib_common.h)
#include "Unknwn.h"

#undef NANOSECONDS
#include "streams.h"

#include "ace/Log_Msg.h"
#include "ace/Synch.h"

#include "common_time_common.h"
#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dev_directshow_tools.h"

#include "stream_lib_defines.h"
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#include "stream_lib_guids.h"
#include "stream_lib_tools.h"

//#include "class_factory.h"

#include "test_i_defines.h"
#include "test_i_av_stream_server_message.h"
#include "test_i_av_stream_server_session_message.h"

// Setup data
const struct REGPINTYPES sudMediaTypes[] =
{
  { &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 }
  , { &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 }
  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_Avi }
  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_NULL  }
  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_MJPG  }
  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2  }
};
//const struct REGPINTYPES sudPinTypes[] =
//{
//  { &MEDIATYPE_Video, &MEDIASUBTYPE_Avi }
//  /*{ &MEDIATYPE_Video, &MEDIASUBTYPE_NULL  }
//  ,*/ /*{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 }
//  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 }
//  , { &MEDIATYPE_Video, &MEDIASUBTYPE_MJPG  }
//  , { &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2  } */
//};

const struct REGFILTERPINS sudOutputPin =
{
  STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME, // name
  FALSE,                                        // rendered ?
  TRUE,                                         // output ?
  FALSE,                                        // can the filter create zero instances ?
  FALSE,                                        // does the filter create multiple instances ?
  &CLSID_NULL,                                  // connects to filter
  NULL,                                         // connectes to pin
  1,                                            // number of media types
  sudMediaTypes                                 // pointer to media types
};
//const struct REGFILTERPINS2 sudOutputPin2 = 
//{
//  REG_PINFLAG_B_OUTPUT,  // dwFlags  (also _B_ZERO, _B_MANY, _B_RENDERER)
//  1,                     // cInstances
//  1,                     // nMediaTypes
//  sudPinTypes,           // lpMediaType
//  0,                     // nMediums
//  NULL,                  // lpMedium
//  &PIN_CATEGORY_CAPTURE, // clsPinCategory
//};

const struct REGFILTER2 sudFilterReg =
{
  1,              // Registration format. (*NOTE*: 2 --> REGFILTERPINS2)
  MERIT_NORMAL,   // Merit.
  {
    1,            // Number of pins
    &sudOutputPin // Pointer to pin information.
  }
  //// *TODO*: cannot do this in C++ yet
  //{
  //  .cPins2 = 1,             // Number of pins
  //  .rgPins2 = &sudOutputPin2 // Pointer to pin information.
  //}
};
//const AdvFilterInfo sudACEStreamax =
//{
//  &CLSID_ACEStream_Source_Filter,          // Filter CLSID
//  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // String name
//  0,                                       // must be 0 for new structure
//  1,                                       // Number of pins
//  &sudOpPin,                               // Pin details
//  MERIT_DO_NOT_USE,                        // filter merit
//  &CLSID_VideoInputDeviceCategory,
//};
const struct REGFILTERPINS sudOutputPinAM =
{
  STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME, // name
  FALSE,                                        // rendered ?
  TRUE,                                         // output ?
  FALSE,                                        // can the filter create zero instances ?
  FALSE,                                        // does the filter create multiple instances ?
  &CLSID_NULL,                                  // connects to filter
  NULL,                                         // connects to pin
  1,                                            // number of media types
  sudMediaTypes                                 // pointer to media types
};

const struct _AMOVIESETUP_FILTER sudFilterRegAM =
{
  &CLSID_ACEStream_MediaFramework_Source_Filter, // filter CLSID
  STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L,    // filter name
  MERIT_NORMAL,                                  // merit
  1,                                             // number of pin types
  &sudOutputPinAM                                // pointer to pin information
};
const struct _AMOVIESETUP_FILTER sudFilterRegAM2 =
{
  &CLSID_ACEStream_MediaFramework_Asynch_Source_Filter, // filter CLSID
  STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L,    // filter name
  MERIT_NORMAL,                                         // merit
  1,                                                    // number of pin types
  &sudOutputPinAM                                       // pointer to pin information
};

// -----------------------------------------------------------------------------

// *TODO*: these type definitions are useless; this filter is monolythic
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_I_AVStream_Server_DirectShow_Message,
                                                         struct Test_I_AVStream_Server_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Stream_MediaFramework_DirectShow_Source_Filter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_I_AVStream_Server_DirectShow_Message,
                                                                struct Test_I_AVStream_Server_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Stream_MediaFramework_DirectShow_Asynch_Source_Filter_t;

void WINAPI InitRoutine (BOOL, const CLSID*);

CFactoryTemplate g_Templates[] = {
  { STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L                       // Name.
  , &CLSID_ACEStream_MediaFramework_Source_Filter                    // CLSID.
  , Stream_MediaFramework_DirectShow_Source_Filter_t::CreateInstance // Creation function.
  , InitRoutine                                                      // Initialization function.
  , &sudFilterRegAM },                                               // Pointer to filter information.

  { STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L                       // Name.
  , &CLSID_ACEStream_MediaFramework_Asynch_Source_Filter                    // CLSID.
  , Stream_MediaFramework_DirectShow_Asynch_Source_Filter_t::CreateInstance // Creation function.
  , InitRoutine                                                             // Initialization function.
  , &sudFilterRegAM2 }                                                      // Pointer to filter information.
};
int g_cTemplates = sizeof (g_Templates) / sizeof (g_Templates[0]);

// -----------------------------------------------------------------------------

// [*NOTE*: "Non-member deallocation functions shall not be declared in a
//          namespace scope other than the global namespace."]

// *NOTE*: "These other signatures are only called automatically by a new - 
//         expression when their object construction fails (e.g., if the
//         constructor of an object throws while being constructed by a new -
//         expression with nothrow, the matching operator delete function
//         accepting a nothrow argument is called)."

// *NOTE*: "A std::nothrow_t deallocation function exists, but you cannot call
//         it with a delete expression... The deallocation function is there for
//         completeness. ..."
void __CRTDECL
operator delete (void* pointer_p, const ACE_nothrow_t& noThrow_in)
{
  STREAM_TRACE (ACE_TEXT ("::delete"));

  ACE_UNUSED_ARG (noThrow_in);

  ::free (pointer_p);
}
void __CRTDECL
operator delete (void* pointer_p)
{
  STREAM_TRACE (ACE_TEXT ("::delete"));

  ::free (pointer_p);
}

//CUnknown*
//WINAPI CreateInstance (LPUNKNOWN interface_in,
//                       HRESULT* result_in)
//{
//  return NULL;
//}

void WINAPI
InitRoutine (BOOL isLoading_in,
             const CLSID* CLSID_in)
{
  STREAM_TRACE (ACE_TEXT ("::InitRoutine"));

  ACE_UNUSED_ARG (CLSID_in);

  // sanity check(s)
  if (!isLoading_in) // DLL being unloaded ?
    return; // nothing to do

  //// step1: initialize COM
  //HRESULT result = CoInitializeEx (NULL,
  //                                 (COINIT_MULTITHREADED |
  //                                  COINIT_DISABLE_OLE1DDE |
  //                                  COINIT_SPEED_OVER_MEMORY));
  //if (FAILED (result)) // RPC_E_CHANGED_MODE : 0x80010106L
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  // step2: initialize media framework
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
}

//STDAPI
//DllCanUnloadNow ()
//{
//  STREAM_TRACE (ACE_TEXT ("::DllCanUnloadNow"));
//
//  if (CClassFactory::IsLocked () ||
//      CBaseObject::ObjectsActive ())
//    return S_FALSE;
//
//  return S_OK;
//}

//STDAPI
//DllGetClassObject (REFCLSID rClsID_in,
//                   REFIID riid_in,
//                   LPVOID* factory_out)
//{
//  STREAM_TRACE (ACE_TEXT ("::DllGetClassObject"));
//
//  // sanity check(s)
//  if (!InlineIsEqualGUID (rClsID_in, CLSID_ACEStream_MediaFramework_Source_Filter) &&
//      !InlineIsEqualGUID (rClsID_in, CLSID_ACEStream_MediaFramework_Asynch_Source_Filter))
//    return CLASS_E_CLASSNOTAVAILABLE;
//  if (!InlineIsEqualGUID (riid_in, IID_IUnknown) &&
//      !InlineIsEqualGUID (riid_in, IID_IClassFactory))
//    return E_NOINTERFACE;
//  if (!factory_out)
//    return E_POINTER;
//
//  // initialize return value(s)
//  *factory_out = NULL;
//
//  const CFactoryTemplate* factory_template_p = NULL;
//  for (int i = 0; i < g_cTemplates; ++i)
//  {
//    factory_template_p = &g_Templates[i];
//    if (factory_template_p->IsClassID (rClsID_in))
//      break;
//  } // end FOR
//  if (!factory_template_p)
//    return CLASS_E_CLASSNOTAVAILABLE;
//
//  ACE_NEW_NORETURN (*factory_out,
//                    CClassFactory (factory_template_p));
//  if (!*factory_out)
//    return E_OUTOFMEMORY;
//
//  return NOERROR;
//}

//////////////////////////////////////////

STDAPI
DllRegisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllRegisterServer"));

  HRESULT result = E_FAIL;

  result = AMovieDllRegisterServer2 (TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AMovieDllRegisterServer2(TRUE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  // *TODO*: is this necessary at all ?
  IFilterMapper2* ifilter_mapper_p = NULL;
  result = CoCreateInstance (CLSID_FilterMapper2,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&ifilter_mapper_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterMapper2): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (ifilter_mapper_p);

  result =
    ifilter_mapper_p->RegisterFilter (CLSID_ACEStream_MediaFramework_Source_Filter, // filter CLSID
                                      STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L,   // filter name
                                      NULL,                                         // device moniker
                                      &CLSID_LegacyAmFilterCategory,                // filter category
                                      STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L,   // instance data
                                      &sudFilterReg);                               // pointer to filter information
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IFilterMapper2::RegisterFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: registered DirectShow source filter \"%s\"\n"),
              ACE_TEXT (ACEStream_PACKAGE_NAME),
              ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)));
  result =
    ifilter_mapper_p->RegisterFilter (CLSID_ACEStream_MediaFramework_Asynch_Source_Filter, // filter CLSID
                                      STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L,   // filter name
                                      NULL,                                                // device moniker
                                      &CLSID_LegacyAmFilterCategory,                       // filter category
                                      STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L,   // instance data
                                      &sudFilterReg);                                      // pointer to filter information
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IFilterMapper2::RegisterFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: registered DirectShow asynch source filter \"%s\"\n"),
              ACE_TEXT (ACEStream_PACKAGE_NAME),
              ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)));

clean:
  if (ifilter_mapper_p)
    ifilter_mapper_p->Release ();

  return result;
} // DllRegisterServer

STDAPI
DllUnregisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllUnregisterServer"));

  HRESULT result = E_FAIL;

  result = AMovieDllRegisterServer2 (FALSE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AMovieDllRegisterServer2(false): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  // *TODO*: is this necessary at all ?
  IFilterMapper2* ifilter_mapper_p = NULL;
  result = CoCreateInstance (CLSID_FilterMapper2,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&ifilter_mapper_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterMapper2): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (ifilter_mapper_p);

  result =
    ifilter_mapper_p->UnregisterFilter (&CLSID_VideoInputDeviceCategory,
                                        STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L,
                                        CLSID_ACEStream_MediaFramework_Source_Filter);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IFilterMapper2::UnregisterFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: deregistered DirectShow source filter \"%s\"\n"),
              ACE_TEXT (ACEStream_PACKAGE_NAME),
              ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)));
  result =
    ifilter_mapper_p->UnregisterFilter (&CLSID_VideoInputDeviceCategory,
                                        STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L,
                                        CLSID_ACEStream_MediaFramework_Asynch_Source_Filter);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IFilterMapper2::UnregisterFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: deregistered DirectShow asynch source filter \"%s\"\n"),
              ACE_TEXT (ACEStream_PACKAGE_NAME),
              ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)));

clean:
  if (ifilter_mapper_p)
    ifilter_mapper_p->Release ();

  return result;
} // DllUnregisterServer

//////////////////////////////////////////

extern "C"
BOOL WINAPI
DllEntryPoint (HINSTANCE, ULONG, LPVOID);
BOOL WINAPI
DllMain (HANDLE hModule,
         DWORD  dwReason,
         LPVOID lpReserved)
{
  STREAM_TRACE (ACE_TEXT ("::DllMain"));

  return DllEntryPoint (static_cast<HINSTANCE> (hModule), dwReason, lpReserved);
}
