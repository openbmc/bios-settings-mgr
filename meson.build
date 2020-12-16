project(
    'biosconfig-manager',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++17'
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

# Wno-psabi reduces the number of "Note:" messages when cross-compiling some STL
# stuff for ARM. See https://stackoverflow.com/questions/48149323/strange-gcc-warning-when-compiling-qt-project
# Basically, gcc 6 and gcc 7 are not ABI compatible, but since the whole OpenBMC
# project uses the same compiler, we can safely ignmore these info notes.
add_project_arguments('-Wno-psabi', language: 'cpp')

conf_data = configuration_data()
conf_data.set_quoted('BIOS_PERSIST_PATH', get_option('bios-persist-path'))
configure_file(output: 'config.h', configuration: conf_data)

boost_args = ['-DBOOST_ALL_NO_LIB',
              '-DBOOST_ASIO_DISABLE_THREADS',
              '-DBOOST_ERROR_CODE_HEADER_ONLY',
              '-DBOOST_NO_RTTI',
              '-DBOOST_NO_TYPEID',
              '-DBOOST_SYSTEM_NO_DEPRECATED']

deps = [dependency('boost'),
        dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('systemd'),
]

executable('biosconfig-manager',
           'src/manager.cpp',
           'src/manager_serialize.cpp',
        implicit_include_directories: true,
        include_directories: ['include'],
        dependencies: deps,
        cpp_args : boost_args,
        install: true,
        install_dir: get_option('bindir'))

executable('biosconfig-password',
           'src/password.cpp',
        implicit_include_directories: true,
        include_directories: ['include'],
        dependencies: deps,
        cpp_args : boost_args,
        install: true,
        install_dir: get_option('bindir'))

systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])

configure_file(
    copy: true,
    input: 'service_files/xyz.openbmc_project.biosconfig_manager.service',
    install: true,
    install_dir: systemd_system_unit_dir,
    output: 'xyz.openbmc_project.biosconfig_manager.service'
)

configure_file(
    copy: true,
    input: 'service_files/xyz.openbmc_project.biosconfig_password.service',
    install: true,
    install_dir: systemd_system_unit_dir,
    output: 'xyz.openbmc_project.biosconfig_password.service'
)