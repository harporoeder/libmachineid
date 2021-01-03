# libmachineid

Cross-platform C89 library to get a platform identifier, `libmachineid` will
attempt to gather a system identifier. If one cannot be determined a random
identifier is generated. The system identifier is hashed, and then optionally
truncated and converted to a UUIDv4 representation.

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

## Error conditions

During utilization of `machineid_generate` an error may be returned of the
type `enum machineid_error`. Error conditions should be checked to ensure
that the result buffer will actually contain a valid result.

* `MACHINEID_ERROR_NONE` An identifier was successfully found.
* `MACHINEID_ERROR_RNG` An identifier was not found and the random number
generator also failed. No result is provided in this case.
* `MACHINEID_ERROR_NULL_OUTPUT_BUFFER` The library was utilized incorrectly,
and nowhere to store the result was indicated. No result is provided in this
case.
* `MACHINEID_ERROR_FALLBACK` An identifier was not found, but a fallback
was successfully generated
* `MACHINEID_HASH_FAILURE` The library utilized for hashing returned an error.
No result is provided in this case.

Error cases can be converted to a constant string with
`machineid_error_to_string`. This is likely useful for logging failures. The
library will never log of it's own volition.

## Flags

Bit flags of the type `enum machineid_flags` are provided to control the
desired behavior of `machineid_generate`.

* `MACHINEID_FLAG_DEFAULT` The default behavior of placing a raw digest in the
result buffer. This flag is only required when no other flags are used. When
using this mode the result buffer must be at least `MACHINEID_HASH_SIZE` in
size.
* `MACHINEID_FLAG_AS_UUID` Instead of returning a raw digest in the result 
buffer, truncate the digest and encode it as a UUIDv4. In this mode the
result buffer must be at least `MACHINEID_UUID_SIZE` in size.
* `MACHINEID_FLAG_NULL_TERMINATE` The character `'\0'` will be inserted at the
end of the result. When using this flag the result buffer must be either
`MACHINEID_HASH_SIZE + 1`, or `MACHINEID_UUID_SIZE + 1` as a minimum size
depending on the other flags used.

## Full example of library usage

```c
#include <stdio.h>

#include "machineid.h"

int main() {
    enum machineid_error err;
    unsigned char buffer[MACHINEID_UUID_SIZE + 1];

    err = machineid_generate(buffer, MACHINEID_FLAG_AS_UUID
        | MACHINEID_FLAG_NULL_TERMINATE);

    printf("error: %s\n", machineid_error_to_string(err));

    if (err != MACHINEID_ERROR_NONE && err != MACHINEID_ERROR_FALLBACK) {
        return 1;
    }
    
    printf("machine id: %s\n", buffer);

    return 0;
}
```

## Cryptography library integrations

For convenience `libmachineid` provides a vendored implementation of `SHA256`,
and platform specific random number generation capabilities. I recommend if
possible utilizing an external cryptography library. Support is available for
`sodium`, or `openssl`.

* To build with `sodium` use `cmake -D MACHINEID_USE_SODIUM=ON ..`
* To build with `openssl` use `cmake -D MACHINEID_USE_OPENSSL=ON ..`

When compiled with either of those options the vendored `SHA256` will not be
built, and random number generation will switch to using them.

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

# Considerations when utilizing Docker

It is common for Docker containers to have neither `/var/lib/dbus/machine-id`,
or `/etc/machine-id`. In this instance a fallback is generated. These files
can be made accessible to a container from the host system utilizing volumes:

```bash
docker run --rm -it \
    -v /etc/machine-id:/etc/machine-id:ro \
    ubuntu /bin/bash
```

This is not a universal property of Docker containers. Some may internally
run DBus, or otherwise provide a `machine-id`.