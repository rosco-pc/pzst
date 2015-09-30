Summary of what I intend to have in first version:

  * cross-platform application, written in Qt
  * open source
  * application UI localization

  * Editor:
    * syntax highlighting
    * regular expression search/replace
    * minimalistic code browser (quickly jump to method definition)

  * Compiler/loader:
    * serial port to use is specified explicitly (I do not like the loader to open all ports available and sending data - I may have whatever connected there)
    * reliable operation with CP2102 USB/UART converter under Linux
    * use of any character encoding for string constants, not only Latin-1

  * Serial terminal:
    * text/hex view
    * logging
    * macros
    * can send as text, text with special chars (C-style \r, \n etc) and hex
    * support for different 8-bit encodings

  * Project manager:
    * project-specific compiler options
    * can add non-spin files to project for easy archive creation (SPIN sources + files to be put on SD card, for example)


Further plans (not sure about these, need to evaluate):

  * code completion (type "serial." and get dropdown list with methods from FullDuplexSerial)
  * simulator
  * debugger


![http://pzst.googlecode.com/files/shot1.png](http://pzst.googlecode.com/files/shot1.png)