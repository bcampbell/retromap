# Architecture Notes

Using MVP (Model/View/Presenter).

```

+-----------------+
|       View      |  MapWidget, others?
+-----------------+
   |           ^
   | user      |  UI updates (IView Methods)
   | actions   |
- -|- - - - - -|- - - - - GUI boundary (no QT below here!)
   |           |
   V           |
+-----------------+
|    Presenter    |  MapPresenter, others?
+-----------------+
   |           ^
   |           |
   | Cmd       |  model state changes
   | objects   |
   V           |
+-----------------+
|      Model      |
+-----------------+


```
`Proj` is the raw data structure - maps, charset, palette etc.

Model contains the `Proj` and adds undo/redo stack, filenames, brushes and other global editor state.

Presenter applies changes to model by submitting `Cmd` objects.

MapPresenter is probably the only real complex presenter we'll need, as it's the most interactive.

Suspect other view classes (EntWidget, PaletteWidget etc) could all share a thin, generic presenter class (although currently they are also presenters!).

`Cmd` modifies the `Proj`, then telling the listeners in the `Model` what happened.

View contains as little as possible.




