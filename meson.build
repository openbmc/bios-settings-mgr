project('bios-settings-mgr', ['cpp'],
        version: '0.1', meson_version: '>=0.53.2',
        default_options: [
          'warning_level=3',
          'default_library=shared',
          'werror=true',
          'cpp_std=c++17'
        ])

deps = [
  dependency('sdbusplus'),
  dependency('phosphor-dbus-interfaces')
]

executable(
  'bios-configd',
  'main.cpp',
  'manager.cpp',
  implicit_include_directories: false,
  dependencies: deps,
  install: true,
  install_dir: get_option('bindir'))

systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])
configure_file(
  copy: true,
  input: 'bios-configd.service',
  install: true,
  install_dir: systemd_system_unit_dir,
  output: 'bios-configd.service',
)
