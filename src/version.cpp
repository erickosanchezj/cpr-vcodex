#include "version.h"

#ifdef SIMULATOR
const char CPR_CROSSPOINT_VERSION[] = "dev-simulator";
const char CPR_VCODEX_BASE_VERSION[] = "1.5.0";
const int CPR_VCODEX_BUILD_SEQ = 0;
const int CPR_VCODEX_RELEASE_SEQ = 0;
const char CPR_VCODEX_BUILD_KIND[] = "simulator";
#else
#include "version.generated.inc"
#endif
