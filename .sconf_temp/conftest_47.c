

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char fseeko();

int main() {
#if defined (__stub_fseeko) || defined (__stub___fseeko)
  fail fail fail
#else
  fseeko();
#endif

  return 0;
}
