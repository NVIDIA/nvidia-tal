project('TAL', 'cpp',
    meson_version: '>= 0.62.2',
    default_options: [
      'buildtype=debugoptimized',
      'cpp_std=c++20',
      'warning_level=3',
      'werror=true',
    ],
    version: '1.0',
)


if not get_option('smbus-telemetry-target').disabled()
    add_project_arguments('-DENABLE_SMBUS', language : 'cpp')
endif

add_project_arguments('-Wno-psabi', language: 'cpp')
add_project_arguments('-Wno-unused-parameter', language: 'cpp')
package_datadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())


if not get_option('smbus-telemetry-target').disabled()
  subdir('smbus-telemetry-target')
  if not get_option('libsmbus-test').disabled()
    subdir('smbus-telemetry-target/test')
  endif
endif

if not get_option('nvidia-tal').disabled()
  subdir('lib')
endif
