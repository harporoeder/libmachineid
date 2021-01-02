cmake_minimum_required (VERSION 3.0.2)

project (machineid)

option(MACHINEID_USE_SODIUM
    "if libsodium should be used for hashing and random number generation"
    OFF
)

option(MACHINEID_USE_OPENSSL
    "if openssl should be used for hashing and random number generation"
    OFF
)

if (MACHINEID_USE_SODIUM AND MACHINEID_USE_OPENSSL)
    message ( FATAL_ERROR
        "Cannot MACHINEID_USE_SODIUM AND MACHINEID_USE_OPENSSL"
    )
endif()

add_library (machineid machineid.c)

if (MACHINEID_USE_SODIUM)
    target_compile_definitions(machineid PRIVATE MACHINEID_USE_SODIUM)
    target_link_libraries (machineid sodium)
elseif (MACHINEID_USE_OPENSSL)
    find_package(OpenSSL REQUIRED COMPONENTS Crypto)
    target_compile_definitions(machineid PRIVATE MACHINEID_USE_OPENSSL)
    target_link_libraries (machineid OpenSSL::Crypto)
else ()
    target_sources (machineid PRIVATE sha256.c)
endif()

add_executable (main main.c)

target_link_libraries (main machineid)

target_compile_options (machineid PRIVATE
    -std=c89
    -pedantic
    -Wextra
)