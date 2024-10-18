project('retromap', 'cpp', default_options: ['cpp_std=c++20'])

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

#incdirs = include_directories('src')

my_headers = [
  'cmd.h',
  'charsetwidget.h',
  'editor.h',
  'helpers.h',
  'mainwindow.h',
  'mapeditor.h',
  'mapsizedialog.h',
  'mapwidget.h',
  'palettewidget.h',
  'penwidget.h',
  'proj.h',
  'tool.h']

my_sources = [
  'charsetwidget.cpp',
  'cmd.cpp',
  'editor.cpp',
  'helpers.cpp',
  'main.cpp',
  'mainwindow.cpp',
  'mapeditor.cpp',
  'mapsizedialog.cpp',
  'mapwidget.cpp',
  'palettewidget.cpp',
  'penwidget.cpp',
  'proj.cpp',
  'tool.cpp']

#qt_resources = ['resources.qrc']

moc_files = qt6.preprocess(moc_headers: my_headers,
#  qresources : qt_resources,
  dependencies: qt6_dep)


executable('retromap',
  sources: [my_sources, moc_files],
  #  include_directories: incdirs,
  dependencies : [qt6_dep], #, png_dep, gif_dep, jpeg_dep],
  win_subsystem: 'windows',
  install: true)




