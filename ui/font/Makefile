all: runes.inc runes.h

runes.inc: generic_5x8.def instrument_25x40.def instrument_9x16.def
	python ./genfont.py data start map font <generic_5x8.def >$@
	python ./genfont.py data body map font <instrument_9x16.def >>$@
	python ./genfont.py data end map font <instrument_25x40.def >>$@

runes.h: generic_5x8.def instrument_25x40.def instrument_9x16.def
	python ./genfont.py inc start map font <generic_5x8.def >$@
	python ./genfont.py inc body map font <instrument_9x16.def >>$@
	python ./genfont.py inc end map font <instrument_25x40.def >>$@
