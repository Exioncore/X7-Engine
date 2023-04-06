#pragma once
// System Includes
// 3rd Party Include
// Interal Module Includes
#include <Logger.h>
// External Module Includes

#define LOGGER(module_name, enable, on_error_only)		inline static TRL::Logger logger = TRL::Logger(module_name, enable, on_error_only)

#define LOG_ERROR_TYPE		int32_t

#define LOG_OK		0
#define LOG_NOK		-1

#define LOG_INFO(text)	logger(__func__, text)

#define DBG
#ifdef DBG
	#define LOG_RETURN_TYPE						TRL::LogResult

	#define	LOG_BEGIN									TRL::LogResult ldat = {0,0}
	#define LOG_END										logger(ldat, 0, __func__, "")
	#define LOG_END_EC(error_code)		logger(error_code, ldat.id, 0, __func__, "")

	#define IS_LOG_OK									(ldat.error_code == TRL::Logger::OK)
	#define GET_LOG_ERROR_CODE				ldat.error_code;

	#define LOG_LR(log_result, text)	ldat = logger(log_result, ldat.id, __func__, text)
	#define LOG_EC(error_code, text)	ldat = logger(error_code, ldat.id, 0, __func__, text)
#else
	#define LOG_RETURN_TYPE		int32_t

	#define	LOG_BEGIN					int32_t ldat = 0
	#define LOG_END						ldat
	#define LOG_END_EC(error_code)		error_code

	#define IS_LOG_OK			(ldat == TRL::Logger::OK)
	#define GET_LOG_ERROR_CODE  ldat;

	#define LOG_LR(log_result, text)		ldat = log_result
	#define LOG_EC(error_code, text)		ldat = error_code
#endif
