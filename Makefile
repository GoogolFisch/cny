
libs := -lm -ggdb -Wall -Wextra
buildConf := -O1 -g

build := ./build

main:
	@echo Building Main


buildCny: $(build)/logger.o
	@echo Building cny
	$(CC) $(libs) $(buildConf) -c -o $(build)/interp.o ./core/interp.c
	$(CC) $(libs) $(buildConf) -c -o $(build)/scope.o ./core/scope.c

logging:
	@echo Buiding logger
	$(CC) $(libd) $(buildConf) -c -o $(build)/logging.o ./core/logging.c
