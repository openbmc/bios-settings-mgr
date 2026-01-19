project(
    'biosconfig-manager',
    'cpp',
    meson_version: '>=1.1.1',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++23',
        'buildtype=debugoptimized',
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

# Wno-psabi reduces the number of "Note:" messages when cross-compiling some STL
# stuff for ARM. See https://stackoverflow.com/questions/48149323/strange-gcc-warning-when-compiling-qt-project
# Basically, gcc 6 and gcc 7 are not ABI compatible, but since the whole OpenBMC
# project uses the same compiler, we can safely ignmore these info notes.
add_project_arguments('-Wno-psabi', language: 'cpp')

if (get_option('enable-bios-secureboot').allowed())
    add_project_arguments('-DENABLE_BIOS_SECUREBOOT', language: 'cpp')
endif

boost_args = [
    '-DBOOST_ALL_NO_LIB',
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_NO_RTTI',
    '-DBOOST_NO_TYPEID',
    '-DBOOST_SYSTEM_NO_DEPRECATED',
]

deps = [
    dependency('boost'),
    dependency('phosphor-dbus-interfaces'),
    dependency('phosphor-logging'),
    dependency('sdbusplus'),
    dependency('libsystemd'),
    dependency('openssl'),
    dependency('nlohmann_json', include_type: 'system'),
]

cereal = dependency('cereal', required: false)
cpp = meson.get_compiler('cpp')
has_cereal = cpp.has_header_symbol(
    'cereal/cereal.hpp',
    'cereal::specialize',
    dependencies: cereal,
    required: false,
)
if not has_cereal
    cereal_opts = import('cmake').subproject_options()
    cereal_opts.add_cmake_defines(
        {'BUILD_TESTS': 'OFF', 'SKIP_PERFORMANCE_COMPARISON': 'ON'},
    )
    cereal_proj = import('cmake').subproject(
        'cereal',
        options: cereal_opts,
        required: false,
    )
    assert(cereal_proj.found(), 'cereal is required')
    cereal = cereal_proj.dependency('cereal')
endif
deps += cereal

src_files = [
    'src/main.cpp',
    'src/manager.cpp',
    'src/manager_serialize.cpp',
    'src/password.cpp',
    'src/secureboot.cpp',
]

executable(
    'biosconfig-manager',
    src_files,
    implicit_include_directories: true,
    include_directories: ['include'],
    dependencies: deps,
    cpp_args: boost_args,
    install: true,
    install_dir: get_option('bindir'),
)

systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_variable(
    'systemd_system_unit_dir',
    pkgconfig_define: ['prefix', get_option('prefix')],
)

fs = import('fs')
fs.copyfile(
    'service_files/xyz.openbmc_project.biosconfig_manager.service',
    install: true,
    install_dir: systemd_system_unit_dir,
)
