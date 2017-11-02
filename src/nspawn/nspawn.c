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

#include <sched.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "fd-util.h" /* _cleanup_close_ */
#include "mount-util.h" /* mount_verbose */

static char *arg_directory = NULL;
static char **arg_parameters = NULL;

static int outer_child(
                const char *directory,
                const char *console,
                int pid_socket) {

        pid_t pid;
        ssize_t l;
        int r;

        assert(directory);
        assert(console);
        assert(pid_socket >= 0);

        r = open(console, O_RDWR, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to open console: %m");

        if (chdir(directory) < 0)
                return log_error_errno(errno, "Failed to chdir: %m");
        if (chroot(".") < 0)
                return log_error_errno(errno, "Failed to chroot: %m");

        if (unshare(CLONE_NEWNS|CLONE_NEWPID) < 0)
                return log_error_errno(errno, "unshare: %m");
        pid = fork();
        switch (pid) {
        case -1:
                return log_error_errno(errno, "Failed to fork inner child: %m");
        case 0:
                pid_socket = safe_close(pid_socket);

                printf("a\n");
                r = mount_verbose(LOG_ERR, "proc", "/proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
                if (r < 0)
                        return r;

                printf("b\n");
                execvp(arg_parameters[0], arg_parameters);
                return -errno;
        default:
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
}

static int run(int master,
               const char* console,
               int *ret) {
        pid_t pid;

        _cleanup_close_pair_ int pid_socket_pair[2] = { -1, -1 };
        int r;
        ssize_t l;

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, pid_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create pid socket pair: %m");

        pid = fork();
        switch (pid) {
        case -1:
                return log_error_errno(errno, "clone() failed%s: %m",
                                       errno == EINVAL ?
                                ", do you have namespace support enabled in your kernel? (You need UTS, IPC, PID and NET namespacing built in)" : "");
        case 0:
                if (unshare(CLONE_NEWNS|CLONE_NEWUSER) < 0)
                        return log_error_errno(errno, "unshare: %m");

                master = safe_close(master);

                pid_socket_pair[0] = safe_close(pid_socket_pair[0]);

                r = outer_child(arg_directory,
                                console,
                                pid_socket_pair[1]);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        default:
                pid_socket_pair[1] = safe_close(pid_socket_pair[1]);

                printf("pid: %ld\n", (long)pid);
                r = wait_for_terminate_and_warn("outer child", pid, NULL);
                if (r != 0)
                        return r < 0 ? r : -EIO;

                /* And now retrieve the PID of the inner child. */
                l = recv(pid_socket_pair[0], &pid, sizeof pid, 0);
                if (l < 0)
                        return log_error_errno(errno, "Failed to read inner child PID: %m");
                if (l != sizeof pid) {
                        log_error("Short read while reading inner child PID.");
                        return -EIO;
                }

                printf("pid: %ld\n", (long)pid);
                r = wait_for_terminate_and_warn("inner child", pid, NULL);
                if (r < 0) {
                        /* We failed to wait for the container, or the container exited abnormally. */
                        return r;
                } else {
                        *ret = r;
                        return 0;
                }
        }
}

int main(int argc, char *argv[]) {

        char *console = NULL;
        _cleanup_close_ int master = -1;
        int r, ret = EXIT_SUCCESS;

        log_parse_environment();
        log_open();

        if (argc < 3)
                return 2;
        arg_directory = argv[1];
        arg_parameters = &argv[2];

        master = posix_openpt(O_RDWR|O_NOCTTY|O_CLOEXEC|O_NDELAY);
        if (master < 0) {
                r = log_error_errno(errno, "Failed to acquire pseudo tty: %m");
                goto finish;
        }

        console = ptsname(master);
        if (!console) {
                r = log_error_errno(errno, "Failed to determine tty name: %m");
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
                &ret);

finish:
        return r < 0 ? EXIT_FAILURE : ret;
}
