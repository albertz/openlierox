// Copyright (c) 2007-2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ---
// Author: Markus Gutschke

// Include linux_syscall_support.h as the first file, so we will compilation
// errors if it has unexpected dependencies on other header files.
#include "linux_syscall_support.h"

#include <stdio.h>

// Used to count the number of check failures
static int check_failures = 0;

#define CHECK(cond)  do {                       \
   if (!(cond)) {                               \
     puts("Check failed: " #cond);              \
     ++check_failures;                          \
   }                                            \
} while (0)


// We need to do some "namespace" magic to be able to include both <asm/stat.h>
// and <sys/stat.h>, which normally are mutually exclusive.
// This is currently the only reason why this test has to be compiled as
// C++ code.
namespace linux_syscall_support {
#include <asm/stat.h>
}
#include <sys/stat.h>

namespace linux_syscall_support {
// Define kernel data structures as known to glibc
#include <asm/poll.h>
#include <asm/posix_types.h>
#include <asm/types.h>
#include <errno.h>
#include <linux/dirent.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>

// Set by the signal handler to show that we received a signal
static int signaled;

static void CheckStructures() {
  puts("CheckStructures...");
  // Compare sizes of the kernel structures. This will allow us to
  // catch cases where linux_syscall_support.h defined structures that
  // are obviously different from the ones the kernel expects. This is
  // a little complicated, because glibc often deliberately defines
  // incompatible versions. We address this issue on a case-by-case
  // basis by including the appropriate linux-specific header files
  // within our own namespace, instead of within the global
  // namespace. Occasionally, this requires careful sorting of header
  // files, too (e.g. in the case of "stat.h"). And we provide cleanup
  // where necessary (e.g. in the case of "struct statfs").  This is
  // far from perfect, but in the worst case, it'll lead to false
  // error messages that need to be fixed manually.  Unfortunately,
  // there are a small number of data structures (e.g "struct
  // kernel_old_sigaction") that we cannot test at all, as glibc does
  // not have any definitions for them.
  CHECK(sizeof(struct dirent64)  == sizeof(struct kernel_dirent64));
  CHECK(sizeof(struct dirent)    == sizeof(struct kernel_dirent));
  CHECK(sizeof(struct iovec)     == sizeof(struct kernel_iovec));
  CHECK(sizeof(struct msghdr)    == sizeof(struct kernel_msghdr));
  CHECK(sizeof(struct pollfd)    == sizeof(struct kernel_pollfd));
  CHECK(sizeof(struct rlimit)    == sizeof(struct kernel_rlimit));
  CHECK(sizeof(struct rusage)    == sizeof(struct kernel_rusage));
  CHECK(sizeof(struct sigaction) == sizeof(struct kernel_sigaction)
  // glibc defines an excessively large sigset_t. Compensate for it:
         + sizeof(((struct sigaction *)0)->sa_mask) - KERNEL_NSIG/8
  #ifdef __mips__
         + 2*sizeof(int)
  #endif
  );
  CHECK(sizeof(struct sockaddr)  == sizeof(struct kernel_sockaddr));
  CHECK(sizeof(struct stat)      == sizeof(struct kernel_stat));
  CHECK(sizeof(struct statfs)    == sizeof(struct kernel_statfs)
#ifdef __USE_FILE_OFFSET64
  // glibc sometimes defines 64-bit wide fields in "struct statfs"
  // even though this is just the 32-bit version of the structure.
  + 5*(sizeof(((struct statfs *)0)->f_blocks) - sizeof(unsigned))
#endif
  );
  CHECK(sizeof(struct timespec)  == sizeof(struct kernel_timespec));
  #ifndef __x86_64__
  CHECK(sizeof(struct stat64)    == sizeof(struct kernel_stat64));
  CHECK(sizeof(struct statfs64)  == sizeof(struct kernel_statfs64));
  #endif
}

#ifdef __mips__
#define ZERO_SIGACT { 0 }
#else
#define ZERO_SIGACT { { 0 } }
#endif

static void SigHandler(int signum) {
  if (signaled) {
    // Caller will report an error, as we cannot do so from a signal handler
    signaled = -1;
  } else {
    signaled = signum;
  }
  return;
}

static void SigAction(int signum, struct siginfo *si, void *arg) {
  SigHandler(signum);
}

static void Sigaction() {
  puts("Sigaction...");
  int signum       = SIGPWR;
  for (int info = 0; info < 2; info++) {
    signaled         = 0;
    struct kernel_sigaction sa = ZERO_SIGACT, old, orig;
    CHECK(!sys_sigaction(signum, NULL, &orig));
    if (info) {
      sa.sa_sigaction_ = SigAction;
    } else {
      sa.sa_handler_   = SigHandler;
    }
    sa.sa_flags      = SA_RESETHAND | SA_RESTART | (info ? SA_SIGINFO : 0);
    CHECK(!sys_sigemptyset(&sa.sa_mask));
    CHECK(!sys_sigaction(signum, &sa, &old));
    CHECK(!memcmp(&old, &orig, sizeof(struct kernel_sigaction)));
    CHECK(!sys_sigaction(signum, NULL, &old));
    #if defined(__i386__) || defined(__x86_64__)
    old.sa_restorer  = sa.sa_restorer;
    old.sa_flags    &= ~SA_RESTORER;
    #endif
    CHECK(!memcmp(&old, &sa, sizeof(struct kernel_sigaction)));
    struct kernel_sigset_t pending;
    CHECK(!sys_sigpending(&pending));
    CHECK(!sys_sigismember(&pending, signum));
    struct kernel_sigset_t mask, oldmask;
    CHECK(!sys_sigemptyset(&mask));
    CHECK(!sys_sigaddset(&mask, signum));
    CHECK(!sys_sigprocmask(SIG_BLOCK, &mask, &oldmask));
    CHECK(!sys_kill(sys_getpid(), signum));
    CHECK(!sys_sigpending(&pending));
    CHECK(sys_sigismember(&pending, signum));
    CHECK(!signaled);
    CHECK(!sys_sigfillset(&mask));
    CHECK(!sys_sigdelset(&mask, signum));
    CHECK(sys_sigsuspend(&mask) == -1);
    CHECK(signaled == signum);
    CHECK(!sys_sigaction(signum, &orig, NULL));
    CHECK(!sys_sigprocmask(SIG_SETMASK, &oldmask, NULL));
  }
}

static void OldSigaction() {
#if defined(__i386__) || defined(__ARM_ARCH_3__) || defined(__PPC__) ||       \
   (defined(__mips__) && _MIPS_SIM == _MIPS_SIM_ABI32)
  puts("OldSigaction...");
  int signum       = SIGPWR;
  for (int info = 0; info < 2; info++) {
    signaled         = 0;
    struct kernel_old_sigaction sa = ZERO_SIGACT, old, orig;
    CHECK(!sys__sigaction(signum, NULL, &orig));
    if (info) {
      sa.sa_sigaction_ = SigAction;
    } else {
      sa.sa_handler_   = SigHandler;
    }
    sa.sa_flags      = SA_RESETHAND | SA_RESTART | (info ? SA_SIGINFO : 0);
    memset(&sa.sa_mask, 0, sizeof(sa.sa_mask));
    CHECK(!sys__sigaction(signum, &sa, &old));
    CHECK(!memcmp(&old, &orig, sizeof(struct kernel_old_sigaction)));
    CHECK(!sys__sigaction(signum, NULL, &old));
    #ifndef __mips__
    old.sa_restorer  = sa.sa_restorer;
    #endif
    CHECK(!memcmp(&old, &sa, sizeof(struct kernel_old_sigaction)));
    unsigned long pending;
    CHECK(!sys__sigpending(&pending));
    CHECK(!(pending & (1UL << (signum - 1))));
    unsigned long mask, oldmask;
    mask             = 1 << (signum - 1);
    CHECK(!sys__sigprocmask(SIG_BLOCK, &mask, &oldmask));
    CHECK(!sys_kill(sys_getpid(), signum));
    CHECK(!sys__sigpending(&pending));
    CHECK(pending & (1UL << (signum - 1)));
    CHECK(!signaled);
    mask             = ~mask;
    CHECK(sys__sigsuspend(
    #ifndef __PPC__
                          &mask, 0,
    #endif
                          mask) == -1);
    CHECK(signaled == signum);
    CHECK(!sys__sigaction(signum, &orig, NULL));
    CHECK(!sys__sigprocmask(SIG_SETMASK, &oldmask, NULL));
  }
#endif
}

template<class A, class B>static void AlmostEquals(A a, B b) {
  double d = 0.0 + a - b;
  if (d < 0) {
    d = -d;
  }
  double avg = a/2.0 + b/2.0;
  if (avg < 4096) {
    // Round up to a minimum size. Otherwise, even minute changes could
    // trigger a false positive.
    avg = 4096;
  }
  // Check that a and b are within one percent of each other.
  CHECK(d / avg < 0.01);
}

static void StatFs() {
  puts("StatFs...");
  struct statfs64      libc_statfs;
  struct kernel_statfs kernel_statfs;
  CHECK(!statfs64("/", &libc_statfs));
  CHECK(!sys_statfs("/", &kernel_statfs));
  CHECK(libc_statfs.f_type          == kernel_statfs.f_type);
  CHECK(libc_statfs.f_bsize         == kernel_statfs.f_bsize);
  CHECK(libc_statfs.f_blocks        == kernel_statfs.f_blocks);
  AlmostEquals(libc_statfs.f_bfree,     kernel_statfs.f_bfree);
  AlmostEquals(libc_statfs.f_bavail,    kernel_statfs.f_bavail);
  CHECK(libc_statfs.f_files         == kernel_statfs.f_files);
  AlmostEquals(libc_statfs.f_ffree,     kernel_statfs.f_ffree);
  CHECK(libc_statfs.f_fsid.__val[0] == kernel_statfs.f_fsid.val[0]);
  CHECK(libc_statfs.f_fsid.__val[1] == kernel_statfs.f_fsid.val[1]);
  CHECK(libc_statfs.f_namelen       == kernel_statfs.f_namelen);
}

static void StatFs64() {
#if defined(__i386__) || defined(__ARM_ARCH_3__) ||                           \
   (defined(__mips__) && _MIPS_SIM != _MIPS_SIM_ABI64)
  puts("StatFs64...");
  struct statfs64        libc_statfs;
  struct kernel_statfs64 kernel_statfs;
  CHECK(!statfs64("/", &libc_statfs));
  CHECK(!sys_statfs64("/", &kernel_statfs));
  CHECK(libc_statfs.f_type          == kernel_statfs.f_type);
  CHECK(libc_statfs.f_bsize         == kernel_statfs.f_bsize);
  CHECK(libc_statfs.f_blocks        == kernel_statfs.f_blocks);
  AlmostEquals(libc_statfs.f_bfree,     kernel_statfs.f_bfree);
  AlmostEquals(libc_statfs.f_bavail,    kernel_statfs.f_bavail);
  CHECK(libc_statfs.f_files         == kernel_statfs.f_files);
  AlmostEquals(libc_statfs.f_ffree,     kernel_statfs.f_ffree);
  CHECK(libc_statfs.f_fsid.__val[0] == kernel_statfs.f_fsid.val[0]);
  CHECK(libc_statfs.f_fsid.__val[1] == kernel_statfs.f_fsid.val[1]);
  CHECK(libc_statfs.f_namelen       == kernel_statfs.f_namelen);
#endif
}

static void Stat() {
  static const char * const entries[] = { "/dev/null", "/bin/sh", "/", NULL };
  puts("Stat...");
  for (int i = 0; entries[i]; i++) {
    struct ::stat64    libc_stat;
    struct kernel_stat kernel_stat;
    CHECK(!::stat64(entries[i], &libc_stat));
    CHECK(!sys_stat(entries[i], &kernel_stat));
//  CHECK(libc_stat.st_dev     == kernel_stat.st_dev);
    CHECK(libc_stat.st_ino     == kernel_stat.st_ino);
    CHECK(libc_stat.st_mode    == kernel_stat.st_mode);
    CHECK(libc_stat.st_nlink   == kernel_stat.st_nlink);
    CHECK(libc_stat.st_uid     == kernel_stat.st_uid);
    CHECK(libc_stat.st_gid     == kernel_stat.st_gid);
    CHECK(libc_stat.st_rdev    == kernel_stat.st_rdev);
    CHECK(libc_stat.st_size    == kernel_stat.st_size);
#if !defined(__i386__) && !defined(__ARM_ARCH_3__) && !defined(__PPC__) &&    \
   !(defined(__mips__) && _MIPS_SIM != _MIPS_SIM_ABI64)
    CHECK(libc_stat.st_blksize == kernel_stat.st_blksize);
    CHECK(libc_stat.st_blocks  == kernel_stat.st_blocks);
#endif
    CHECK(libc_stat.st_atime   == kernel_stat.st_atime_);
    CHECK(libc_stat.st_mtime   == kernel_stat.st_mtime_);
    CHECK(libc_stat.st_ctime   == kernel_stat.st_ctime_);
  }
}

static void Stat64() {
#if defined(__i386__) || defined(__ARM_ARCH_3__) || defined(__PPC__) ||       \
   (defined(__mips__) && _MIPS_SIM != _MIPS_SIM_ABI64)
  puts("Stat64...");
  static const char * const entries[] = { "/dev/null", "/bin/sh", "/", NULL };
  for (int i = 0; entries[i]; i++) {
    struct ::stat64      libc_stat;
    struct kernel_stat64 kernel_stat;
    CHECK(!::stat64(entries[i], &libc_stat));
    CHECK(!sys_stat64(entries[i], &kernel_stat));
    CHECK(libc_stat.st_dev     == kernel_stat.st_dev);
    CHECK(libc_stat.st_ino     == kernel_stat.st_ino);
    CHECK(libc_stat.st_mode    == kernel_stat.st_mode);
    CHECK(libc_stat.st_nlink   == kernel_stat.st_nlink);
    CHECK(libc_stat.st_uid     == kernel_stat.st_uid);
    CHECK(libc_stat.st_gid     == kernel_stat.st_gid);
    CHECK(libc_stat.st_rdev    == kernel_stat.st_rdev);
    CHECK(libc_stat.st_size    == kernel_stat.st_size);
    CHECK(libc_stat.st_blksize == kernel_stat.st_blksize);
    CHECK(libc_stat.st_blocks  == kernel_stat.st_blocks);
    CHECK(libc_stat.st_atime   == kernel_stat.st_atime_);
    CHECK(libc_stat.st_mtime   == kernel_stat.st_mtime_);
    CHECK(libc_stat.st_ctime   == kernel_stat.st_ctime_);
  }
#endif
}

} // namespace

int main(int argc, char *argv[]) {
  linux_syscall_support::CheckStructures();
  linux_syscall_support::Sigaction();
  linux_syscall_support::OldSigaction();
  linux_syscall_support::StatFs();
  linux_syscall_support::StatFs64();
  linux_syscall_support::Stat();
  linux_syscall_support::Stat64();

  if (check_failures == 0)
    puts("PASS");
  else
    puts("FAIL");
  return check_failures;
}
