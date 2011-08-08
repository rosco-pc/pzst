.. vim: textwidth=80 :

Menu system
------------

**File** menu
^^^^^^^^^^^^^

 * **New** - creates a new editor tab with empty file
 * **Open** - opens file from disk. After choosing a file,
   a new tab is created. If the file selected is already opened, existing tab is
   activated instead of creating a new one.
 * **Recent files** - opens a submenu with list of recently edited files.
 * **Save** - saves contents of editor in current tab to disk file. If the file
   was not yet saved since it has been created, a dialog appears to chhose file
   name for saving.
 * **Save as** - saves contents of editor in current tab to disk under a
   diffrenet file name
 * **Close** - Closes current tab. If editor contents was modified since last
   save, the software suggests to save the file.
 * **Close all** - Closes all open tabs. If editor contents was modified since last
   save, the software suggests to save the file(s).
 * **Print** - print text in current tab to printer
 * **Quit** - closes the program. If there are tabs with unsaved files, the
   software suggests to save the files before terminating.
 
**Edit** menu
^^^^^^^^^^^^^
 * **Undo** - undo last editing action in current editor tab. Each tab holds
   undo history until the tab is closed
 * **Redo** - redo tast undone editing action
 * **Cut** - cuts selected text to clipboard
 * **Copy** - copies selected text to clipboard
 * **Paste** - pastes text from clipboard into cursor position
 * **Search/Replace** - opens search/replace dialog. For more information about
   searching, see :ref:`searching`
 * **Find next** - repeats last search operation
 * **Replace and find next** - replaces selected text with replace string entered
   in search dialog, and continues searching
 * **Quick search** - Activates quick search line. For more information about
   quick search, see :ref:`quicksearch`
 * **Autocomplete** - activates autocompletion function. See :ref:`autocomplete`
 * **Call tip** - activates call tip. See :ref:`calltips`
 * **Preferences** - opens PZST Preferences dialog. See :ref:`settings`

**View** menu
^^^^^^^^^^^^^
 * **Encrease font size** - increase editor's font size
 * **Decrease font size** - decrease editor's font size
 * **Errors**, **Compiler information**, **Character table**, **Search
   results** - show or hide widgets

**Compile** menu
^^^^^^^^^^^^^^^^
Before using most commands from this menu, you need to connect the Propeller to
your computer's serial port, and select the port in :ref:`Preferences <settings>`
dialog. See :ref:`compilation` section for details about compilation process.

 * **Detect Propeller**  - detects if Propeller is tonnected to serial port
   specified in program preferences
 * **Compile** - compiles current file
 * **Compile and load RAM** - compiles current file, and loads compiled program
   to Propeller's RAM
 * **Compile and load EEPROM** - compiles current file, and loads compiled program
   to EEPROM
 * **Compile and save BINARY file** - compiles current file, and saves compiled
   program into BINARY file. File name is requested in file dialog
 * **Compile and save EEPROM file** - compiles current file, and saves compiled
   program into EEPROM file. File name is requested in file dialog

**Window** menu
^^^^^^^^^^^^^^^
This menu contains list of open files. Selecting an item from this menu
activates corresponding editor's tab.
