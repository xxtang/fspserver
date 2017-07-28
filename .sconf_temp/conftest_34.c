

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char srandomdev();

int main() {
#if defined (__stub_srandomdev) || defined (__stub___srandomdev)
  fail fail fail
#else
  srandomdev();
#endif

  return 0;
}
