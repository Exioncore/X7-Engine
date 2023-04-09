#include "EventSink.h"
// System Includes
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace WMW {

EventSink::EventSink(std::function<void(IWbemClassObject*)> callback) {
  m_lRef = 0;
  this->callback = callback;
}

ULONG EventSink::AddRef() { return InterlockedIncrement(&m_lRef); }

ULONG EventSink::Release() {
  LONG lRef = InterlockedDecrement(&m_lRef);
  if (lRef == 0) delete this;
  return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv) {
  if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
    *ppv = (IWbemObjectSink*)this;
    AddRef();
    return WBEM_S_NO_ERROR;
  } else
    return E_NOINTERFACE;
}

HRESULT EventSink::Indicate(long lObjectCount, IWbemClassObject** apObjArray) {
  for (long i = 0; i < lObjectCount; i++) {
    callback(apObjArray[i]);
  }

  return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus(
    /* [in] */ LONG lFlags,
    /* [in] */ HRESULT hResult,
    /* [in] */ BSTR strParam,
    /* [in] */ IWbemClassObject __RPC_FAR* pObjParam) {
  return WBEM_S_NO_ERROR;
}

}  // namespace WMW