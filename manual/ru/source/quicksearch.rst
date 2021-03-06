.. vim: textwidth=80 :

.. _quicksearch:

Быстрый поиск
-------------

На панели инструментов расположена строка быстрого поиска, которая позволяет
вести поиск и замену без открытия диалога. Активизировать строку поиска можно с
помощью мыши, или нажав комбинацию клавиш *Ctrl+/*.

По мере ввода поискового запроса в строку поиска, сразу осуществляется поиск в
текущем документе, и найденный текст выделяется. Если текст не найден, фон
строки быстрого поиска подсвечивается желтым цветом. Если допущена ошибка в
синтаксисе запроса, фон строки быстрого поиска подсвечивается красным цветом.

Поисковый запрос состоит из нескольких частей:

 * указатель направления поиска
 * строка поиска
 * строка замены
 * опции поиска

Обязательной является только строка поиска, остальные части можно опустить. В
таблице приведены все возможные варианты запросов:

=========================================== =============================================================================
Только поиск вперед                         ``СтрокаПоиска`` или ``/СтрокаПоиска``
Только поиск назад                          ``?СтрокаПоиска``
Поиск вперед с опциями                      ``СтрокаПоиска/Опции`` или ``/СтрокаПоиска/Опции``
Поиск назад  с опциями                      ``?СтрокаПоиска/Опции``
Поиск вперед с заменой                      ``СтрокаПоиска/СтрокаЗамены/`` или ``/СтрокаПоиска/СтрокаЗамены/``
Поиск назад  с заменой                      ``?СтрокаПоиска/СтрокаЗамены/``
Поиск вперед с заменой и опциями            ``СтрокаПоиска/СтрокаЗамены/Опции`` или ``/СтрокаПоиска/СтрокаЗамены/Опции``
Поиск назад  с заменой и опциями            ``?СтрокаПоиска/СтрокаЗамены/Опции``
=========================================== =============================================================================

В быстром поиске и замене всегда используются регулярные выражения.
Синтаксис строки поиска и замены аналогичен используемому в диалоге поиска, с
небольшим дополнением: если строка поиска или замены содержит символ ``/``, то
его нужно предварить символом ``\``.

Опции состоят из одного или нескольких символов. Допустимы следующие символы:

 * ``i`` -- поиск без учета регистра символов. Если эта опция не указана, поиск
   ведется с учетом регистра
 * ``g`` -- найти (и при необходимости заменить) все вхождения строки поиска в
   текст документа
 * ``U`` - использовать "нежадные" квантификаторы
