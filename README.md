Autothing
=========

Autothing is a set of Makefile fragments that you can `include` in
your GNU Makefiles to provide two core things:

 1. Make it _easy_ to write non-recursive Makefiles. (search for the
    paper "Recursive Make Considered Harmful")
 2. Provide boilerplate for standard features people expect to be
    implemented in Makefiles.

Between these two, it should largely obviate GNU Automake in your
projects.

The recommended including Autothing into your project is to add the
autothing repository as a `git` remote, and merge the `core` branch,
and whichever `mod-*` module branches you want into your project's
branch.

| module name | dependencies | description                                                                                      |
|-------------+--------------+--------------------------------------------------------------------------------------------------|
| at          |              | The core of Autothing                                                                            |
|-------------+--------------+--------------------------------------------------------------------------------------------------|
| std         | at           | Provide .PHONY targets: all/build/install/uninstall/mostlyclean/clean/distclean/maintainer-clean |
| dist        | std          | Provide .PHONY target: dist                                                                      |
| gnuconf     | dist         | Provide default values for user-variables from the GNU Coding Standards' Makefile Conventions    |
| gnustuff    | gnuconf      | Provide remaining stuff from the GNU Coding Standards' Makefile Conventions                      |

Core (psuedo-module: at)
------------------------

As harmful as recursive make is, it's historically been difficult to
to write non-recursive Makefiles.  The goal of the core of Autothing
is to make it easy.

In each source directory, you write a `Makefile`, very similarly to if
you were writing for plain GNU Make, with the form:

    topoutdir ?= ...
    topsrcdir ?= ...
    include $(topsrcdir)/build-aux/Makefile.head.mk

    # your makefile

    include $(topsrcdir)/build-aux/Makefile.tail.mk

| at.path | Use $(call at.path,FILENAME1 FILENAME2...) sanitize filenames that are not in the current Makefile's directory or its children |
| at.nl   | A newline, for convenience, since it is difficult to type a newline in GNU Make expressions                                    |

| at.dirlocal | Which variables to apply the namespacing mechanism to                                      |
| at.phony    | Which targets to mark as .PHONY, and have automatic recursive dependencies                 |
| at.subdirs  | Which directories to consider as children of this one                                      |
| at.depdirs  | Which directories are't subdirs, but may contain dependencies of targets in this directory |

outdir
srcdir

Module: std
-----------

| Variable      | Create Command | Delete Command              | Description                       | Relative to |
|---------------+----------------+-----------------------------+-----------------------------------+-------------|
| std.src_files | emacs          | rm -rf .                    | Files that the developer writes   | srcdir      |
| std.gen_files | ???            | make maintainer-clean       | Files the developer compiles      | srcdir      |
| std.cfg_files | ./configure    | make distclean              | Users' compile-time configuration | outdir      |
| std.out_files | make all       | make mostlyclean/make clean | Files the user compiles           | outdir      |
| std.sys_files | make install   | make uninstall              | Files the user installs           | DESTDIR     |

In addition, there are two more variables that control not how files
are created, but how they are deleted:

| Variable        | Affected command | Description                                    | Relative to |
|-----------------+------------------+------------------------------------------------+-------------|
| std.clean_files | make clean       | A list of things to `rm` in addition to the    | outdir      |
|                 |                  | files in `$(std.out_files)`.  (Example: `*.o`) |             |
|-----------------+------------------+------------------------------------------------+-------------|
| std.slow_files  | make mostlyclean | A list of things that (as an exception) should | outdir      |
|                 |                  | _not_ be deleted.  (otherwise, `mostlyclean`   |             |
|                 |                  | is the same as `clean`)                        |             |

| Variable | Default  | Description |
|----------+----------+-------------|
| DESTDIR  |          |             |
|----------+----------+-------------|
| RM       | rm -f    |             |
| RMDIR_P  | rmdir -p |             |
| TRUE     | true     |             |

Module: dist
------------

The `dist` module produces distribution tarballs

| Variable     | Default    | Description |
|--------------+------------+-------------|
| dist.exts    | .tar.gz    |             |
| dist.pkgname | $(PACKAGE) |             |
| dist.version | $(VERSION) |             |

