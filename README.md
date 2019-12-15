# HttpClientLinux
Http Client on Linux  


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
- компилятор: g++
 
////////////////////////////////////////////////

USAGE: curl_ivb link [filename]

example:
./curl_ivb cpk.msu.ru/files/2018/tasks/math.pdf f1.pdf

///////////

BUILD:

./build.sh

RUN TESTS:

./runtests.sh

