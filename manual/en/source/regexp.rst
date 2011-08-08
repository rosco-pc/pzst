.. vim: textwidth=80 :

.. _regexp:

Regular expressions
-------------------

Regular expression are text patterns which define search rules. At first glance,
the use of regular expressions can be complicated, but it makes sense to spend time
in their development because they are a powerful tool for searching and replacing text.

Regular expressions syntax
^^^^^^^^^^^^^^^^^^^^^^^^^^

A regular expression is composed from *characters* and *metacharacters*. 
Most characters represent themselves, except these metacharacters:

    ``[ ] \ ^ $ . | ? * + ( ) { }``

If a regular expression contains no metacharacters, searching for such expression
is identical to searching for literal text, without regular expressions usage.
Metacharacters have special meaning, described later. If you need to include a
literal of metacharacter, prepend a backslash ``\\`` to it. For example, ``.``
means "any character", but ``\.`` means a dot character. A literal backslash is
represented by double backslash: ``\\``.

Any character
"""""""""""""

``.``  metacharacter represents any character, including newline.
For example, ``crow.`` matches ``crown`` and ``crowd``.

Character class (set of characters)
"""""""""""""""""""""""""""""""""""

A set of characters in square brackets matches any character from the set. For
example, ``sto[nr]e`` matches ``stone`` and ``store`` (but not ``stole``).

``^`` metacharacter at the beginning of character class inverts the class. An
inverted character class matches any character except those included in the
class. If ``^`` metacharacter is located not at the beginning of the class, 
then it loses its special meaning and represents itself. For example, ``sto[^nr]e``
matches ``stole``, but not ``store`` or ``stone``, and ``sto[nr^]e`` matches
``store``, ``stone`` and ``sto^e``. Also, special meaning of ``^`` can be
cancelled by a backslash ``\`` :  ``sto[\^]e`` matches ``sto^e``.

Inside character classes, ``.`` metacharacter has no special meaning, and
represents itself.

Predefined character classes
""""""""""""""""""""""""""""

Some common character classes have abbriveated forms:

``\d``
    digit (0 to 9)
``\D``
    any character, except digits
``\w``
    a «word» character (letter, digit of underscore)
``\W``
    a non-«word» character
``\s``
    whitespace character
``\S``
    non-whitespace character

Alternation
"""""""""""

Vertical line character ``|`` means alternation. For example, ``sheep|bull|pig``
matches ``sheep``, ``bull`` and ``pig``.

Grouping
""""""""

A part of regular expression inside round brackets is processed as a whole.
Grouping is used for two purposes:

#. to "remember" a part of matched text, and use it in further search or replace
#. to apply a quantifier (see below) to multiple characters

Part of text matching a group can be used in search by means of *backreferences*
like ``\1``, where digit represents group number. For example, ``([ab])_\1``
matches ``a_a`` and ``b_b``, but not ``b_a`` or ``a_b``.

Quantifiers
"""""""""""

A *quantifiers* after a character, character class or a group defines how many
times the preceding expression can occur. Quantifier can apply to more than one
character only if it follows a character class or a group.

The following quantifiers are possible (``m`` and ``n`` -- numeric values):

 * ``{n}`` -- exactly ``n`` times. For example, ``she{2}p`` matches ``sheep``.
 * ``{m, n}`` -- ``m`` to ``n`` times inclusive. For example, ``she{2,4}p``
   matches ``sheep``, ``sheeep`` and ``sheeeep``.
 * ``{m,}`` -- not less than ``m`` times. For example, ``she{2,}p``
   matches ``sheep``, ``sheeep``, ``sheeeep``, ``sheeeeep`` etc.
 * ``{,n}`` -- not more than ``n``. For example, ``she{,3}p``
   matches ``shp``, ``shep``, ``sheep`` and ``sheeep``.
 * ``*`` -- zero or more times, identical to ``{0,}``. For example
   ``colou*r`` matches ``color``, ``colour``, ``colouur``  etc.
 * ``+`` -- one or more times, identical to ``{1,}``. For example,
   ``colou+r`` matches ``colour``, ``colouur``, ``colouuur``  etc.
 * ``?`` -- zero or one time, identical to ``{0,1}``. For example,
   ``colou?r`` matches ``color`` and ``colour``.

Quantifiers can be used in two modes -- "greedy" and "ungreedy" (or "lazy").
In "greedy" mode, quantifier matches as many characters of source text as
possible.
In "ungreedy" mode, quantifier matches as few characters of source text as
possible.
For example, when searching in text ``maaaaaaaaaaaaaaaaaa`` using regular
expression ``ma+``, "greedy" mode finds ``maaaaaaaaaaaaaaaaaa``, 
and "ungreedy" mode finds ``ma``. Greedyness of quantifiers can be specified
with **"Ungreedy" regular expressions** option in :ref:`Search/Replace dialog <figure_find>`.

Implementation details
""""""""""""""""""""""

Regular expressions implementation in PZST has few traits:

 * ``.`` metacharacter matches newline
 * ``^`` and ``$`` metacharacters match the beginning and the end of whole text,
   not a single line of text

Replacement string
^^^^^^^^^^^^^^^^^^

When using regular expressions for searching, replacement text can contain
special characters representing parts of matched text, or modifying the text.

 * ``\t`` -- tabulation character
 * ``\n`` -- newline character
 * ``\1`` -- backreference. Corresponds to a part of matched text inside a group
   Digit defines group number, ``\0`` represents complete match.
 * ``\l`` -- translates one character to lower case
 * ``\u`` -- translates one character to upper case
 * ``\L`` -- translates all following characters to lower case, until ``\e`` or ``\E``
   is met
 * ``\L`` -- translates all following characters to upper case, until ``\e`` or ``\E``
   is met
 * ``\e``, ``\E`` -- cancels case change
 * ``\\`` -- ``\`` character

Exemples of metacharacters in replacement text (it is supposed that search is
case-insensitive):

===================== ==================== ========================  ========================
Source text           Search string        Replacement string         Text after replacement
===================== ==================== ========================  ========================
``AB``                ``([AB])([AB])``     ``\1 \\ \2``              ``A \ B``
``AB``                ``([AB])([AB])``     ``\2 and \1``             ``B and A``
``Start``             ``(start|stop)``     ``\l\1``                  ``start``
``stoP``              ``(start|stop)``     ``\l\1``                  ``stoP``
``stoP``              ``(start|stop)``     ``\L\1``                  ``stop``
``stoP``              ``(start|stop)``     ``\u\1``                  ``StoP``
``stoP``              ``(start|stop)``     ``\U\1``                  ``STOP``
``Start or stop``     ``(start)(.*)``      ``\U\1\E\2``              ``START or stop``
``Start or stop``     ``.*``               ``\U\0``                  ``START OR STOP``
===================== ==================== ========================  ========================

