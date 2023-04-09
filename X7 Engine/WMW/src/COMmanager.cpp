#include "COMmanager.h"
// System Includes
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
// External Module Includes

namespace WMW {
	//////////////////////////
	// WMIwatcher Class //
	//////////////////////////
	COMmanager::COMmanager() : initialized(false) {
	}

	COMmanager::~COMmanager() {
		deInitialize();
	}

	///////////////
	//// Methods //
	///////////////
	LOG_RETURN_TYPE COMmanager::initialize() {
		LOG_BEGIN;

		if (!initialized) {
			// Initialize COM
			LOG_EC(CoInitializeEx(NULL, COINIT_MULTITHREADED), "CoInitializeEx");
			// Set COM security levels
			if (IS_LOG_OK) {
				LOG_EC(CoInitializeSecurity(
					NULL,
					-1,                          // COM negotiates service
					NULL,                        // Authentication services
					NULL,                        // Reserved
					RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
					RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
					NULL,                        // Authentication info
					EOAC_NONE,                   // Additional capabilities 
					NULL                         // Reserved
				), "CoInitializeSecurity");
			}
			else {
				CoUninitialize();
			}
			initialized = IS_LOG_OK;
		}

		return LOG_END;
	}

	LOG_RETURN_TYPE COMmanager::deInitialize() {
		LOG_BEGIN;

		if (initialized) {
			CoUninitialize();
			initialized = false;
		}

		return LOG_END;
	}

}  // namespace WMW