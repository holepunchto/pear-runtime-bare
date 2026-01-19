#include <assert.h>
#include <dynload.h>
#include <path.h>
#include <stddef.h>
#include <stdio.h>
#include <uv.h>

typedef int (*pear_main_t)(int, char *[]);

static inline int
pear__resolve(pear_main_t *result) {
  int err;

  size_t len;

  char exe[4096];
  len = sizeof(exe);
  err = uv_exepath(exe, &len);
  assert(err == 0);

  char prefix[4096];
  len = sizeof(prefix);
  err = path_join((const char *[]) {exe, "..", "..", "lib", NULL}, prefix, &len, path_behavior_system);
  assert(err == 0);

  char dl[4096];
  len = sizeof(dl);
  err = dynload_resolve("pear-runtime", prefix, dl, &len);
  if (err < 0) return err;

  uv_lib_t lib;
  err = uv_dlopen(dl, &lib);
  if (err < 0) return err;

  pear_main_t pear_main;
  err = uv_dlsym(&lib, "pear_main", (void *) &pear_main);
  if (err < 0) return err;

  *result = pear_main;

  return 0;
}

int
main(int argc, char *argv[]) {
  int err;

  pear_main_t pear_main;
  err = pear__resolve(&pear_main);
  if (err < 0) return 1;

  return pear_main(argc, argv);
}
