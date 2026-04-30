gtest_dep = dependency('gtest', main: true, disabler: true, required: false)
gmock_dep = dependency('gmock', disabler: true, required: false)
if not gtest_dep.found() or not gmock_dep.found()
    gtest_proj = import('cmake').subproject('googletest', required: false)
    if gtest_proj.found()
        gtest_dep = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ],
        )
        gmock_dep = gtest_proj.dependency('gmock')
    else
        assert(
            not get_option('tests').enabled(),
            'Googletest is required if tests are enabled',
        )
    endif
endif

phosphor_logging_dep = dependency('phosphor-logging')

# nv_tal_includes and shmem are defined in lib/meson.build (parent scope)
# We compile tal.cpp directly alongside stubs instead of linking libtal,
# so only stub implementations are linked (no real shmem/smbus calls).
stub_sources = ['tal_shmem_stub.cpp', 'tal_smbus_stub.cpp']

tests = ['tal_utest']
foreach t : tests
    test(
        'test_' + t.underscorify(),
        executable(
            'test-' + t.underscorify(),
            t + '.cpp',
            stub_sources,
            '../tal.cpp',
            implicit_include_directories: false,
            include_directories: [nv_tal_includes],
            dependencies: [gtest_dep, gmock_dep, shmem, phosphor_logging_dep],
        ),
        workdir: meson.project_source_root(),
    )
endforeach
