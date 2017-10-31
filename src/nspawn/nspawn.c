/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "alloc-util.h"
#include "barrier.h"
#include "copy.h"
#include "fd-util.h"
#include "mount-util.h"
#include "raw-clone.h"
#include "signal-util.h"
#include "strv.h"
#include "terminal-util.h"
#include "user-util.h"

static char *arg_directory = NULL;
static char **arg_parameters = NULL;
static unsigned long arg_clone_ns_flags = CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS;

static int inner_child(
                Barrier *barrier,
                const char *directory) {

        int r;

        assert(barrier);
        assert(directory);

        r = mount_verbose(LOG_ERR, "proc", "/proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
        if (r < 0)
                return r;

        /* Wait until we are cgroup-ified, so that we
         * can mount the right cgroup path writable */
        if (!barrier_place_and_sync(barrier)) { /* #3 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        execvp(arg_parameters[0], arg_parameters);

        r = -errno;
        (void) log_open();
        return log_error_errno(r, "execv(%s) failed: %m", arg_parameters[0]);
}

static int outer_child(
                Barrier *barrier,
                const char *directory,
                const char *console,
                int pid_socket) {

        pid_t pid;
        ssize_t l;
        int r;
        _cleanup_close_ int fd = -1;

        assert(barrier);
        assert(directory);
        assert(console);
        assert(pid_socket >= 0);

        r = open(console, O_RDWR, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to open console: %m");

        /* Mark everything as slave, so that we still
         * receive mounts from the real root, but don't
         * propagate mounts to the real root. */
        r = mount_verbose(LOG_ERR, NULL, "/", NULL, MS_SLAVE|MS_REC, NULL);
        if (r < 0)
                return r;

        /* Turn directory into bind mount */
        r = mount_verbose(LOG_ERR, directory, directory, NULL, MS_BIND|MS_REC, NULL);
        if (r < 0)
                return r;

        /* Mark everything as shared so our mounts get propagated down. This is
         * required to make new bind mounts available in systemd services
         * inside the containter that create a new mount namespace.
         * See https://github.com/systemd/systemd/issues/3860
         * Further submounts (such as /dev) done after this will inherit the
         * shared propagation mode. */
        r = mount_verbose(LOG_ERR, NULL, directory, NULL, MS_SHARED|MS_REC, NULL);
        if (r < 0)
                return r;

        if (chdir(directory) < 0)
                return log_error_errno(errno, "Failed to chdir: %m");
        if (chroot(".") < 0)
                return log_error_errno(errno, "Failed to chroot: %m");

        pid = raw_clone(SIGCHLD|CLONE_NEWNS|
                        arg_clone_ns_flags);
        if (pid < 0)
                return log_error_errno(errno, "Failed to fork inner child: %m");
        if (pid == 0) {
                pid_socket = safe_close(pid_socket);

                /* The inner child has all namespaces that are
                 * requested, so that we all are owned by the user if
                 * user namespaces are turned on. */

                r = inner_child(barrier, directory);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        }

        l = send(pid_socket, &pid, sizeof(pid), MSG_NOSIGNAL);
        if (l < 0)
                return log_error_errno(errno, "Failed to send PID: %m");
        if (l != sizeof(pid)) {
                log_error("Short write while sending PID.");
                return -EIO;
        }

        pid_socket = safe_close(pid_socket);

        return 0;
}

static int run(int master,
               const char* console,
               pid_t *pid, int *ret) {

        _cleanup_close_pair_ int pid_socket_pair[2] = { -1, -1 };
        _cleanup_(barrier_destroy) Barrier barrier = BARRIER_NULL;
        int r;
        ssize_t l;

        r = barrier_create(&barrier);
        if (r < 0)
                return log_error_errno(r, "Cannot initialize IPC barrier: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, pid_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create pid socket pair: %m");

        *pid = raw_clone(SIGCHLD|CLONE_NEWNS|CLONE_NEWUSER);
        if (*pid < 0)
                return log_error_errno(errno, "clone() failed%s: %m",
                                       errno == EINVAL ?
                                       ", do you have namespace support enabled in your kernel? (You need UTS, IPC, PID and NET namespacing built in)" : "");

        if (*pid == 0) {
                /* The outer child only has a file system namespace. */
                barrier_set_role(&barrier, BARRIER_CHILD);

                master = safe_close(master);

                pid_socket_pair[0] = safe_close(pid_socket_pair[0]);

                r = outer_child(&barrier,
                                arg_directory,
                                console,
                                pid_socket_pair[1]);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        }

        barrier_set_role(&barrier, BARRIER_PARENT);

        pid_socket_pair[1] = safe_close(pid_socket_pair[1]);

        /* Wait for the outer child. */
        r = wait_for_terminate_and_warn("namespace helper", *pid, NULL);
        if (r != 0)
                return r < 0 ? r : -EIO;

        /* And now retrieve the PID of the inner child. */
        l = recv(pid_socket_pair[0], pid, sizeof *pid, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to read inner child PID: %m");
        if (l != sizeof *pid) {
                log_error("Short read while reading inner child PID.");
                return -EIO;
        }

        /* Let the child know that we are ready and wait that the child is completely ready now. */
        if (!barrier_place_and_sync(&barrier)) { /* #4 */
                log_error("Child died too early.");
                return -ESRCH;
        }

        r = wait_for_terminate(*pid, NULL);
        *pid = 0;

        if (r < 0) {
                /* We failed to wait for the container, or the container exited abnormally. */
                return r;
        } else {
                *ret = r;
                return 0;
        }
}

int main(int argc, char *argv[]) {

        _cleanup_free_ char *console = NULL;
        _cleanup_close_ int master = -1;
        int r, ret = EXIT_SUCCESS;
        pid_t pid = 0;

        log_parse_environment();
        log_open();

        /* Make sure rename_process() in the stub init process can work */
        saved_argv = argv;
        saved_argc = argc;

        if (argc < 3)
                return 2;
        arg_directory = strdup(argv[1]);
        arg_parameters = strv_copy(argv + 2);

        assert(arg_directory);
        assert(!strv_isempty(arg_parameters));

        master = posix_openpt(O_RDWR|O_NOCTTY|O_CLOEXEC|O_NDELAY);
        if (master < 0) {
                r = log_error_errno(errno, "Failed to acquire pseudo tty: %m");
                goto finish;
        }

        r = ptsname_malloc(master, &console);
        if (r < 0) {
                r = log_error_errno(r, "Failed to determine tty name: %m");
                goto finish;
        }

        if (unlockpt(master) < 0) {
                r = log_error_errno(errno, "Failed to unlock tty: %m");
                goto finish;
        }

        /* for some reason removing this appends " (deleted)" to the entry in /proc/self/fd/ */
        if (prctl(PR_SET_CHILD_SUBREAPER, 1) < 0) {
                r = log_error_errno(errno, "Failed to become subreaper: %m");
                goto finish;
        }

        r = run(master,
                console,
                &pid, &ret);

finish:
        if (pid > 0)
                (void) kill(pid, SIGKILL);

        /* Try to flush whatever is still queued in the pty */
        if (master >= 0) {
                (void) copy_bytes(master, STDOUT_FILENO, (uint64_t) -1, 0);
                master = safe_close(master);
        }

        if (pid > 0)
                (void) wait_for_terminate(pid, NULL);

        free(arg_directory);
        strv_free(arg_parameters);

        return r < 0 ? EXIT_FAILURE : ret;
}
