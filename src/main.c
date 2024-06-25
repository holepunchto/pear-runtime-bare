#include <assert.h>
#include <bare.h>
#include <path.h>
#include <string.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define RESTART_EXIT_CODE 75

int
main (int argc, char *argv[]) {
  int err;

  argv = uv_setup_args(argc, argv);

  js_platform_t *platform;
  err = js_create_platform(uv_default_loop(), NULL, &platform);
  assert(err == 0);

  int exit_code;

  do {
    bare_t *bare;
    err = bare_setup(uv_default_loop(), platform, NULL, argc, argv, NULL, &bare);
    assert(err == 0);

    char path[4096];
    size_t len = 4096;

    err = uv_exepath(path, &len);
    assert(err == 0);

    uv_fs_t req;
    err = uv_fs_realpath(uv_default_loop(), &req, path, NULL);
    assert(err == 0);

    strcpy(path, (char *) req.ptr);

    uv_fs_req_cleanup(&req);

    len = 4096;

    err = path_join(
      (const char *[]){path, "..", "..", "..", "..", NULL},
      path,
      &len,
      path_behavior_system
    );
    assert(err == 0);

    path[len++] = path_separator_system;
    path[len++] = 'b';
    path[len++] = 'o';
    path[len++] = 'o';
    path[len++] = 't';
    path[len++] = '.';

    size_t dot = len;

    path[len++] = 'b';
    path[len++] = 'u';
    path[len++] = 'n';
    path[len++] = 'd';
    path[len++] = 'l';
    path[len++] = 'e';
    path[len++] = '\0';

    err = uv_fs_access(uv_default_loop(), &req, path, F_OK, NULL);

    uv_fs_req_cleanup(&req);

    if (err != 0) {
      path[dot++] = 'j';
      path[dot++] = 's';
      path[dot++] = '\0';
    }

    err = bare_load(bare, path, NULL, NULL);
    assert(err == 0);

    err = bare_run(bare);
    assert(err == 0);

    err = bare_teardown(bare, &exit_code);
    assert(err == 0);
  } while (exit_code == RESTART_EXIT_CODE);

  err = js_destroy_platform(platform);
  assert(err == 0);

  err = uv_run(uv_default_loop(), UV_RUN_ONCE);
  assert(err == 0);

  return exit_code;
}