| Variable  | Default     | Description                                        |
|-----------+-------------+----------------------------------------------------|
| CP        | cp          |                                                    |
| GZIP      | gzip        |                                                    |
| MKDIR     | mkdir       |                                                    |
| MKDIR_P   | mkdir -p    |                                                    |
| MV        | mv          |                                                    |
| RM        | rm -f       |                                                    |
| TAR       | tar         |                                                    |
|-----------+-------------+----------------------------------------------------|
| GZIPFLAGS | $(GZIP_ENV) |                                                    |
| GZIP_ENV  | --best      | Because of GNU Automake, users expect this to work |

Module: gnuconf
---------------

The `gnuconf` module provides default values for user-facing toggles
required by the GNU Coding Standards.

There is only one developer configuration option:

| Variable        | Default    | Description                                   |
|-----------------+------------+-----------------------------------------------|
| gnuconf.pkgname | $(PACKAGE) | The package name to use in the default docdir |

There is an extensive list of user configuration options:

| Variable        | Default                               | Description                               |
|-----------------+---------------------------------------+-------------------------------------------|
| AWK             | awk                                   |                                           |
| CAT             | cat                                   |                                           |
| CMP             | cmp                                   |                                           |
| CP              | cp                                    |                                           |
| DIFF            | diff                                  |                                           |
| ECHO            | echo                                  |                                           |
| EGREP           | egrep                                 |                                           |
| EXPR            | expr                                  |                                           |
| FALSE           | false                                 |                                           |
| GREP            | grep                                  |                                           |
| INSTALL_INFO    | install-info                          |                                           |
| LN              | ln                                    |                                           |
| LS              | ls                                    |                                           |
| MKDIR           | mkdir                                 |                                           |
| MV              | mv                                    |                                           |
| PRINTF          | printf                                |                                           |
| PWD             | pwd                                   |                                           |
| RM              | rm                                    |                                           |
| RMDIR           | rmdir                                 |                                           |
| SED             | sed                                   |                                           |
| SLEEP           | sleep                                 |                                           |
| SORT            | sort                                  |                                           |
| TAR             | tar                                   |                                           |
| TEST            | test                                  |                                           |
| TOUCH           | touch                                 |                                           |
| TR              | tr                                    |                                           |
| TRUE            | true                                  |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| AR              | ar                                    |                                           |
| ARFLAGS         |                                       |                                           |
| BISON           | bison                                 |                                           |
| BISONFLAGS      |                                       |                                           |
| CC              | cc                                    |                                           |
| CCFLAGS         | $(CFLAGS)                             |                                           |
| FLEX            | flex                                  |                                           |
| FLEXFLAGS       |                                       |                                           |
| INSTALL         | install                               |                                           |
| INSTALLFLAGS    |                                       |                                           |
| LD              | ld                                    |                                           |
| LDFLAGS         |                                       |                                           |
| LDCONFIG        | ldconfig                              | TODO: detect absence, fall back to `true` |
| LDCONFIGFLAGS   |                                       |                                           |
| LEX             | lex                                   |                                           |
| LEXFLAGS        | $(LFLAGS)                             |                                           |
| MAKEINFO        | makeinfo                              |                                           |
| MAKEINFOFLAGS   |                                       |                                           |
| RANLIB          | ranlib                                | TODO: detect absence, fall back to `true` |
| RANLIBFLAGS     |                                       |                                           |
| TEXI2DVI        | texi2dvi                              |                                           |
| TEXI2DVIFLAGS   |                                       |                                           |
| YACC            | yacc                                  |                                           |
| YACCFLAGS       | $(YFLAGS)                             |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| CFLAGS          |                                       |                                           |
| LFLAGS          |                                       |                                           |
| YFLAGS          |                                       |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| LN_S            | ln -s                                 | TODO: detect when to fall back to `cp`    |
|-----------------+---------------------------------------+-------------------------------------------|
| CHGRP           | chgrp                                 |                                           |
| CHMOD           | chmod                                 |                                           |
| CHOWN           | chown                                 |                                           |
| MKNOD           | mknod                                 |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| INSTALL_PROGRAM | $(INSTALL)                            |                                           |
| INSTALL_DATA    | ${INSTALL} -m 644                     |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| prefix          | /usr/local                            |                                           |
| exec_prefix     | $(prefix)                             |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| bindir          | $(exec_prefix)/bin                    |                                           |
| sbindir         | $(exec_prefix)/sbin                   |                                           |
| libexecdir      | $(exec_prefix)/libexec                |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| datadir         | $(datarootdir)                        |                                           |
| sysconfdir      | $(prefix)/etc                         |                                           |
| sharedstatedir  | $(prefix)/com                         |                                           |
| localstatedir   | $(prefix)/var                         |                                           |
| runstatedir     | $(localstatedir)/run                  |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| includedir      | $(prefix)/include                     |                                           |
| oldincludedir   | /usr/include                          |                                           |
| docdir          | $(datarootdir)/doc/$(gnuconf.pkgname) |                                           |
| infodir         | $(datarootdir)/info                   |                                           |
| htmldir         | $(docdir)                             |                                           |
| dvidir          | $(docdir)                             |                                           |
| pdfdir          | $(docdir)                             |                                           |
| psdir           | $(docdir)                             |                                           |
| libdir          | $(exec_prefix)/lib                    |                                           |
| lispdir         | $(datarootdir)/emacs/site-lisp        |                                           |
| localedir       | $(datarootdir)/locale                 |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| mandir          | $(datarootdir)/man                    |                                           |
| man1dir         | $(mandir)/man1                        |                                           |
| man2dir         | $(mandir)/man2                        |                                           |
| man3dir         | $(mandir)/man3                        |                                           |
| man4dir         | $(mandir)/man4                        |                                           |
| man5dir         | $(mandir)/man5                        |                                           |
| man6dir         | $(mandir)/man6                        |                                           |
| man7dir         | $(mandir)/man7                        |                                           |
| man8dir         | $(mandir)/man8                        |                                           |
|-----------------+---------------------------------------+-------------------------------------------|
| manext          | .1                                    |                                           |
| man1ext         | .1                                    |                                           |
| man2ext         | .2                                    |                                           |
| man3ext         | .3                                    |                                           |
| man4ext         | .4                                    |                                           |
| man5ext         | .5                                    |                                           |
| man6ext         | .6                                    |                                           |
| man7ext         | .7                                    |                                           |
| man8ext         | .8                                    |                                           |

