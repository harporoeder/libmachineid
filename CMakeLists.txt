cmake_minimum_required (VERSION 3.0.2)

project (machineid)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles")

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
    find_package(Sodium REQUIRED)
    target_compile_definitions(machineid PRIVATE MACHINEID_USE_SODIUM)
    target_link_libraries (machineid PRIVATE ${sodium_LIBRARIES})
    target_include_directories (machineid PRIVATE ${sodium_INCLUDE_DIR})
elseif (MACHINEID_USE_OPENSSL)
    find_package(OpenSSL REQUIRED COMPONENTS Crypto)
    target_compile_definitions(machineid PRIVATE MACHINEID_USE_OPENSSL)
    target_link_libraries (machineid PRIVATE ${OPENSSL_LIBRARIES})
    target_include_directories (machineid PRIVATE ${OPENSSL_INCLUDE_DIR})
else ()
    target_sources (machineid PRIVATE sha256.c)
endif()

if (WIN32)
else ()
    target_compile_options (machineid PRIVATE
        -std=c89
        -pedantic
        -Wextra
    )
endif()

if (APPLE)
    target_link_libraries (machineid PRIVATE
        "-framework CoreFoundation" "-framework IOKit"
    )
endif()

add_executable (test test.c)

target_link_libraries (test machineid)