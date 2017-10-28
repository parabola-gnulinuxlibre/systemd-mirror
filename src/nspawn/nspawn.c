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

#ifdef HAVE_BLKID
#include <blkid.h>
#endif
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <linux/loop.h>
#include <pwd.h>
#include <sched.h>
#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/personality.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sd-bus.h"
#include "sd-daemon.h"
#include "sd-id128.h"

#include "alloc-util.h"
#include "barrier.h"
#include "base-filesystem.h"
#include "blkid-util.h"
#include "btrfs-util.h"
#include "bus-util.h"
#include "cap-list.h"
#include "capability-util.h"
#include "copy.h"
#include "dev-setup.h"
#include "dissect-image.h"
#include "env-util.h"
#include "fd-util.h"
#include "fdset.h"
#include "fileio.h"
#include "format-util.h"
#include "fs-util.h"
#include "gpt.h"
#include "hexdecoct.h"
#include "hostname-util.h"
#include "id128-util.h"
#include "log.h"
#include "loop-util.h"
#include "loopback-setup.h"
#include "machine-image.h"
#include "macro.h"
#include "missing.h"
#include "mkdir.h"
#include "mount-util.h"
#include "netlink-util.h"
#include "nspawn-mount.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "ptyfwd.h"
#include "random-util.h"
#include "raw-clone.h"
#include "rm-rf.h"
#include "selinux-util.h"
#include "signal-util.h"
#include "socket-util.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "terminal-util.h"
#include "udev-util.h"
#include "umask-util.h"
#include "user-util.h"
#include "util.h"

/* Note that devpts's gid= parameter parses GIDs as signed values, hence we stay away from the upper half of the 32bit
 * UID range here. We leave a bit of room at the lower end and a lot of room at the upper end, so that other subsystems
 * may have their own allocation ranges too. */
#define UID_SHIFT_PICK_MIN ((uid_t) UINT32_C(0x00080000))
#define UID_SHIFT_PICK_MAX ((uid_t) UINT32_C(0x6FFF0000))

#define EXIT_FORCE_RESTART 133

typedef enum ContainerStatus {
        CONTAINER_TERMINATED,
        CONTAINER_REBOOTED
} ContainerStatus;

typedef enum LinkJournal {
        LINK_NO,
        LINK_AUTO,
        LINK_HOST,
        LINK_GUEST
} LinkJournal;

static char *arg_directory = NULL;
static char *arg_chdir = NULL;
static char *arg_machine = NULL;
static const char *arg_selinux_context = NULL;
static const char *arg_selinux_apifs_context = NULL;
static uint64_t arg_caps_retain =
        (1ULL << CAP_AUDIT_CONTROL) |
        (1ULL << CAP_AUDIT_WRITE) |
        (1ULL << CAP_CHOWN) |
        (1ULL << CAP_DAC_OVERRIDE) |
        (1ULL << CAP_DAC_READ_SEARCH) |
        (1ULL << CAP_FOWNER) |
        (1ULL << CAP_FSETID) |
        (1ULL << CAP_IPC_OWNER) |
        (1ULL << CAP_KILL) |
        (1ULL << CAP_LEASE) |
        (1ULL << CAP_LINUX_IMMUTABLE) |
        (1ULL << CAP_MKNOD) |
        (1ULL << CAP_NET_BIND_SERVICE) |
        (1ULL << CAP_NET_BROADCAST) |
        (1ULL << CAP_NET_RAW) |
        (1ULL << CAP_SETFCAP) |
        (1ULL << CAP_SETGID) |
        (1ULL << CAP_SETPCAP) |
        (1ULL << CAP_SETUID) |
        (1ULL << CAP_SYS_ADMIN) |
        (1ULL << CAP_SYS_BOOT) |
        (1ULL << CAP_SYS_CHROOT) |
        (1ULL << CAP_SYS_NICE) |
        (1ULL << CAP_SYS_PTRACE) |
        (1ULL << CAP_SYS_RESOURCE) |
        (1ULL << CAP_SYS_TTY_CONFIG);
