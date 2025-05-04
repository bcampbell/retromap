# Architecture Notes


## core layer:

`Proj` is the raw data structure - maps, charset, palette etc.

`Model` contains the `Proj` and adds undo/redo stack, filenames, brushes and other global editor state.
`Model` also has list of listeners (`IModelListener`) who will be informed when changes are made.

Model is usually changed by submitting `Cmd` objects.

`Cmd` modifies the `Proj`, then telling the listeners in the `Model` what happened.
`Model` holds a list of applied `Cmd`s, which can be Undone/Redone.

`MapEditor` provides the core functionality for editing a map, and provides hooks for the GUI layer.

## GUI layer:

GUI code is held entirely within the `qt` dir.

`MapWidget` extends `MapEditor` and handles the GUI side of things.

Other widgets are much simpler, and just implement IModelListener.

Some GUI commands create `Cmd` objects and submit them to the `Model`.


