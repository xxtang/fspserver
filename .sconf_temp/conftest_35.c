

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char random();

int main() {
#if defined (__stub_random) || defined (__stub___random)
  fail fail fail
#else
  random();
#endif

  return 0;
}