static char **arg_setenv = NULL;
static bool arg_quiet = false;
static uid_t arg_uid_shift = UID_INVALID, arg_uid_range = 0x10000U;
static int arg_kill_signal = 0;
static CGroupUnified arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_UNKNOWN;
static char **arg_parameters = NULL;
static const char *arg_container_service_name = "systemd-nspawn";
static bool arg_use_cgns = true;
static unsigned long arg_clone_ns_flags = CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS;
static MountSettingsMask arg_mount_settings = MOUNT_APPLY_APIVFS_RO;
static void *arg_root_hash = NULL;

static int parse_argv(int argc, char *argv[]) {

        static const struct option options[] = {
                { "directory",             required_argument, NULL, 'D'                     },
                {}
        };

        int c, r;
        uint64_t plus = 0, minus = 0;

        assert(argc >= 0);
        assert(argv);

        while ((c = getopt_long(argc, argv, "+hD:u:abL:M:jS:Z:qi:xp:nUE:", options, NULL)) >= 0)

                switch (c) {

                case 'D':
                        r = parse_path_argument_and_warn(optarg, false, &arg_directory);
                        if (r < 0)
                                return r;
                        break;

                case '?':
                        return -EINVAL;

                default:
                        assert_not_reached("Unhandled option");
                }

        if (argc > optind) {
                arg_parameters = strv_copy(argv + optind);
                if (!arg_parameters)
                        return log_oom();
        }

        arg_caps_retain = (arg_caps_retain | plus) & ~minus;

        r = cg_unified_flush();
        if (r < 0)
                return log_error_errno(r, "Failed to determine whether the unified cgroups hierarchy is used: %m");

        arg_use_cgns = cg_ns_supported();

        return 1;
}

static int setup_pts(const char *dest) {
        _cleanup_free_ char *options = NULL;
        const char *p;
        int r;

#ifdef HAVE_SELINUX
        if (arg_selinux_apifs_context)
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT ",context=\"%s\"",
                                arg_uid_shift + TTY_GID,
                                arg_selinux_apifs_context);
        else
#endif
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT,
                                arg_uid_shift + TTY_GID);

        if (!options)
                return log_oom();

        /* Mount /dev/pts itself */
        p = prefix_roota(dest, "/dev/pts");
        if (mkdir(p, 0755) < 0)
                return log_error_errno(errno, "Failed to create /dev/pts: %m");
        r = mount_verbose(LOG_ERR, "devpts", p, "devpts", MS_NOSUID|MS_NOEXEC, options);
        if (r < 0)
                return r;

        /* Create /dev/ptmx symlink */
        p = prefix_roota(dest, "/dev/ptmx");
        if (symlink("pts/ptmx", p) < 0)
                return log_error_errno(errno, "Failed to create /dev/ptmx symlink: %m");

        /* And fix /dev/pts/ptmx ownership */
        p = prefix_roota(dest, "/dev/pts/ptmx");

        return 0;
}

static int setup_dev_console(const char *dest, const char *console) {
        _cleanup_umask_ mode_t u;
        const char *to;
        int r;

        assert(dest);
        assert(console);

        u = umask(0000);

        r = chmod_and_chown(console, 0600, arg_uid_shift, arg_uid_shift);
        if (r < 0)
                return log_error_errno(r, "Failed to correct access mode for TTY: %m");

        /* We need to bind mount the right tty to /dev/console since
         * ptys can only exist on pts file systems. To have something
         * to bind mount things on we create a empty regular file. */

        to = prefix_roota(dest, "/dev/console");
        r = touch(to);
        if (r < 0)
                return log_error_errno(r, "touch() for /dev/console failed: %m");

        return mount_verbose(LOG_ERR, console, to, NULL, MS_BIND, NULL);
}

