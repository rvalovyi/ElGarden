CC=gcc

TARGET_SERVER = server

CFLAGS = -g -Wall -I.
CFLAGS += $(shell pkg-config --cflags json-c) 
LDFLAGS += $(shell pkg-config --libs json-c )
LDFLAGS += -lbsd -lpthread
SRC_SERVER := server.c

all: $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_CALC)

$(TARGET_SERVER):
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SRC_SERVER) $(LDFLAGS) 

.PHONY: clean

clean:
	$(RM) -f $(TARGET_SERVER) 
