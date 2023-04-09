#include "WMIwatcher.h"
// System Includes
#include <string>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace WMW {
//////////////////////////
// WMIwatcher Class //
//////////////////////////
WMIwatcher::WMIwatcher() : initialized(false), pLoc(NULL), pSvc(NULL), id(0) {}

///////////////
//// Methods //
///////////////
LOG_RETURN_TYPE WMIwatcher::initialize() {
  LOG_BEGIN;

  // Obtain the initial locator WMI
  LOG_EC(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID*)&pLoc),
         "CoCreateInstance");
  if (IS_LOG_OK) {
    // Connect to WMI through the IWbemLocation::ConnectServer method
    LOG_EC(pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0,
                               0, &pSvc),
           "IWbemLocation::ConnectServer");

    if (IS_LOG_OK) {
      // Set security levels on the proxy
      LOG_EC(
          CoSetProxyBlanket(pSvc,               // Indicates the proxy to set
                            RPC_C_AUTHN_WINNT,  // RPC_C_AUTHN_xxx
                            RPC_C_AUTHZ_NONE,   // RPC_C_AUTHZ_xxx
                            NULL,               // Server principal name
                            RPC_C_AUTHN_LEVEL_CALL,  // RPC_C_AUTHN_LEVEL_xxx
                            RPC_C_IMP_LEVEL_IMPERSONATE,  // RPC_C_IMP_LEVEL_xxx
                            NULL,                         // client identity
                            EOAC_NONE                     // proxy capabilities
                            ),
          "CoSetProxyBlanket");

      if (!IS_LOG_OK) {
        pSvc->Release();
      }
    }

    if (!IS_LOG_OK) {
      pLoc->Release();
    }
  }

  initialized = IS_LOG_OK;

  return LOG_END;
}

LOG_RETURN_TYPE WMIwatcher::deInitialize() {
  LOG_BEGIN;

  if (initialized) {
    for (auto& [id, event_sink] : registered_callbacks) {
      (void)pSvc->CancelAsyncCall(event_sink->pStubSink);
      event_sink->Release();
    }
    pSvc->Release();
    pLoc->Release();
    initialized = false;
  }

  return LOG_END;
}

LOG_RETURN_TYPE WMIwatcher::registerCallback(
    uint16_t& id, std::string query,
    std::function<void(IWbemClassObject*)> callback) {
  LOG_BEGIN;
  std::unique_ptr<EventSink> event_sink = std::make_unique<EventSink>(callback);

  CComPtr<EventSink> pSink(event_sink.get());

  // Use an unsecured apartment for security
  CComPtr<IUnsecuredApartment> pUnsecApp;

  LOG_EC(CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER,
                          IID_IUnsecuredApartment, (void**)&pUnsecApp),
         "CoCreateInstance");

  if (IS_LOG_OK) {
    CComPtr<IUnknown> pStubUnk;
    pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

    pStubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&pSink->pStubSink);

    LOG_EC(pSvc->ExecNotificationQueryAsync(
               _bstr_t("WQL"), _bstr_t(query.c_str()), WBEM_FLAG_SEND_STATUS,
               NULL, pSink->pStubSink),
           "ExecNotificationQueryAsync");

    if (IS_LOG_OK) {
      id = this->id;
      registered_callbacks.emplace(id, std::move(event_sink));
      this->id += 1;
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE WMIwatcher::deRegisterCallback(uint16_t id) {
  LOG_BEGIN;

  if (registered_callbacks.count(id) == 1) {
    (void) pSvc->CancelAsyncCall(registered_callbacks.at(id)->pStubSink);
    registered_callbacks.at(id)->Release();
    registered_callbacks.erase(id);
  } else {
    LOG_EC(1, "Callback with id " + std::to_string(id) +
                  " cannot be de-registered as it does not exist");
  }

  return LOG_END;
}

WMIwatcher& WMIwatcher::getInstance() {
  static WMIwatcher instance;
  return instance;
}

}  // namespace WMW
