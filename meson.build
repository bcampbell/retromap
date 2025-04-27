project('retromap', 'cpp', default_options: ['cpp_std=c++20'])

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

#incdirs = include_directories('src')

my_headers = [
  'cmd.h',
  'draw.h',
  'model.h',
  'mappresenter.h',
  'view.h',
  'proj.h',
  'tool.h',

  'qt/CharsetWidget.h',
  'qt/EntWidget.h',
  'qt/helpers.h',
  'qt/MainWindow.h',
  'qt/MapExchangeDialog.h',
  'qt/MapSizeDialog.h',
  'qt/MapWidget.h',
  'qt/PaletteWidget.h',
  'qt/PenWidget.h']
  

my_sources = [
  'cmd.cpp',
  'draw.cpp',
  'model.cpp',
  'mappresenter.cpp',
  'proj.cpp',
  'tool.cpp',

  'qt/main.cpp',
  'qt/helpers.cpp',
  'qt/CharsetWidget.cpp',
  'qt/EntWidget.cpp',
  'qt/MainWindow.cpp',
  'qt/MapExchangeDialog.cpp',
  'qt/MapSizeDialog.cpp',
  'qt/MapWidget.cpp',
  'qt/PaletteWidget.cpp',
  'qt/PenWidget.cpp']

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




