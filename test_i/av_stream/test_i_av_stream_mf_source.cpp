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

#include "strsafe.h"

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
#include "WinReg.h"
#else
#include "ace/Synch.h"

#include "common_tools.h"

#include "stream_lib_guids.h"

//WINADVAPI
LSTATUS
APIENTRY
RegDeleteTreeA (__in        HKEY     hKey,
                __in_opt    LPCSTR  lpSubKey)
{
  return (Common_Tools::deleteKey (hKey, lpSubKey) ? -1 : ERROR_SUCCESS);
};
//WINADVAPI
LSTATUS
APIENTRY
RegDeleteTreeW (__in        HKEY     hKey,
                __in_opt    LPCWSTR  lpSubKey)
{
  return RegDeleteTreeA (hKey, ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (lpSubKey)));
};
#ifdef UNICODE
#define RegDeleteTree  RegDeleteTreeW
#else
#define RegDeleteTree  RegDeleteTreeA
#endif // !UNICODE

//WINADVAPI
LSTATUS
APIENTRY
RegDeleteKeyValueA (__in     HKEY   hKey,
                    __in_opt LPCSTR lpSubKey,
                    __in_opt LPCSTR lpValueName)
{
  return (Common_Tools::deleteKeyValue (hKey, lpSubKey, lpValueName) ? -1 : ERROR_SUCCESS);
};
//WINADVAPI
LSTATUS
APIENTRY
RegDeleteKeyValueW (__in      HKEY     hKey,
                    __in_opt  LPCWSTR lpSubKey,
                    __in_opt  LPCWSTR lpValueName)
{
  return RegDeleteKeyValueA (hKey, ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (lpSubKey)), ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (lpValueName)));
};
#ifdef UNICODE
#define RegDeleteKeyValue  RegDeleteKeyValueW
#else
#define RegDeleteKeyValue  RegDeleteKeyValueA
#endif // !UNICODE
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

#include "ace/Log_Msg.h"
//#include "ace/Synch.h"

#include "class_factory.h"
#include "registry.h"       // Helpers to register COM objects.

#include "common_time_common.h"

#include "stream_macros.h"

#include "stream_lib_common.h"
#include "stream_lib_mediafoundation_mediasource.h"

//#include "MPEG1ByteStreamHandler.h"

#include "test_i_av_stream_server_common.h"

HMODULE g_hModule; // DLL module handle

// Defines the class factory lock variable:
DEFINE_CLASSFACTORY_SERVER_LOCK;

//#define IF_FAILED_GOTO (result, label) if (FAILED (result)) { goto label; }
//#define CHECK_HR (result) IF_FAILED_GOTO (result, done)

// -----------------------------------------------------------------------------

typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<Common_TimePolicy_t,
                                                            Test_I_AVStream_Server_MediaFoundation_Message,
                                                            struct Test_I_MediaFoundationConfiguration> Stream_MediaFramework_MediaFoundation_MediaSource_t;

// g_ClassFactories: Array of class factory data.
// Defines a look-up table of CLSIDs and corresponding creation functions.
ClassFactoryData g_ClassFactories[] =
{
  { &CLSID_ACEStream_MediaFramework_MF_MediaSource,
    Stream_MediaFramework_MediaFoundation_MediaSource_t::CreateInstance }
};
DWORD g_numClassFactories = ARRAYSIZE (g_ClassFactories);

// Text strings

// Description string for the bytestream handler.
//const TCHAR* sByteStreamHandlerDescription =
//  TEXT ("ACEStream Source ByteStreamHandler");
// File extension for WAVE files.
//const TCHAR* sFileExtension = TEXT(".mpg");
// Registry location for bytestream handlers.
//const TCHAR* REGKEY_MF_BYTESTREAM_HANDLERS =
//  TEXT ("Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers");

BOOL APIENTRY
DllMain (HANDLE module_in,
         DWORD  reason_in,
         LPVOID lpReserved_in)
{
  //STREAM_TRACE (ACE_TEXT ("::DllMain"));

  ACE_UNUSED_ARG (lpReserved_in);

  switch (reason_in)
  {
    case DLL_PROCESS_ATTACH:
      g_hModule = (HMODULE)module_in;
      //TRACE_INIT();
      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      //TRACE_CLOSE();
      break;
  } // end SWITCH

  return TRUE;
}

