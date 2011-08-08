.. vim: textwidth=80 :

.. _quicksearch:

Quick search
------------

The toolbar contains a quick search line, used to search and replace text in
text editor without opening the search dialog. The quick search line can be
activated using the mouse, or by pressing *Ctrl+/* shortcut.

As you type your search query into quick search input, search is performed
immediately, and first match is highlighted. If no matches found, quick search
line's background is highlighted in yellow. If the search query is invalid,
background is highlighted in red.

Quick search query is composed of few parts:

 * search direction indicator
 * search expression
 * replacement expression
 * search options

The only mandatory part of search request is the search expression, other parts
can be omitted. The table below lists all possible search query forms:

============================================ =============================================================================
Search forward                               ``Search`` or ``/Search``
Search backward                              ``?Search``
Search forward with options                  ``Search/Options`` or ``/Search/Options``
Search backward with options                 ``?Search/Options``
Search forward with replacement              ``Search/Replace/`` or ``/Search/Replace/``
Search backward with replacement             ``?Search/Replace/``
Search forward with replacement and options  ``Search/Replace/Options`` or ``/Search/Replace/Options``
Search backward with replacement and options ``?Search/Replace/Options``
============================================ =============================================================================

Quick search and replace always use regular expression. The syntax of search and
replace expressions is identical to that used in Search/Replace dialog, with one
exception: to use a slash character ``/`` in search or replace expression,
prepend a backslash to it : ``\/``.

Options are composed from one or more characters. Possible characters are:

 * ``i`` -- case insensitive search
 * ``g`` -- find (and replace, if requested) all matches of search expression
 * ``U`` - use "ungreedy" quantifiers
