Autothing 3: The smart way to write GNU Makefiles
=================================================

Autothing is a thing that does things automatically.

Ok, more helpfully: Autothing is a pair of .mk Makefile fragments that
you can `include` from your Makefiles to make them easier to write;
specifically, it makes it _easy_ to write non-recursive Makefiles--and
ones that are similar to plain recursive Makefiles, at that!

Sample
------

Write your makefiles of the form:

	topsrcdir ?= ...
	topoutdir ?= ...
	at.Makefile ?= Makefile # Optional
	include $(topsrcdir)/build-aux/Makefile.head.mk

	$(outdir)/%.o: $(srcdir)/%.c:
		$(CC) -c -o $@ $<

	$(outdir)/hello: $(outdir)/hello.o

	at.subdirs = ...
	at.targets = ...

	include $(topsrcdir)/build-aux/Makefile.tail.mk

This is similar to, but not quite, the comfortable way that you probably
already write your Makefiles.

What's where?
-------------

There are three things that Autothing provides:

 1. Variable namespacing
 2. Tools for dealing with paths
 3. A module (plugin) system.

This repository contains both Autothing itself, and several modules.

Autothing itself is described in `build-aux/Makefile.README.txt`.
That file is the core documentation.

There is a "mod" module that adds self-documenting capabilities to the
module system; adding Make targets that print documentation about the
modules used in a project.  For convenience, in the top level of this
repository, there is a Makefile allowing you to use these targets to
get documentation on the modules in this repository.

Running `make at-modules` will produce a list of modules, and short
descriptions of them:

	$ make at-modules
	Autothing modules used in this project:
	 - dist             `dist` target for distribution tarballs        (more)
	 - files            Keeping track of groups of files               (more)
	 - gitfiles         Automatically populate files.src.src from git  (more)
	 - gnuconf          GNU standard configuration variables           (more)
	 - mod              Display information about Autothing modules    (more)
	 - nested           Easy nested .PHONY targets                     (more)
	 - quote            Macros to quote tricky strings                 (more)
	 - texinfo          The GNU documentation system                   (more)
	 - var              Depend on the values of variables              (more)
	 - write-atomic     `write-atomic` auxiliary build script          (more)
	 - write-ifchanged  `write-ifchanged` auxiliary build script       (more)

The "(more)" at the end of a line indicates that there is further
documentation for that module, which can be produced by running the
command `make at-modules/MODULE_NAME`.

Further development
-------------------

The raison d'être of GNU Automake is that targeting multiple
implementations of Make is hard; the different dialects have diverged
significantly.

But GNU's requirement of supporting multiple implementations of Make
is relaxing.  With GNU Emacs 25, it GNU Make is explicitly required.
We can finally rise up from our Automake shackles!

... But we soon discover that the GNU Coding Standards require many
things of our Makefiles, which Automake took care of for us.

So, several of the modules in this repository combine to attempt to
provide the things that the GNU Coding Standards require.  Between
`gnuconf`, `dist`, `files`, and `texinfo`; the GNU Coding Standards
for Makefiles are nearly entirely satisfied.  However, there are a few
targets that are required, but aren't implemented by a module (yet!):

 - `install-strip`
 - `TAGS`
 - `check`
 - `installcheck` (optional, but recommended)

Further, none of the standard modules actually provide rules for
installing files; they merely define the standard install targets with
dependencies on the files they need to install.  This is because
actual rules for installing them can be project-specific, but also
depend on classes of files that none of the modules are aware of;
binary executables might need a strip flag passed to INSTALL, but we
need to avoid that flag for scripts; some parts might need libtool
install commands, others not.

----
Copyright (C) 2016-2017  Luke Shumaker

This documentation file is placed into the public domain.  If that is
not possible in your legal system, I grant you permission to use it in
absolutely every way that I can legally grant to you.
