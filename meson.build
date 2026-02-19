project(
  'xxhash-wrapper',
  'c',
  version: '0.8.3.0',
  default_options: [
    'c_std=c99',
    'warning_level=2',
  ],
)

cc = meson.get_compiler('c')

inc = include_directories(
  'include',
  'src',
  'vendor/xxHash',
)

test_inc = include_directories(
  'tests/unity',
)

wrapper_sources = files(
  'src/xxh3_wrapper.c',
  'src/variants/x86/sse2.c',
  'src/variants/x86/avx2.c',
  'src/variants/x86/avx512.c',
  'src/variants/arm/neon.c',
  'src/variants/arm/sve.c',
  'vendor/xxHash/xxhash.c',
)

libxxh3_wrapper_shared = library(
  'xxh3_wrapper',
  wrapper_sources,
  include_directories: inc,
  install: true,
)

libxxh3_wrapper_static = static_library(
  'xxh3_wrapper_static',
  wrapper_sources,
  include_directories: inc,
  install: true,
)

xxh3_dep = declare_dependency(
  include_directories: inc,
  link_with: libxxh3_wrapper_shared,
)

install_headers('include/xxh3.h')

test_exe = executable(
  'test_variants',
  'tests/unit/test_variants.c',
  'tests/unity/unity.c',
  include_directories: [inc, test_inc],
  dependencies: [xxh3_dep],
)
add_test_setup('unity',
  env: ['UNITY_CONFIG=unity.config'],
)
test('unit-variants', test_exe)

bench_exe = executable(
  'bench_variants',
  'tests/bench/bench_variants.c',
  include_directories: inc,
  dependencies: [xxh3_dep],
)

if false
  executable(
    'fuzz_xxh3',
    'tests/fuzz/fuzz_xxh3.c',
    include_directories: inc,
    dependencies: [xxh3_dep],
    c_args: ['-fsanitize=fuzzer,address'],
    link_args: ['-fsanitize=fuzzer,address'],
  )
endif
