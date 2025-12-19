
libs := -lm -ggdb -Wall -Wextra
buildConf := -O1 -g

build := ./build

main:
	@echo Building Main


buildCny:
	@echo Building cny
	$(CC) $(libs) $(buildConf) -c -o $(build)/interp.o ./core/interp.c
