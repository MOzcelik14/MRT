project(
  'mrt',
  'c',
  version: '0.2.0',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ]
)

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)

inc_dir = include_directories('src')

mrt_sources = files(
  'src/token.c',
  'src/error.c',
  'src/lexer.c',
  'src/ast.c',
  'src/parser.c',
  'src/array.c',
  'src/map.c',
  'src/value.c',
  'src/environment.c',
  'src/builtin.c',
  'src/interpreter.c',
  'src/formatter.c',
  'src/inspector.c',
)

mrt_lib = static_library(
  'mrt_core',
  mrt_sources,
  include_directories: inc_dir,
  dependencies: [m_dep],
)

mrt_exe = executable(
  'mrt',
  files('src/main.c'),
  include_directories: inc_dir,
  link_with: mrt_lib,
  dependencies: [m_dep],
  install: true,
)

test_exe = executable(
  'mrt_test',
  files('tests/test_main.c'),
  include_directories: inc_dir,
  link_with: mrt_lib,
  dependencies: [m_dep],
)

test('mrt_test_suite', test_exe)

if get_option('install_examples')
  install_subdir('examples', install_dir: get_option('datadir') / 'mrt')
endif

if get_option('build_studio')
  gtk_dep = dependency('gtk4', required: true)
  gtksource_dep = dependency('gtksourceview-5', required: true)
  glib_dep = dependency('glib-2.0', required: true)
  gio_dep = dependency('gio-2.0', required: true)

  studio_inc = include_directories('mrt-studio/src')
  studio_sources = files(
    'mrt-studio/src/application.c',
    'mrt-studio/src/editor.c',
    'mrt-studio/src/i18n.c',
    'mrt-studio/src/main.c',
    'mrt-studio/src/outline.c',
    'mrt-studio/src/output.c',
    'mrt-studio/src/palette.c',
    'mrt-studio/src/project.c',
    'mrt-studio/src/quickopen.c',
    'mrt-studio/src/runner.c',
    'mrt-studio/src/search.c',
    'mrt-studio/src/session.c',
    'mrt-studio/src/settings.c',
    'mrt-studio/src/statusbar.c',
    'mrt-studio/src/welcome.c',
    'mrt-studio/src/window.c',
  )

  executable(
    'mrt-studio',
    studio_sources,
    include_directories: studio_inc,
    dependencies: [gtk_dep, gtksource_dep, glib_dep, gio_dep, m_dep],
    install: true,
  )
endif
