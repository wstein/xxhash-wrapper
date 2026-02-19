project(
  'xxhash-wrapper',
  'c',
  version: '0.8.3.0',
  meson_version: '>= 1.1',
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

# Platform-specific variant compilation per FR-005
# x86/x64: always compile scalar, SSE2, AVX2, AVX512
# aarch64: only compile ARM NEON, SVE (when targeting aarch64)
wrapper_sources = files(
  'src/xxh3_wrapper.c',
  'vendor/xxHash/xxhash.c',
)

# Add x86 variants only on x86_64 builds
if host_machine.cpu_family() == 'x86_64'
  wrapper_sources += files(
    'src/variants/x86/sse2.c',
    'src/variants/x86/avx2.c',
    'src/variants/x86/avx512.c',
  )
  message('Building x86/x64 variants: SSE2, AVX2, AVX512')
endif

# Add ARM variants only on aarch64 builds
if host_machine.cpu_family() == 'aarch64'
  wrapper_sources += files(
    'src/variants/arm/neon.c',
    'src/variants/arm/sve.c',
  )
  message('Building aarch64 variants: NEON, SVE')
endif

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

# Fuzz target: enabled only when the compiler supports -fsanitize=fuzzer,address
# Activate with: meson setup -Db_sanitize=address -Dfuzz=true builddir
option_fuzz = get_option('fuzz')
has_fuzzer = cc.has_argument('-fsanitize=fuzzer')
has_asan   = cc.has_argument('-fsanitize=address')

if option_fuzz and has_fuzzer and has_asan
  executable(
    'fuzz_xxh3',
    'tests/fuzz/fuzz_xxh3.c',
    include_directories: inc,
    dependencies: [xxh3_dep],
    c_args: ['-fsanitize=fuzzer,address'],
    link_args: ['-fsanitize=fuzzer,address'],
  )
elif option_fuzz
  warning('Fuzz target requested but compiler does not support -fsanitize=fuzzer,address; skipping.')
endif