Module: gnustuff
----------------

This is a poorly-thought-out module implementing remaining things from
the GNU Coding Standards.

This is poorly thought out and poorly tested because it's basically
the part of the GNU Coding Standards that I don't use.

Developer configuration options:

| Variable              | Default                                                                                 | Description                                                                         |
|-----------------------+-----------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------|
| gnustuff.info_docs    |                                                                                         | The list of texinfo documents in the current directory, without the `.texi` suffix. |
| gnustuff.program_dirs | $(bindir) $(sbindir) $(libexecdir)                                                      | Directories to use $(INSTALL_PROGRAM) for inserting files into.                     |
| gnustuff.data_dirs    | <every variable from gnuconf ending in `dir` that isn't bindir, sbindir, or libexecdir> | Directories to use $(INSTALL_DATA) for inserting files into.                        |
| gnustuff.dirs         | $(gnustuff.program_dirs) $(gnustuff.data_dirs)                                          | Directories to create                                                               |

User configuration options:

| Variable  | Default         | Description |
|-----------+-----------------+-------------|
| STRIP     | strip           |             |
| TEXI2HTML | makeinfo --html |             |
| TEXI2PDF  | texi2pdf        |             |
| TEXI2PS   | texi2dvi --ps   |             |
| MKDIR_P   | mkdir -p        |             |

It provides several `.phony` targets:
 - install-{html,dvi,pdf,ps}
 - install-strip
 - {info,html,dvi,pdf,ps}
 - TAGS
 - check
 - installcheck

It also augments the `std` `install` rule to run $(INSTALL_INFO) as
necessary.

And several real rules:
 - How to install files into any of the $(gnustuff.program_dirs) or
   $(gnustuff.data_dirs).
 - How to generate info, dvi, html, pdf, and ps files from texi files.
