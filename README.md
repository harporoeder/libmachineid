# libmachineid

Cross-platform C89 library to get a platform identifier.

# Quick start

An example on Ubuntu:

```bash
sudo apt-get -y update
sudo apt-get install -y build-essential cmake git
git clone https://github.com/harporoeder/libmachineid.git
cd libmachineid
mkdir build
cmake ..
make
./test
> machine id: 4f6d8e5d-3837-473a-a707-8ec1f310e717
> error: MACHINEID_ERROR_NONE
```

# Platform support

The projected has been tested on Windows, MacOS, Linux, FreeBSD, and OpenBSD.
If a platform does not have first class support it will gracefully degrade with
reduced functionality.

If you need support for a specific platform that is not already handled open
an issue and it can likely be added contingent on my ability to test against it.

# Compiler support

The projected has been tested with Clang, MSVC, GCC, and TCC.

If you need support for additional compilers, and `libmachineid` does not
already build feel free to open an issue.

# Versioning

This project follows [semantic versioning](https://semver.org/). Additionally
the `main` branch is maintained in a usable state with development happening
on branches.

There exist 4 declarations inside `machineid.h` that can be used to
programmatically determine the version. There is a version string of the name
`MACHINEID_VERSION`, and 3 declarations for each specific component
`MACHINEID_VERSION_MAJOR`, `MACHINEID_VERSION_MINOR`,
and `MACHINEID_VERSION_PATCH`.

# Build system requirements

While [CMake](https://cmake.org/) is used in this repository it can be easily
avoided based on preference for an alternative build system.

The only notable complication is that you are required to define
`MACHINEID_USE_SODIUM`, or `MACHINEID_USE_OPENSSL` if you utilize either of
of those libraries with `libmachineid`.

# Sources of identifiers

## Linux

On Linux the file `/etc/machine-id` is used with `/var/lib/dbus/machine-id`
as an alternative.

## OpenBSD

On OpenBSD `HW_UUID` is queried. The file `/etc/machine-id` is used as an
alternative. DBUS is not installed by default and `/etc/machine-id` may not
be present.

## FreeBSD

On FreeBSD the file `/etc/hostid` is used.

## MacOS

On MacOS the registry key `kIOPlatformUUIDKey` is queried.

## Windows

On Windows the registry key `MachineGuid` is queried.

# Sources of entropy

When built with either `sodium` or `openssl` the random number generators
provided by those libraries are always used, otherwise a platform specific
generator is used.

On Windows `rand_s` is used. For OpenBSD or FreeBSD `arc4random_buf` is used.
On other platforms `rand` is used. When using `rand` you must ensure that you
seed with `srand` else fallback identifiers will be the same across
instances of your application.