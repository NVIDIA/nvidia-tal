nv_tal_includes = include_directories('include', '.')

subdir('include')

nv_tal_gen = []

# Add source files
sources = ['tal.cpp', 'tal_shmem.cpp', 'tal_smbus.cpp',
]

shmem = dependency('nvidia-shmem')
nv_tal_deps = [
  shmem
]

if not get_option('smbus-telemetry-target').disabled()
  nv_tal_deps += [smbus_slave_telemetry_dep]
endif
# Create library
libtal = static_library(
  'tal', sources, 
  include_directories : nv_tal_includes,
  dependencies: [
    nv_tal_deps,
    ],
  install: true
  )

nv_tal_reqs = []
foreach dep : nv_tal_deps
    if dep.type_name() == 'pkgconfig'
        nv_tal_reqs += dep
    endif
endforeach

import('pkgconfig').generate(
  name: 'nvidia-tal',
  description: 'Nvidia TAL Library',
  version: meson.project_version(),
  requires: nv_tal_reqs,
  libraries: libtal)

nv_tal_dep = declare_dependency(
    include_directories: [nv_tal_includes],
    link_with: libtal,
    sources: nv_tal_gen,
    dependencies: nv_tal_deps,
)

