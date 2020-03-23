ifeq ($(shell uname -s), Darwin)
	FTDI = $(shell brew --prefix libftdi || echo /opt/local)/include/libftdi1
else
	FTDI = /usr/include/libftdi1
endif

IDIR = inc
ODIR = obj
CDIR = src
BDIR = bin

CFLAGS = -Wall -Wextra -Werror -std=gnu99 -O3 -fPIC -I $(FTDI) -I $(IDIR)

_INC = clock.h timespec.h utils.h libentstream.h entstream.h
INC = $(patsubst %,$(IDIR)/%,$(_INC))

_OBJ = clock.o timespec.o utils.o libentstream.o entstream.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: dirs entstream

entstream: $(OBJ)
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $^ -lftdi1 -lzmq -lczmq -lpopt -lm -L.

$(ODIR)/%.o: $(CDIR)/%.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: dirs clean install uninstall

dirs:
	@mkdir -p ${ODIR} ${BDIR}

clean:
	@rm -rf ${ODIR} ${BDIR}

install:
	@cp ${BDIR}/entstream /usr/local/bin/

uninstall:
	@rm /usr/local/bin/entstream