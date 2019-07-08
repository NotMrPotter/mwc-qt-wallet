# Use C preprocessor definitions to identify the build target, and generate
# a valid rust toolchain triple for a platform we support.

# Derived from:
# https://github.com/axr/solar-cmake/blob/master/TargetArch.cmake

# Copyright (c) 2012 Petroules Corporation. All rights reserved.

# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:

# Redistributions of source code must retain the above copyright notice, this list of
# conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice, this list
# of conditions and the following disclaimer in the documentation and/or other materials
# provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set(triple_code "
// ARM
#if defined(__arm__)              \
 || defined(__TARGET_ARCH_ARM)    \
 || defined(_M_ARM)               \
 || defined(_M_ARM64)             \
 || defined(__aarch64__)          \
 || defined(__ARM64__)
#  if defined(__ARM64_ARCH_8__)   \
   || defined(__aarch64__)        \
   || defined(__ARMv8__)          \
   || defined(__ARMv8_A__)        \
   || defined(_M_ARM64)
#    if defined(__android__)
#      error rust_triple aarch64-linux-android
#    elif defined(__gnu_linux__)
#      error rust_triple aarch64-unknown-linux-gnu
#    endif
#  elif defined(__ARM_ARCH_7__)   \
     || defined(__ARM_ARCH_7A__)  \
     || defined(__ARM_ARCH_7R__)  \
     || defined(__ARM_ARCH_7M__)  \
     || defined(_ARM_ARCH_7)      \
     || defined(__CORE_CORTEXA__)
#    if defined(__android__)
#      error rust_triple armv7-linux-androideabi
#    elif defined(__gnu_linux__)
#      error rust_triple armv7-unknown-linux-gnueabihf
#    endif
#  else
#    if defined(__android__)
#      error rust_triple arm-linux-androideabi
#    elif defined(__gnu_linux__)
#      if defined(__ARMEL__)
#        error rust_triple arm-unknown-linux-gnueabi
#      else
#        error rust_triple arm-unknown-linux-gnueabihf
#      endif
#    endif
#  endif

// X64
#elif defined(__x86_64)           \
   || defined(__x86_64__)         \
   || defined(__amd64)            \
   || defined(_M_X64)
#  if defined(__APPLE__)
#    error rust_triple x86_64-apple-darwin
#  elif defined(__android__)
#    error rust_triple x86_64-linux-android
#  elif defined(__MINGW32__)
#    error rust_triple x86_64-pc-windows-gnu
#  elif defined(_MSC_VER)
#    error rust_triple x86_64-pc-windows-msvc
#  elif defined(__FreeBSD__)
#    error rust_triple x86_64-unknown-freebsd
#  elif defined(__gnu_linux__)
#    error rust_triple x86_64-unknown-linux-gnu
#  endif

// X86
#elif defined(__i386)             \
   || defined(__i386__)           \
   || defined(_M_IX86)
#  if defined(__APPLE__)
#    error rust_triple i686-apple-darwin
#  elif defined(__android__)
#    error rust_triple i686-linux-android
#  elif defined(__MINGW32__)
#    error rust_triple i686-pc-windows-gnu
#  elif defined(_MSC_VER)
#    error rust_triple i686-pc-windows-msvc
#  elif defined(__gnu_linux__)
#    error rust_triple i686-unknown-linux-gnu
#  endif
#endif

#error rust_triple unsupported-target
")

function(rust_triple var)
    file(WRITE "${CMAKE_BINARY_DIR}/cmake/rust_triple.cpp" "${triple_code}")

    enable_language(C)

    try_run(
        run_result_unused
        compile_result_unused
        "${CMAKE_BINARY_DIR}"
        "${CMAKE_BINARY_DIR}/cmake/rust_triple.cpp"
        COMPILE_OUTPUT_VARIABLE TRIPLE
    )

    string(REGEX MATCH "rust_triple ([a-zA-Z0-9_-]+)" TRIPLE "${TRIPLE}")
    string(REPLACE "rust_triple " "" TRIPLE "${TRIPLE}")

    if (NOT TRIPLE)
        set(TRIPLE CMAKE-ERROR)
    endif()

    set(${var} "${TRIPLE}" PARENT_SCOPE)
endfunction()