STDAPI
DllCanUnloadNow ()
{
  //STREAM_TRACE (ACE_TEXT ("::DllCanUnloadNow"));

  return (!ClassFactory::IsLocked () ? S_OK : S_FALSE);
}

STDAPI
DllRegisterServer ()
{
  //STREAM_TRACE (ACE_TEXT ("::DllRegisterServer"));

  HRESULT result = S_OK;

  // register the bytestream handler's CLSID
  result =
    RegisterObject (g_hModule,
                    CLSID_ACEStream_MediaFramework_MF_MediaSource,
#if defined (UNICODE)
                    ACE_TEXT_ALWAYS_WCHAR (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_DESCRIPTION),
                    ACE_TEXT_ALWAYS_WCHAR ("Both"));
#else
                    ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_DESCRIPTION),
                    ACE_TEXT_ALWAYS_CHAR ("Both"));
#endif // UNICODE
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to RegisterObject(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_ACEStream_MediaFramework_MF_MediaSource).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  //// register the bytestream handler as the handler for the MPEG-1 file extension
  //result =
  //  RegisterByteStreamHandler (CLSID_MFSampleMPEG1ByteStreamHandler,                                 // CLSID 
  //                             fileExtension_in,                                                     // Supported file extension
  //                             ACE_TEXT (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_DESCRIPTION)); // Description
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to RegisterByteStreamHandler(%s,\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::GUIDToString (CLSID_MFSampleMPEG1ByteStreamHandler).c_str ()),
  //              fileExtension_in,
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  return result;
  //} // end IF

  return result;
}
STDAPI
DllUnregisterServer ()
{
  //STREAM_TRACE (ACE_TEXT ("::DllUnregisterServer"));

  // unregister the CLSIDs
  HRESULT result = UnregisterObject (CLSID_ACEStream_MediaFramework_MF_MediaSource);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to UnregisterObject(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_ACEStream_MediaFramework_MF_MediaSource).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  //// unregister the bytestream handler for the file extension
  //UnregisterByteStreamHandler (CLSID_ACEStream_MediaFramework_MF_MediaSource, fileExtension_in);

  return result;
}

