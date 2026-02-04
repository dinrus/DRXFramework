SHELL := /bin/bash

SOURCE_FILES := $(shell find ../../modules -type f -name "drx_*.h" -or -name "drx_*.dox" | sed 's/ /\\ /g')

.PHONY: clean

doc/index.html: build/drx_modules.dox Doxyfile
	doxygen

build/drx_modules.dox: process_source_files.py $(SOURCE_FILES)
	python $< ../../modules build

clean:
	rm -rf build doc

