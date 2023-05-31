// Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <wasi/api.h>

int printf(const char *, ...);

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) {
  if (offset < 0) {
    errno = EINVAL;
    return -1;
  }
  printf("wasixlibcDEBUG: before __wasi_fd_pread\n");
  __wasi_iovec_t iov = {.buf = buf, .buf_len = nbyte};
  __wasi_size_t bytes_read;
  __wasi_errno_t error;
  while (1) {
    error = __wasi_fd_pread(fildes, &iov, 1, offset, &bytes_read);
    if (error != 0) {
      if (error == EAGAIN) {
        usleep(10000);
        continue;
      }
      __wasi_fdstat_t fds;
      printf("wasixlibcDEBUG: after __wasi_fd_pread errno: %d\n", error);
      printf("wasixlibcDEBUG: before __wasi_fd_fdstat_get\n");
      if (error == ENOTCAPABLE && __wasi_fd_fdstat_get(fildes, &fds) == 0) {
        // Determine why we got ENOTCAPABLE.
        if ((fds.fs_rights_base & __WASI_RIGHTS_FD_READ) == 0)
          error = EBADF;
        else
          error = ESPIPE;
      }
      errno = error;
      return -1;
    }
    break;
  }
  printf("wasixlibcDEBUG: _wasi_fd_pread succeeded\n");
  return bytes_read;
}
