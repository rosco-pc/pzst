.. vim: textwidth=80 :

.. _regexp:

Регулярные выражения
--------------------

Регулярное выражение -- это строка-образец, задающая правила поиска. На первый
взгляд использование регулярных выражений может показаться сложным, однако имеет
смысл потратить время на их освоение, так как они являются мощным инструментом
поиска и замены текста.

Синтаксис регулярных выражений
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Регулярное выражение состоит из *символов* и *метасимволов*. Большинство
символов представляют сами себя, за исключение метасимволов

    ``[ ] \ ^ $ . | ? * + ( ) { }``

Если регулярное выражение не содержит метасимволов, то поиск такого выражения
аналогичен поиску простой строки, без использования регулярных выражений.
Метасимволы имеют специальное значение, описанное ниже. Если необходимо указать
в строке поиска символ, имеющий специальное значение, то его нужно предварить
символом обратной косой черты ``\`` . Например ``.`` означает "любой символ",
а ``\.`` -- точку. Если необходимо найти символ обратной косой черты, то
записывают ``\\``.

Любой символ
""""""""""""

Метасимвол ``.`` представляет любой символ, в том числе и символ новой строки.
Например, регулярное выражение ``crow.`` соответствует строкам ``crown`` и
``crowd``.

Символьный класс (набор символов)
"""""""""""""""""""""""""""""""""

Набор символов, заключенный в квадратные скобки, означает, что в этом месте
может находится любой символ из набора. Например, выражению ``sto[nr]e``
соответствуют строки ``stone`` и ``store`` (но не ``stole``).

Метасимвол ``^`` в начале символьного класса инвертирует набор символов -
он будет соответствовать любому символу, за исключением перечисленных. Если
``^`` расположен не в начале набора, то он не имеет специального значения и
представляет сам себя. Например, выражению ``sto[^nr]e``
соответствует строка ``stole``, но не ``store`` или ``stone``, а выражению
``sto[nr^]e`` соответствуют строки ``store``, ``stone`` и ``sto^e``. Также, можно
отменить специальное значение метаcимвола ``^``, если предварить его символом
обратной косой черты ``\`` :  ``sto[\^]e`` соответствует строке ``sto^e``.

Внутри символьных классов символ ``.`` не имеет специального значения, и
представляет сам себя, то есть точку.

Предопределенные символьные классы
""""""""""""""""""""""""""""""""""
Для некоторых часто используемых символьных классов есть сокращенные записи:

``\d``
    цифра (от 0 до 9)
``\D``
    любой символ, кроме цифры
``\w``
    символ, образующий «слово» (буква, цифра или символ подчёркивания)
``\W``
    символ, не образующий «слово»
``\s``
    пробельный символ
``\S``
    непробельный символ

Альтернатива
""""""""""""

Символ вертикальной черты ``|`` означает альтернативу, то есть соответствие
одному из нескольких вариантов. Например, выражение ``sheep|bull|pig``
соответствует строкам ``sheep``, ``bull`` и ``pig``.

Группировка
"""""""""""

Часть регулярного выражения, заключенная в круглые скобки, обрабатывается как
единое целое. Группировка применяется для двух целей:

#. чтобы запомнить часть текста, и использовать его в дальнейшем поиске или
   замене
#. чтобы применить квантификатор (см ниже) не к одному, а сразу к нескольким 
   символам

Часть текста, совпадающая с группой, может быть использована в поиске с помощью
обратных ссылок вида ``\1``, где цифра обозначает порядковый номер группы.
Например, выражение ``([ab])_\1`` соответствует строкам ``a_a`` и ``b_b``, но не
``b_a`` или ``a_b``.

Квантификаторы
""""""""""""""

*Квантификатор* после символа, символьного класса или группы определяет, сколько
раз предшествующее выражение может встречаться. Квантификатор может относиться
более чем к одному символу в регулярном выражении, только если это символьный
класс или группа.

Возможны следующие квантификаторы (``m`` и ``n`` -- числовые значения):

 * ``{n}`` -- ровно ``n`` раз. Например, ``she{2}p`` соответствует строке
   ``sheep``.
 * ``{m, n}`` -- от ``m`` до ``n`` включительно. Например, ``she{2,4}p``
   соответствует строкам ``sheep``, ``sheeep`` и ``sheeeep``.
 * ``{m,}`` -- не менее ``m``. Например, ``she{2,}p``
   соответствует строкам ``sheep``, ``sheeep``, ``sheeeep``, ``sheeeeep`` и т.д.
 * ``{,n}`` -- не более ``n``. Например, ``she{,3}p``
   соответствует строкам ``shp``, ``shep``, ``sheep`` и ``sheeep``.
 * ``*`` -- ноль или более, эквивалентно ``{0,}``. Например, выражение
   ``colou*r`` соответствует строкам ``color``, ``colour``, ``colouur``  и т.д.
 * ``+`` -- один или более, эквивалентно ``{1,}``. Например, выражение
   ``colou+r`` соответствует строкам ``colour``, ``colouur``, ``colouuur``  и т.д.
 * ``?`` -- ноль или один, эквивалентно ``{0,1}``. Например, выражение
   ``colou?r`` соответствует строкам ``color`` и ``colour``.

Квантификаторы могут использоваться в двух режимах -- "жадном" и "нежадном" (или
"ленивом"). В "жадном" режиме квантификатор стремится захватить как можно больше
символов текста, в "нежадном" -- как можно меньше. Например при поиске в тексте
``maaaaaaaaaaaaaaaaaa`` с помощью выражения ``ma+``, в "жадном" режиме будет
найдено ``maaaaaaaaaaaaaaaaaa``, а в "нежадном" -- ``ma``. Для задания режима
квантификаторов используется флаг **"Нежадные" регулярные выражения** в  
:ref:`диалоге Поиск/Замена <figure_find>`.

Особенности реализации
""""""""""""""""""""""

Реализация регулярных выражений в PZST имеет несколько особенностей:

 * метасивол ``.`` соответствует в том числе и символу новой строки
 * метасимволы ``^`` и ``$`` соответствуют началу и концу всего текста, а не
   отдельной строки текста

Строка замены
^^^^^^^^^^^^^

При использовании регулярных выражений для поиска, в строке замены могут
использоваться специальные символы, представляющие части найденного текста или
изменяющие текст.

 * ``\t`` -- символ табуляции
 * ``\n`` -- символ новой строки
 * ``\1`` -- обратная ссылка. Соответствует части найденного текста, захваченной
   группой. Цифра задает номер группы, ``\0`` соответствует всему найденному
   тексту.
 * ``\l`` -- переводит один следующий символ в нижний регистр
 * ``\u`` -- переводит один следующий символ в верхний регистр
 * ``\L`` -- переводит все последующие символы в нижний регистр до конца строки,
   или пока не встретится ``\e`` или ``\E``
 * ``\U`` -- переводит все последующие символы в верхний регистр до конца строки,
   или пока не встретится ``\e`` или ``\E``
 * ``\e``, ``\E`` -- отменяет изменение регистра
 * ``\\`` -- символ ``\``

Примеры использования метасимволов в строке замены (предполагается, что поиск
ведется без учета регистра)

===================== ==================== ========================  ========================
Исходный текст        Строка поиска        Строка замены             Текст после замены
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

