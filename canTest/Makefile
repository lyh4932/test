TARGET    :=    can_test

COBJS     +=    can_test.o

CC        :=    arm-none-linux-gnueabi-gcc

CFLAGS    +=    -O2
CFLAGS    +=    -I../inc
CFLAGS    +=    -lpthread
CFLAGS    +=    -DPF_CAN=29 
CFLAGS    +=    -DAF_CAN=PF_CAN
#CFLAGS    +=    -g
LDLIBS	+=	-static  
LDLIBS	+=	-L.
LDLIBS	+=	-lsocketcan


all:$(TARGET)
$(TARGET):$(COBJS)
	$(CC) $^ $(LDLIBS) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) -c $< 
	
clean:
	rm -f $(TARGET) $(COBJS)

