#pragma once
// System Includes
#include <functional>
// 3rd Party Include
#define _WIN32_DCOM
#include <Wbemidl.h>
#include <atlcomcli.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
// Interal Module Includes
// External Module

namespace WMW {
class EventSink : public IWbemObjectSink {
 public:
  CComPtr<IWbemObjectSink> pStubSink;

  EventSink(std::function<void(IWbemClassObject*)> callback);

  virtual ULONG STDMETHODCALLTYPE AddRef();

  virtual ULONG STDMETHODCALLTYPE Release();

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

  virtual HRESULT STDMETHODCALLTYPE Indicate(
      LONG lObjectCount, IWbemClassObject __RPC_FAR* __RPC_FAR* apObjArray);

  virtual HRESULT STDMETHODCALLTYPE SetStatus(
      /* [in] */ LONG lFlags,
      /* [in] */ HRESULT hResult,
      /* [in] */ BSTR strParam,
      /* [in] */ IWbemClassObject __RPC_FAR* pObjParam);

 private:
  LONG m_lRef;
  std::function<void(IWbemClassObject*)> callback;
};

}  // namespace WMW