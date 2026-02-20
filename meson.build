project(
  'xxhash-wrapper',
  'c',
  version: '0.8.3.0',
  meson_version: '>= 1.1',
  default_options: [
    'c_std=c99',
    'warning_level=2',
    'buildtype=release',  # Default to release for performance
    'b_ndebug=if-release', # Define NDEBUG for release/minsize builds; omit for debug
  ],
)

cc = meson.get_compiler('c')

# Architecture and performance tuning
cpu_family = host_machine.cpu_family()
c_args = []
c_link_args = []

inc = include_directories(
  'include',
  'src',
  'vendor/xxHash',
)

# Shared sources (streaming API, XXH32, XXH64)
# These use a single compilation of xxhash.c
wrapper_sources = files(
  'src/xxh3_wrapper.c',
  'vendor/xxHash/xxhash.c',
)

# Helper for variant libraries
variant_libs = []

# Scalar variant (always compiled)
variant_libs += static_library('xxh3_scalar',
  'src/variants/scalar.c',
  include_directories: inc,
  c_args: ['-DXXH_VECTOR=XXH_SCALAR'],
)

# Add x86 variants only on x86_64 builds
if cpu_family == 'x86_64'
  variant_libs += static_library('xxh3_sse2',
    'src/variants/x86/sse2.c',
    include_directories: inc,
    c_args: ['-DXXH_VECTOR=XXH_SSE2', '-msse2'],
  )
  variant_libs += static_library('xxh3_avx2',
    'src/variants/x86/avx2.c',
    include_directories: inc,
    c_args: ['-DXXH_VECTOR=XXH_AVX2', '-mavx2'],
  )
  variant_libs += static_library('xxh3_avx512',
    'src/variants/x86/avx512.c',
    include_directories: inc,
    c_args: ['-DXXH_VECTOR=XXH_AVX512', '-mavx512f', '-mavx512bw', '-mavx512dq'],
  )
  message('Building x86/x64 variants: SSE2, AVX2, AVX512')
endif

# Add ARM variants only on aarch64 builds
if cpu_family == 'aarch64'
  variant_libs += static_library('xxh3_neon',
    'src/variants/arm/neon.c',
    include_directories: inc,
    c_args: ['-DXXH_VECTOR=XXH_NEON', '-march=armv8.2-a'],
  )
  
  # Always build SVE on aarch64; consumer handles CPU feature detection at runtime
  # (similar to how x86 always exports SSE2/AVX2/AVX512 unconditionally)
  variant_libs += static_library('xxh3_sve',
    'src/variants/arm/sve.c',
    include_directories: inc,
    c_args: ['-DXXH_VECTOR=XXH_SVE', '-march=armv8.2-a+sve'],
  )
  message('Building aarch64 variants: NEON, SVE')
endif

libxxh3_wrapper_shared = library(
  'xxh3_wrapper',
  wrapper_sources,
  include_directories: inc,
  link_whole: variant_libs,
  install: true,
)

libxxh3_wrapper_static = static_library(
  'xxh3_wrapper_static',
  wrapper_sources,
  include_directories: inc,
  link_whole: variant_libs,
  install: true,
)

xxh3_dep = declare_dependency(
  include_directories: inc,
  link_with: libxxh3_wrapper_shared,
)

test_inc = include_directories(
  'tests/unity',
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
  c_args: c_args,
  link_args: c_link_args,
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
