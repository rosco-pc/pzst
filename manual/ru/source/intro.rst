.. vim: textwidth=80 :
Введение
========

Программа Propeller Zone Spin Tool (сокращенно PZST) предназначена для
разработки программ на языках SPIN и PASM (Propeller Assembler) для
микроконтроллера Propeller, выпускаемого компанией `Parallax, Inc.
<http://www.parallax.com>`_

PZST включает в себя редактор исходного кода и загрузчик программ. Для
компиляции используется оптимизирующий компилятор
`bstc <http://www.fnarfbargle.com/bst.html>`_.

По функциональности PZST  подобен программам 
`Propeller/Spin Tool <http://www.parallax.com/tabid/832/Default.aspx#Software>`_ и
`bst <http://www.fnarfbargle.com/bst.html>`_, с несколькими существенными
отличиями: 
    * в отличии от Propeller/Spin Tool и  bst, PZST поддерживает работу со
      строками в различных национальных кодировках. Propeller/Spin Tool и
      bst работают только со строками в кодировке Latin-1 (западноевропейская)
    * PZST  корректно работает с преобразователями USB-serial на основе
      микросхем CP210x. bst ненадежно работает с CP210x под Linux при
      использовании ядра старше 2.6.30
    * дополнительные возможности редактора кода, такие как автозавершение,
      подсказки и быстрый поиск, делают работу более продуктивной
В настоящее время, PZST работает под операционными системами Microsoft Windows
(Windows 2000 и старше), Linux и Mac OS X.


