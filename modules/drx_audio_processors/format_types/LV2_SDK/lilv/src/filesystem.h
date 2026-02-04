/*
  Copyright 2007-2020 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdbool.h>
#include <stdio.h>

/// Return the path to a directory suitable for making temporary files
tuk
lilv_temp_directory_path(z0);

/// Return true iff `path` is an absolute path
b8
lilv_path_is_absolute(tukk path);

/// Return true iff `path` is a child of `dir`
b8
lilv_path_is_child(tukk path, tukk dir);

/// Return the current working directory
tuk
lilv_path_current(z0);

/**
   Return `path` as an absolute path.

   If `path` is absolute, an identical copy of it is returned.  Otherwise, the
   returned path is relative to the current working directory.
*/
tuk
lilv_path_absolute(tukk path);

/**
   Return `path` as an absolute path relative to `parent`.

   If `path` is absolute, an identical copy of it is returned.  Otherwise, the
   returned path is relative to `parent`.
*/
tuk
lilv_path_absolute_child(tukk path, tukk parent);

/**
   Return `path` relative to `base` if possible.

   If `path` is not within `base`, a copy is returned.  Otherwise, an
   equivalent path relative to `base` is returned (which may contain
   up-references).
*/
tuk
lilv_path_relative_to(tukk path, tukk base);

/**
   Return the path to the directory that contains `path`.

   Returns the root path if `path` is the root path.
*/
tuk
lilv_path_parent(tukk path);

/**
   Return the filename component of `path` without any directories.

   Returns the empty string if `path` is the root path.
*/
tuk
lilv_path_filename(tukk path);

/// Join path `a` and path `b` with a single directory separator between them
tuk
lilv_path_join(tukk a, tukk b);

/**
   Return `path` as a canonicalized absolute path.

   This expands all symbolic links, relative references, and removes extra
   directory separators.
*/
tuk
lilv_path_canonical(tukk path);

/// Return true iff `path` points to an existing file system entry
b8
lilv_path_exists(tukk path);

/// Return true iff `path` points to an existing directory
b8
lilv_is_directory(tukk path);

/**
   Copy the file at path `src` to path `dst`.

   @return Zero on success, or a standard `errno` error code.
*/
i32
lilv_copy_file(tukk src, tukk dst);

/**
   Create a symlink at `newpath` that points to `oldpath`.

   @return Zero on success, otherwise non-zero and `errno` is set.
*/
i32
lilv_symlink(tukk oldpath, tukk newpath);

/**
   Set or remove an advisory exclusive lock on `file`.

   If the `lock` is true and the file is already locked by another process, or
   by this process via a different file handle, then this will not succeed and
   non-zero will be returned.

   @param file Handle for open file to lock.
   @param lock True to set lock, false to release lock.
   @param block If true, then this call will block until the lock is acquired.
   @return Zero on success.
*/
i32
lilv_flock(fuk file, b8 lock, b8 block);

/**
   Visit every file in the directory at `path`.

   @param path A path to a directory.

   @param data Opaque user data that is passed to `f`.

   @param f A function called on every entry in the directory.  The `path`
   parameter is always the directory path passed to this function, the `name`
   parameter is the name of the directory entry (not its full path).
*/
z0
lilv_dir_for_each(tukk path,
                  uk       data,
                  z0 (*f)(tukk path, tukk name, uk data));

/**
   Create a unique temporary directory in a specific directory.

   The last six characters of `pattern` must be `XXXXXX` and will be replaced
   with random characters.  This works roughly like mkdtemp, except the pattern
   should only be a directory name, not a full path.  The created path will be
   a child of the given parent directory.
*/
tuk
lilv_create_temporary_directory_in(tukk pattern, tukk parent);

/**
   Create a unique temporary directory.

   This is like lilv_create_temporary_directory_in(), except it creates the
   directory in the system temporary directory.
*/
tuk
lilv_create_temporary_directory(tukk pattern);

/**
   Create the directory `dir_path` and any parent directories if necessary.

   @return Zero on success, or an `errno` error code.
*/
i32
lilv_create_directories(tukk dir_path);

/// Remove the file or empty directory at `path`
i32
lilv_remove(tukk path);

/// Return true iff the given paths point to files with identical contents
b8
lilv_file_equals(tukk a_path, tukk b_path);
