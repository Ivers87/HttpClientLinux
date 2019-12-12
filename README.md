# HttpClientLinux
Http Client on Linux  // комментарий на русском


Сделано :
Скачивание файлов через GET запрос, для случаев
- Content-Length известен заранее
- Content-Length не известен заранее, заголовок Trasfer-Encoding: chunked (html-файлы)
- тестирование посредством bash-скрипта

Не сделано :
- HTTPS
- запросы с login/pass
- управление логированием


API:
- unix sockets: send/recv
- для парсинга Header-ов: sreambufs
- regex - отсутствует

 
