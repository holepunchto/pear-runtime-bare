#include <assert.h>
#include <bare.h>
#include <path.h>
#include <rlimit.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define RESTART_EXIT_CODE 75

static uv_sem_t pear__platform_ready;
static uv_async_t pear__platform_shutdown;
static js_platform_t *pear__platform;

static void
pear__on_platform_shutdown(uv_async_t *handle) {
  uv_close((uv_handle_t *) handle, NULL);
}

static void
pear__on_platform_thread(void *data) {
  int err;

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  assert(err == 0);

  err = uv_async_init(&loop, &pear__platform_shutdown, pear__on_platform_shutdown);
  assert(err == 0);

  err = js_create_platform(&loop, NULL, &pear__platform);
  assert(err == 0);

  uv_sem_post(&pear__platform_ready);

  err = uv_run(&loop, UV_RUN_DEFAULT);
  assert(err == 0);

  err = js_destroy_platform(pear__platform);
  assert(err == 0);

  err = uv_run(&loop, UV_RUN_DEFAULT);
  assert(err == 0);

  err = uv_loop_close(&loop);
  assert(err == 0);
}

int
pear_main(int argc, char *argv[]) {
  int err;

#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif

  err = rlimit_set(rlimit_open_files, rlimit_infer);
  assert(err == 0);

  uv_loop_t *loop = uv_default_loop();

  uv_disable_stdio_inheritance();

  argv = uv_setup_args(argc, argv);

  err = uv_sem_init(&pear__platform_ready, 0);
  assert(err == 0);

  uv_thread_t thread;
  err = uv_thread_create(&thread, pear__on_platform_thread, NULL);
  assert(err == 0);

  uv_sem_wait(&pear__platform_ready);

  uv_sem_destroy(&pear__platform_ready);

  int exit_code;

  do {
    bare_t *bare;
    err = bare_setup(loop, pear__platform, NULL, argc, (const char **) argv, NULL, &bare);
    assert(err == 0);

    char path[4096];
    size_t len = 4096;

    err = uv_exepath(path, &len);
    assert(err == 0);

    uv_fs_t req;
    err = uv_fs_realpath(loop, &req, path, NULL);
    assert(err == 0);

    strcpy(path, (char *) req.ptr);

    uv_fs_req_cleanup(&req);

    len = 4096;

    err = path_join(
      (const char *[]) {path, "..", "..", "..", "..", NULL},
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

    err = uv_fs_access(loop, &req, path, F_OK, NULL);

    uv_fs_req_cleanup(&req);

    if (err != 0) {
      path[dot++] = 'j';
      path[dot++] = 's';
      path[dot++] = '\0';
    }

    err = bare_load(bare, path, NULL, NULL);
    assert(err == 0);

    err = bare_run(bare, UV_RUN_DEFAULT);
    assert(err == 0);

    err = bare_teardown(bare, UV_RUN_DEFAULT, &exit_code);
    assert(err == 0);
  } while (exit_code == RESTART_EXIT_CODE);

  err = uv_loop_close(loop);
  assert(err == 0);

  err = uv_async_send(&pear__platform_shutdown);
  assert(err == 0);

  uv_thread_join(&thread);

  return exit_code;
}
