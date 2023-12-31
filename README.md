# ИДЗ-4 Коледаев Алексей Дмитриевич | БПИ-213

# Вариант 16 | Отчет

## Условие

На клумбе растет 40 цветов, за ними непрерывно следят два процесс–садовника и поливают увядшие цветы,
при этом оба садовника очень боятся полить один и тот же цветок, который еще не начал вянуть. **Создать приложение, моделирующее состояния цветков на клумбе и действия садовников.** _Сервер используется для обмена информацией между
клиентами садовниками и клиентом — клумбой. Клумба — клиент, отслеживающий состояния всех цветов. Каждый садовник
— отдельный клиент._

## Входные данные

### Server

`server <Порт для получения сообщений> <Адрес мультикаста> <Порт для мультикаста> [<TTL>]`

### Flowers

`flowers <Адрес мультикаста> <Порт мультикаста> <Адрес сервера> <Порт сервера>`

### Gardener

`gardener <id(1 или 2)> <Адрес мультикаста> <Порт мультикаста> <Адрес сервера> <Порт сервера>`

### Watcher

`Watcher <Адрес мультикаста> <Порт мультикаста>`

## Сценарий

Грех был не воспользоваться преимуществом UDP и не сделать мультикаст рассылку. Начнем с порядка запуска.

### Порядок запуска

1. Server
2. Watcher(if needed), можно подключится в любое время, но часть логов до подключения будет потеряна
3. Gardener с id 1 и Gardener с id 2
4. Flowers

### Ход работы

Для начала, стоит сказать, что, не считая распределения нагрузки садовников, сервер выполняет единственную функцию -- рассылка пришедших сообщений в мультикаст.
Далее, с помощью полей структуры сообщений каждый клиент сам фильтрует, ему адресовано сообщение или нет. Садовники отправляют сообщение либо наблюдателю, либо клумбе, клумба -- либо наблюдателю либо садовникам. Логика клумбы проста, отправить случайное кол-во сообщений о вянущих цветах на сервер, дождаться ответа, и изменить состояние, и так до бесконечности. Логика садовника -- тоже довольно простая, его задача отправить сообщение о том, что он приступил к поливу, а после -- подтвердить его окончание. Клиент наблюдатель парсит каждое сообщение приходящее на мультикаст, и в зависимости от содержания полей структуры логирует происходящее, практически дублируя все, что выводят остальные программы. Завершение по SIGINT закрывает сокеты и выходит из программ.

## На 4-5

- [тесты](./4-5/tests/)

## На 6-8

Пишу на 6-8, потому что, благодаря мультикастовой рассылке, не составляет труда подключить множество наблюдателей.

- [тесты](./6-8/tests/)
