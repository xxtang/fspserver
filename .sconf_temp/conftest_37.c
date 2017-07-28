

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char setsid();

int main() {
#if defined (__stub_setsid) || defined (__stub___setsid)
  fail fail fail
#else
  setsid();
#endif

  return 0;
}
