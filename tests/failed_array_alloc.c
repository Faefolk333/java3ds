#include <stddef.h>
#include <stdlib.h>
void*__real_calloc(size_t,size_t);
// Linked only into the dedicated allocation-failure regression executable.
void*__wrap_calloc(size_t n,size_t s){size_t bytes=n*s;if(getenv("JAVA_TEST_FAIL_19200")&&bytes>76800&&bytes<77000)return NULL;return __real_calloc(n,s);}