/*
 * Return values:
 * < 0 : wait_for_terminate() failed to get the state of the
 *       container, the container was terminated by a signal, or
 *       failed for an unknown reason.  No change is made to the
 *       container argument.
 * > 0 : The program executed in the container terminated with an
 *       error.  The exit code of the program executed in the
 *       container is returned.  The container argument has been set
 *       to CONTAINER_TERMINATED.
 *   0 : The container is being rebooted, has been shut down or exited
 *       successfully.  The container argument has been set to either
 *       CONTAINER_TERMINATED or CONTAINER_REBOOTED.
 *
 * That is, success is indicated by a return value of zero, and an
 * error is indicated by a non-zero value.
 */
static int wait_for_container(pid_t pid, ContainerStatus *container) {
        siginfo_t status;
        int r;

        r = wait_for_terminate(pid, &status);
        if (r < 0)
                return log_warning_errno(r, "Failed to wait for container: %m");

        switch (status.si_code) {

        case CLD_EXITED:
                if (status.si_status == 0)
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s exited successfully.", arg_machine);
                else
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s failed with error code %i.", arg_machine, status.si_status);

                *container = CONTAINER_TERMINATED;
                return status.si_status;

        case CLD_KILLED:
                if (status.si_status == SIGINT) {
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s has been shut down.", arg_machine);
                        *container = CONTAINER_TERMINATED;
                        return 0;

                } else if (status.si_status == SIGHUP) {
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s is being rebooted.", arg_machine);
                        *container = CONTAINER_REBOOTED;
                        return 0;
                }

                /* fall through */

        case CLD_DUMPED:
                log_error("Container %s terminated by signal %s.", arg_machine, signal_to_string(status.si_status));
                return -EIO;

        default:
                log_error("Container %s failed due to unknown reason.", arg_machine);
                return -EIO;
        }
}

static int on_orderly_shutdown(sd_event_source *s, const struct signalfd_siginfo *si, void *userdata) {
        pid_t pid;

        pid = PTR_TO_PID(userdata);
        if (pid > 0) {
                if (kill(pid, arg_kill_signal) >= 0) {
                        log_info("Trying to halt container. Send SIGTERM again to trigger immediate termination.");
                        sd_event_source_set_userdata(s, NULL);
                        return 0;
                }
        }

        sd_event_exit(sd_event_source_get_event(s), 0);
        return 0;
}

static int on_sigchld(sd_event_source *s, const struct signalfd_siginfo *ssi, void *userdata) {
        for (;;) {
                siginfo_t si = {};
                if (waitid(P_ALL, 0, &si, WNOHANG|WNOWAIT|WEXITED) < 0)
                        return log_error_errno(errno, "Failed to waitid(): %m");
                if (si.si_pid == 0) /* No pending children. */
                        break;
                if (si.si_pid == PTR_TO_PID(userdata)) {
                        /* The main process we care for has exited. Return from
                         * signal handler but leave the zombie. */
                        sd_event_exit(sd_event_source_get_event(s), 0);
                        break;
                }
                /* Reap all other children. */
                (void) waitid(P_PID, si.si_pid, &si, WNOHANG|WEXITED);
        }

        return 0;
}

static int determine_names(void) {

        assert(arg_directory);

        if (!arg_machine) {
                arg_machine = strdup(basename(arg_directory));
                if (!arg_machine)
                        return log_oom();

                hostname_cleanup(arg_machine);
                if (!machine_name_is_valid(arg_machine)) {
                        log_error("Failed to determine machine name automatically, please use -M.");
                        return -EINVAL;
                }
        }

        return 0;
}

