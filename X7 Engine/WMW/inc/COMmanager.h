#pragma once
// System Includes
// 3rd Party Include
// Interal Module Includes
// External Module
#include "EasyLogger.h"

namespace WMW {
	class COMmanager {
	public:
		COMmanager();
		~COMmanager();

		// Methods
		LOG_RETURN_TYPE initialize();
		LOG_RETURN_TYPE deInitialize();

	private:
		LOGGER("COMmanager", true, true);

		bool initialized;

	};

}  // namespace WMW