STDAPI
DllGetClassObject (REFCLSID CLSID_in,
                   REFIID IID_in,
                   void** handle_out)
{
  //STREAM_TRACE (ACE_TEXT ("::DllGetClassObject"));

  HRESULT result = CLASS_E_CLASSNOTAVAILABLE;

  // find an entry in the static look-up table
  ClassFactory* class_factory_p = NULL;
  for (DWORD index = 0; index < g_numClassFactories; index++)
    if (InlineIsEqualGUID (*g_ClassFactories[index].pclsid, CLSID_in))
    {
      // Found an entry. Create a new class factory object.
      ACE_NEW_NORETURN (class_factory_p,
                        ClassFactory (g_ClassFactories[index].pfnCreate));
      result = (class_factory_p ? S_OK : E_OUTOFMEMORY);
      break;
    } // end IF
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ClassFactory() (CLSID was: %s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (class_factory_p);
  result = class_factory_p->QueryInterface (IID_in, handle_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ClassFactory::QueryInterface() (CLSID was: %s, IID was: %s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_in).c_str ()),
                ACE_TEXT (Common_OS_Tools::GUIDToString (IID_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    class_factory_p->Release (); class_factory_p = NULL;
    return result;
  } // end IF
  class_factory_p->Release (); class_factory_p = NULL;

  return result;
}

///////////////////////////////////////////////////////////////////////
// Name: CreateRegistryKey
// Desc: Creates a new registry key. (Thin wrapper just to encapsulate
//       all of the default options.)
///////////////////////////////////////////////////////////////////////
HRESULT
CreateRegistryKey (HKEY parentKey_in,
                   LPCTSTR key_in,
                   HKEY* key_out)
{
  //STREAM_TRACE (ACE_TEXT ("::CreateRegistryKey"));

  HRESULT result = E_FAIL;

  // sanity check(s)
  ACE_ASSERT (key_out);

  LONG result_2 =
    RegCreateKeyEx (parentKey_in,         // parent key
                    key_in,               // name of subkey
                    0,                    // reserved
                    NULL,                 // class string (can be NULL)
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,                 // security attributes
                    key_out,
                    NULL);                // receives the "disposition" (is it a new or existing key)
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    result = HRESULT_FROM_WIN32 (result_2);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to RegDeleteKeyValue(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  return result;
}

///////////////////////////////////////////////////////////////////////
// Name: RegisterByteStreamHandler
// Desc: Register a bytestream handler for the Media Foundation
//       source resolver.
//
// guid:            CLSID of the bytestream handler.
// sFileExtension:  File extension.
// sDescription:    Description.
//
// Note: sFileExtension can also be a MIME type although that is not
//       illustrated in this sample.
///////////////////////////////////////////////////////////////////////
HRESULT
RegisterByteStreamHandler (REFCLSID CLSID_in,
                           const TCHAR* fileExtension_in,
                           const TCHAR* description_in)
{
  //STREAM_TRACE (ACE_TEXT ("::RegisterByteStreamHandler"));

  // Open HKCU/<byte stream handlers>/<file extension>
  // Create {clsid} = <description> key

  HRESULT result = S_OK;
  int result_2 = -1;
  HKEY key_p = NULL;
  HKEY subkey_p = NULL;
  std::string clsid_string = Common_OS_Tools::GUIDToString (CLSID_in);
  size_t length_i = 0;
  result = StringCchLength (description_in,
                            STRSAFE_MAX_CCH,
                            &length_i);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to StringCchLength(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  result =
      CreateRegistryKey (HKEY_LOCAL_MACHINE,
#if defined (UNICODE)
                         ACE_TEXT_ALWAYS_WCHAR (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY),
#else
                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY),
#endif // UNICODE
                         &key_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateRegistryKey(%s): \"%s\", aborting\n"),
                ACE_TEXT (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (key_p);
  result = CreateRegistryKey (key_p,
                              fileExtension_in,
                              &subkey_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateRegistryKey(%s): \"%s\", aborting\n"),
                fileExtension_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (subkey_p);
  result =
    RegSetValueEx (subkey_p,
#if defined (UNICODE)
                   ACE_TEXT_ALWAYS_WCHAR (clsid_string.c_str ()),
#else
                   ACE_TEXT_ALWAYS_CHAR (clsid_string.c_str ()),
#endif // UNICODE
                   0,
                   REG_SZ,
                   reinterpret_cast<const BYTE*> (description_in),
                   static_cast<DWORD> ((length_i + 1) * sizeof (TCHAR)));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to RegSetValueEx(%s): \"%s\", aborting\n"),
                fileExtension_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF

clean:
  if (subkey_p)
    RegCloseKey (subkey_p);
  if (key_p)
    RegCloseKey (key_p);

  return result;
}
HRESULT
UnregisterByteStreamHandler (REFCLSID CLSID_in,
                             const TCHAR* fileExtension_in)
{
  //STREAM_TRACE (ACE_TEXT ("::UnregisterByteStreamHandler"));

  HRESULT result = S_OK;
  TCHAR buffer_a[MAX_PATH];
  std::string clsid_string = Common_OS_Tools::GUIDToString (CLSID_in);
  LSTATUS result_2;

  // create the subkey name
  result =
    StringCchPrintf (buffer_a,
                     sizeof (TCHAR[MAX_PATH]),
#if defined (UNICODE)
                     ACE_TEXT_ALWAYS_WCHAR ("%s\\%s"),
#else
                     ACE_TEXT_ALWAYS_CHAR ("%s\\%s"),
#endif // UNICODE
                     ACE_TEXT (STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY),
                     fileExtension_in);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to StringCchPrintf(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  // delete the CLSID entry under the subkey
  // Note: There might be multiple entries for this file extension, so we
  //       should not delete the entire subkey, just the entry for this CLSID
  result_2 =
    RegDeleteKeyValue (HKEY_LOCAL_MACHINE,
                       buffer_a,
#if defined (UNICODE)
                       ACE_TEXT_ALWAYS_WCHAR (clsid_string.c_str ()));
#else
                       ACE_TEXT_ALWAYS_CHAR (clsid_string.c_str ()));
#endif // UNICODE
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    result = HRESULT_FROM_WIN32 (result_2);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to RegDeleteKeyValue(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  return S_OK;
}