static int inner_child(
                Barrier *barrier,
                const char *directory,
                FDSet *fds) {

        _cleanup_free_ char *home = NULL;
        unsigned n_env = 1;
        const char *envp[] = {
                "PATH=" DEFAULT_PATH_SPLIT_USR,
                NULL, /* container */
                NULL, /* TERM */
                NULL, /* HOME */
                NULL, /* USER */
                NULL, /* LOGNAME */
                NULL, /* LISTEN_FDS */
                NULL, /* LISTEN_PID */
                NULL
        };
        const char *exec_target;

        _cleanup_strv_free_ char **env_use = NULL;
        int r;

        assert(barrier);
        assert(directory);

        r = reset_uid_gid();
        if (r < 0)
                return log_error_errno(r, "Couldn't become new root: %m");

        r = mount_all(NULL,
                      arg_mount_settings | MOUNT_IN_USERNS,
                      arg_uid_shift,
                      arg_uid_range,
                      arg_selinux_apifs_context);

        if (r < 0)
                return r;

        r = mount_sysfs(NULL, arg_mount_settings);
        if (r < 0)
                return r;

        /* Wait until we are cgroup-ified, so that we
         * can mount the right cgroup path writable */
        if (!barrier_place_and_sync(barrier)) { /* #3 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (arg_use_cgns && cg_ns_supported()) {
                r = unshare(CLONE_NEWCGROUP);
                if (r < 0)
                        return log_error_errno(errno, "Failed to unshare cgroup namespace");
                r = mount_cgroups(
                                "",
                                arg_unified_cgroup_hierarchy,
                                false,
                                arg_uid_shift,
                                arg_uid_range,
                                arg_selinux_apifs_context,
                                true);
                if (r < 0)
                        return r;
        } else {
                r = mount_systemd_cgroup_writable("", arg_unified_cgroup_hierarchy);
                if (r < 0)
                        return r;
        }

        umask(0022);

        if (setsid() < 0)
                return log_error_errno(errno, "setsid() failed: %m");

#ifdef HAVE_SELINUX
        if (arg_selinux_context)
                if (setexeccon(arg_selinux_context) < 0)
                        return log_error_errno(errno, "setexeccon(\"%s\") failed: %m", arg_selinux_context);
#endif

        /* LXC sets container=lxc, so follow the scheme here */
        envp[n_env++] = strjoina("container=", arg_container_service_name);

        envp[n_env] = strv_find_prefix(environ, "TERM=");
        if (envp[n_env])
                n_env++;

        if ((asprintf((char**)(envp + n_env++), "HOME=%s", home ? home: "/root") < 0) ||
            (asprintf((char**)(envp + n_env++), "USER=%s", "root") < 0) ||
            (asprintf((char**)(envp + n_env++), "LOGNAME=%s", "root") < 0))
                return log_oom();

        if (fdset_size(fds) > 0) {
                r = fdset_cloexec(fds, false);
                if (r < 0)
                        return log_error_errno(r, "Failed to unset O_CLOEXEC for file descriptors.");

                if ((asprintf((char **)(envp + n_env++), "LISTEN_FDS=%u", fdset_size(fds)) < 0) ||
                    (asprintf((char **)(envp + n_env++), "LISTEN_PID=1") < 0))
                        return log_oom();
        }

        env_use = strv_env_merge(2, envp, arg_setenv);
        if (!env_use)
                return log_oom();

        /* Let the parent know that we are ready and
         * wait until the parent is ready with the
         * setup, too... */
        if (!barrier_place_and_sync(barrier)) { /* #4 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (arg_chdir)
                if (chdir(arg_chdir) < 0)
                        return log_error_errno(errno, "Failed to change to specified working directory %s: %m", arg_chdir);

        /* Now, explicitly close the log, so that we
         * then can close all remaining fds. Closing
         * the log explicitly first has the benefit
         * that the logging subsystem knows about it,
         * and is thus ready to be reopened should we
         * need it again. Note that the other fds
         * closed here are at least the locking and
         * barrier fds. */
        log_close();
        (void) fdset_close_others(fds);

        if (!strv_isempty(arg_parameters)) {
                exec_target = arg_parameters[0];
                execvpe(arg_parameters[0], arg_parameters, env_use);
        } else {
                if (!arg_chdir)
                        /* If we cannot change the directory, we'll end up in /, that is expected. */
                        (void) chdir(home ?: "/root");

                execle("/bin/bash", "-bash", NULL, env_use);
                execle("/bin/sh", "-sh", NULL, env_use);

                exec_target = "/bin/bash, /bin/sh";
        }

        r = -errno;
        (void) log_open();
        return log_error_errno(r, "execv(%s) failed: %m", exec_target);
}

static int outer_child(
                Barrier *barrier,
                const char *directory,
                const char *console,
                bool interactive,
                int pid_socket,
                int uid_shift_socket,
                FDSet *fds) {

        pid_t pid;
        ssize_t l;
        int r;
        _cleanup_close_ int fd = -1;

        assert(barrier);
        assert(directory);
        assert(console);
        assert(pid_socket >= 0);

        if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
                return log_error_errno(errno, "PR_SET_PDEATHSIG failed: %m");

        if (interactive) {
                close_nointr(STDIN_FILENO);
                close_nointr(STDOUT_FILENO);
                close_nointr(STDERR_FILENO);

                r = open_terminal(console, O_RDWR);
                if (r != STDIN_FILENO) {
                        if (r >= 0) {
                                safe_close(r);
                                r = -EINVAL;
                        }

                        return log_error_errno(r, "Failed to open console: %m");
                }

                if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO ||
                    dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
                        return log_error_errno(errno, "Failed to duplicate console: %m");
        }

        /* Mark everything as slave, so that we still
         * receive mounts from the real root, but don't
         * propagate mounts to the real root. */
        r = mount_verbose(LOG_ERR, NULL, "/", NULL, MS_SLAVE|MS_REC, NULL);
        if (r < 0)
                return r;

        arg_uid_shift = 0;

        /* Turn directory into bind mount */
        r = mount_verbose(LOG_ERR, directory, directory, NULL, MS_BIND|MS_REC, NULL);
        if (r < 0)
                return r;

        r = setup_pivot_root(directory, NULL, NULL);
        if (r < 0)
                return r;

        r = setup_volatile(
                        directory,
                        false,
                        false,
                        arg_uid_shift,
                        arg_uid_range,
                        arg_selinux_context);
        if (r < 0)
                return r;

        r = setup_volatile_state(
                        directory,
                        false,
                        false,
                        arg_uid_shift,
                        arg_uid_range,
                        arg_selinux_context);
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

        r = base_filesystem_create(directory, arg_uid_shift, (gid_t) arg_uid_shift);
        if (r < 0)
                return r;

        r = mount_all(directory,
                      arg_mount_settings,
                      arg_uid_shift,
                      arg_uid_range,
                      arg_selinux_apifs_context);
        if (r < 0)
                return r;

        dev_setup(directory, arg_uid_shift, arg_uid_shift);

        r = setup_pts(directory);
        if (r < 0)
                return r;

        r = setup_dev_console(directory, console);
        if (r < 0)
                return r;

        if (!arg_use_cgns || !cg_ns_supported()) {
                r = mount_cgroups(
                                directory,
                                arg_unified_cgroup_hierarchy,
                                false,
                                arg_uid_shift,
                                arg_uid_range,
                                arg_selinux_apifs_context,
                                false);
                if (r < 0)
                        return r;
        }

        r = mount_move_root(directory);
        if (r < 0)
                return log_error_errno(r, "Failed to move root directory: %m");

        pid = raw_clone(SIGCHLD|CLONE_NEWNS|
                        arg_clone_ns_flags);
        if (pid < 0)
                return log_error_errno(errno, "Failed to fork inner child: %m");
        if (pid == 0) {
                pid_socket = safe_close(pid_socket);
                uid_shift_socket = safe_close(uid_shift_socket);

                /* The inner child has all namespaces that are
                 * requested, so that we all are owned by the user if
                 * user namespaces are turned on. */

                r = inner_child(barrier, directory, fds);
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
               bool interactive,
               FDSet *fds,
               pid_t *pid, int *ret) {

        static const struct sigaction sa = {
                .sa_handler = nop_signal_handler,
                .sa_flags = SA_NOCLDSTOP|SA_RESTART,
        };

        _cleanup_release_lock_file_ LockFile uid_shift_lock = LOCK_FILE_INIT;
        _cleanup_close_ int etc_passwd_lock = -1;
        _cleanup_close_pair_ int
                pid_socket_pair[2] = { -1, -1 },
                uid_shift_socket_pair[2] = { -1, -1 };
        _cleanup_(barrier_destroy) Barrier barrier = BARRIER_NULL;
        _cleanup_(sd_event_unrefp) sd_event *event = NULL;
        _cleanup_(pty_forward_freep) PTYForward *forward = NULL;
        ContainerStatus container_status = 0;
        char last_char = 0;
        int r;
        ssize_t l;
        sigset_t mask_chld;

        assert_se(sigemptyset(&mask_chld) == 0);
        assert_se(sigaddset(&mask_chld, SIGCHLD) == 0);

        r = barrier_create(&barrier);
        if (r < 0)
                return log_error_errno(r, "Cannot initialize IPC barrier: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, pid_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create pid socket pair: %m");

        /* Child can be killed before execv(), so handle SIGCHLD in order to interrupt
         * parent's blocking calls and give it a chance to call wait() and terminate. */
        r = sigprocmask(SIG_UNBLOCK, &mask_chld, NULL);
        if (r < 0)
                return log_error_errno(errno, "Failed to change the signal mask: %m");

        r = sigaction(SIGCHLD, &sa, NULL);
        if (r < 0)
                return log_error_errno(errno, "Failed to install SIGCHLD handler: %m");

        *pid = raw_clone(SIGCHLD|CLONE_NEWNS);
        if (*pid < 0)
                return log_error_errno(errno, "clone() failed%s: %m",
                                       errno == EINVAL ?
                                       ", do you have namespace support enabled in your kernel? (You need UTS, IPC, PID and NET namespacing built in)" : "");

        if (*pid == 0) {
                /* The outer child only has a file system namespace. */
                barrier_set_role(&barrier, BARRIER_CHILD);

                master = safe_close(master);

                pid_socket_pair[0] = safe_close(pid_socket_pair[0]);
                uid_shift_socket_pair[0] = safe_close(uid_shift_socket_pair[0]);

                (void) reset_all_signal_handlers();
                (void) reset_signal_mask();

                r = outer_child(&barrier,
                                arg_directory,
                                console,
                                interactive,
                                pid_socket_pair[1],
                                uid_shift_socket_pair[1],
                                fds);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        }

        barrier_set_role(&barrier, BARRIER_PARENT);

        fds = fdset_free(fds);

        pid_socket_pair[1] = safe_close(pid_socket_pair[1]);
        uid_shift_socket_pair[1] = safe_close(uid_shift_socket_pair[1]);

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

        log_debug("Init process invoked as PID "PID_FMT, *pid);

        /* Notify the child that the parent is ready with all
         * its setup (including cgroup-ification), and that
         * the child can now hand over control to the code to
         * run inside the container. */
        (void) barrier_place(&barrier); /* #3 */

        /* Block SIGCHLD here, before notifying child.
         * process_pty() will handle it with the other signals. */
        assert_se(sigprocmask(SIG_BLOCK, &mask_chld, NULL) >= 0);

        /* Reset signal to default */
        r = default_signals(SIGCHLD, -1);
        if (r < 0)
                return log_error_errno(r, "Failed to reset SIGCHLD: %m");

        r = sd_event_new(&event);
        if (r < 0)
                return log_error_errno(r, "Failed to get default event source: %m");

        /* Let the child know that we are ready and wait that the child is completely ready now. */
        if (!barrier_place_and_sync(&barrier)) { /* #4 */
                log_error("Child died too early.");
                return -ESRCH;
        }

        /* At this point we have made use of the UID we picked, and thus nss-mymachines
         * will make them appear in getpwuid(), thus we can release the /etc/passwd lock. */
        etc_passwd_lock = safe_close(etc_passwd_lock);

        if (arg_kill_signal > 0) {
                /* Try to kill the init system on SIGINT or SIGTERM */
                sd_event_add_signal(event, NULL, SIGINT, on_orderly_shutdown, PID_TO_PTR(*pid));
                sd_event_add_signal(event, NULL, SIGTERM, on_orderly_shutdown, PID_TO_PTR(*pid));
        } else {
                /* Immediately exit */
                sd_event_add_signal(event, NULL, SIGINT, NULL, NULL);
                sd_event_add_signal(event, NULL, SIGTERM, NULL, NULL);
        }

        /* Exit when the child exits */
        sd_event_add_signal(event, NULL, SIGCHLD, on_sigchld, PID_TO_PTR(*pid));

        r = pty_forward_new(event, master,
                            PTY_FORWARD_IGNORE_VHANGUP | (interactive ? 0 : PTY_FORWARD_READ_ONLY),
                            &forward);
        if (r < 0)
                return log_error_errno(r, "Failed to create PTY forwarder: %m");

        r = sd_event_loop(event);
        if (r < 0)
                return log_error_errno(r, "Failed to run event loop: %m");

        pty_forward_get_last_char(forward, &last_char);

        forward = pty_forward_free(forward);

        if (!arg_quiet && last_char != '\n')
                putc('\n', stdout);

        /* Normally redundant, but better safe than sorry */
        (void) kill(*pid, SIGKILL);

        r = wait_for_container(*pid, &container_status);
        *pid = 0;

        if (r < 0)
                /* We failed to wait for the container, or the container exited abnormally. */
                return r;
        if (r > 0 || container_status == CONTAINER_TERMINATED) {
                /* r > 0 → The container exited with a non-zero status.
                 *         As a special case, we need to replace 133 with a different value,
                 *         because 133 is special-cased in the service file to reboot the container.
                 * otherwise → The container exited with zero status and a reboot was not requested.
                 */
                if (r == EXIT_FORCE_RESTART)
                        r = EXIT_FAILURE; /* replace 133 with the general failure code */
                *ret = r;
                return 0; /* finito */
        }

        /* CONTAINER_REBOOTED, loop again */

        /* Special handling if we are running as a service: instead of simply
         * restarting the machine we want to restart the entire service, so let's
         * inform systemd about this with the special exit code 133. The service
         * file uses RestartForceExitStatus=133 so that this results in a full
         * nspawn restart. This is necessary since we might have cgroup parameters
         * set we want to have flushed out. */
        *ret = EXIT_FORCE_RESTART;
        return 0; /* finito */
}

int main(int argc, char *argv[]) {

        _cleanup_free_ char *console = NULL;
        _cleanup_close_ int master = -1;
        _cleanup_fdset_free_ FDSet *fds = NULL;
        int r, ret = EXIT_SUCCESS;
        pid_t pid = 0;
        _cleanup_release_lock_file_ LockFile tree_global_lock = LOCK_FILE_INIT, tree_local_lock = LOCK_FILE_INIT;
        bool interactive;
        _cleanup_(loop_device_unrefp) LoopDevice *loop = NULL;
        _cleanup_(decrypted_image_unrefp) DecryptedImage *decrypted_image = NULL;

        log_parse_environment();
        log_open();

        /* Make sure rename_process() in the stub init process can work */
        saved_argv = argv;
        saved_argc = argc;

        r = parse_argv(argc, argv);
        if (r <= 0)
                goto finish;

        r = determine_names();
        if (r < 0)
                goto finish;

        assert(arg_directory);

        interactive =
                isatty(STDIN_FILENO) > 0 &&
                isatty(STDOUT_FILENO) > 0;

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

        assert_se(sigprocmask_many(SIG_BLOCK, NULL, SIGCHLD, SIGWINCH, SIGTERM, SIGINT, -1) >= 0);

        if (prctl(PR_SET_CHILD_SUBREAPER, 1) < 0) {
                r = log_error_errno(errno, "Failed to become subreaper: %m");
                goto finish;
        }

        r = run(master,
                console,
                interactive,
                fds,
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
        free(arg_machine);
        free(arg_chdir);
        strv_free(arg_setenv);
        strv_free(arg_parameters);
        free(arg_root_hash);

        return r < 0 ? EXIT_FAILURE : ret;
}
