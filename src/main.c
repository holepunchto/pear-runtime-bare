#include <assert.h>
#include <bare.h>
#include <uv.h>
#include <stdlib.h>

#ifdef __linux__
#include <string.h>
#include <unistd.h>
#endif

#define PATH_MAX_STRING_SIZE 4096

#ifdef _WIN32
static const char SEP = '\\';
#define PATH_SEP_STRING "\\"
#else
static const char SEP = '/';
#define PATH_SEP_STRING "/"
#endif

static void
cd_dot_dot (char *p) {
  char *lst = p + strlen(p) - 1;
  while (lst != p && *lst != SEP) {
    lst--;
  }
  *lst = '\0';
}

static int
get_binary_path (char *exec_path) {
#ifdef __linux__
  ssize_t s = readlink("/proc/self/exe", exec_path, PATH_MAX_STRING_SIZE);
  if (s < 0) return (int) s;
  exec_path[s] = '\0';
  return 0;
#elif _WIN32
  DWORD result = GetModuleFileName(NULL, exec_path, PATH_MAX_STRING_SIZE);
  if (result == 0) return -1;
  return 0;
#else
  uint32_t exec_path_size = PATH_MAX_STRING_SIZE;
  char tmp[PATH_MAX_STRING_SIZE];
  int err = _NSGetExecutablePath(tmp, &exec_path_size);
  if (err < 0) return err;
  return realpath(tmp, exec_path) == NULL ? -1 : 0;
#endif
}

static int
file_exists (char *filename) {
#ifdef _WIN32
  WCHAR tmp[PATH_MAX_STRING_SIZE];
  mbstowcs(tmp, filename, PATH_MAX_STRING_SIZE);
  DWORD attr = GetFileAttributesW(tmp);
  return attr == INVALID_FILE_ATTRIBUTES ? 0 : 1;
#else
  struct stat buffer;
  return stat(filename, &buffer) == 0 ? 1 : 0;
#endif
}

int
main (int argc, char *argv[]) {
  int err;

  argv = uv_setup_args(argc, argv);

  js_platform_t *platform;
  err = js_create_platform(uv_default_loop(), NULL, &platform);
  assert(err == 0);

  bare_t *bare;
  err = bare_setup(uv_default_loop(), platform, NULL, argc, argv, NULL, &bare);
  assert(err == 0);

  char path[PATH_MAX_STRING_SIZE];
  err = get_binary_path(path);
  assert(err == 0);

  // binary is in <root>/by-arch/<host>/bin/<executable>
  // so we need 4 cd ..
  cd_dot_dot(path);
  cd_dot_dot(path);
  cd_dot_dot(path);
  cd_dot_dot(path);

  int len = strlen(path);

  path[len++] = SEP;
  path[len++] = 'b';
  path[len++] = 'o';
  path[len++] = 'o';
  path[len++] = 't';
  path[len++] = '.';

  int dot = len;

  path[len++] = 'b';
  path[len++] = 'u';
  path[len++] = 'n';
  path[len++] = 'd';
  path[len++] = 'l';
  path[len++] = 'e';
  path[len++] = '\0';

  if (!file_exists(path)) {
    path[dot++] = 'j';
    path[dot++] = 's';
    path[dot++] = '\0';
  }

  err = bare_run(bare, path, NULL);
  assert(err == 0);

  int exit_code;
  err = bare_teardown(bare, &exit_code);
  assert(err == 0);

  err = js_destroy_platform(platform);
  assert(err == 0);

  err = uv_run(uv_default_loop(), UV_RUN_ONCE);
  assert(err == 0);

  return exit_code;
}
