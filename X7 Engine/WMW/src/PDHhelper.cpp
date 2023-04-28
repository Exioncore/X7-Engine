#include "PDHhelper.h"
// System Includes
// 3rd Party Include
#include <PdhMsg.h>
// Interal Module Includes
// External Module Includes

namespace WMW {
///////////////
//// Methods //
///////////////
LOG_RETURN_TYPE PDHhelper::getInstancesInCategory(
    std::string category, std::vector<std::string>& instances) {
  LOG_BEGIN;

  // Determine size of buffers
  DWORD pcchCounterListLength = 0;
  DWORD pcchInstanceListLength = 0;
  PDH_STATUS r = PdhEnumObjectItems(
      NULL, NULL, category.c_str(), NULL, &pcchCounterListLength, NULL,
      &pcchInstanceListLength, PERF_DETAIL_ADVANCED, 0);

  if (r == PDH_MORE_DATA) {
    // Populate buffers
    PZZSTR mszCounterList =
        (PZZSTR)malloc(pcchCounterListLength * sizeof(PZZSTR));
    PZZSTR mszInstanceList =
        (PZZSTR)malloc(pcchInstanceListLength * sizeof(PZZSTR));
    LOG_EC(PdhEnumObjectItems(NULL, NULL, category.c_str(), mszCounterList,
                              &pcchCounterListLength, mszInstanceList,
                              &pcchInstanceListLength, PERF_DETAIL_ADVANCED, 0),
           "Enumerate objects in category");

    // Retrieve number of instances and their indeces
    if (IS_LOG_OK) {
      DWORD lastIndex = 0;
      for (DWORD i = 1; i < pcchInstanceListLength; i++) {
        if (mszInstanceList[i - 1] == NULL) {
          std::string name(mszInstanceList + lastIndex, i - lastIndex);
          instances.push_back(name);
          lastIndex = i;
        }
      }
    }
    // Release memory
    free(mszCounterList);
    free(mszInstanceList);
  }

  return LOG_END;
}

}  // namespace WMW