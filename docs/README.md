# Документация DRX

В этой директории находятся файлы, документирующие Формат Модуля DRX и DRX
CMake API.

Сами модули DRX можно найти в поддиректории `modules` этого репозитория DRX.

Примеры с проектами для CMake находятся в директори `examples/CMake`.

Сам DRX API документирован inline, но из исходного кода можно генерировать докуметы HTML, используя инструмент `doxygen`. Эти документы HTML можно найти[online](https://DRX.com/learn/documentation), либо сгенерировать локальную копию, которой можно будет пользоваться без подключения к интернету. Инструкции
по генерированию внелинейной документации приведены ниже.

# Генерирование Offline Документации HTML

## Зависимости

- doxygen
- python
- make
- graphviz (для генерации диаграмм наследования)

Make sure that all the dependencies can be found on your PATH.

## Построение

- cd into the `doxygen` directory on the command line
- run `make`

Doxygen will create a new subdirectory "doc". Open doc/index.html in your browser
to access the generated HTML documentation.
