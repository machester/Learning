TARGET	= xfm10213_i2c

CCPREFIX =
CCFLAGS = -g -Wall -DDATE=\"$(shell date +%Y-%m-%d_%H:%M:%S)\"
#CCFLAGS = -g -w

LIBS = 
INCS = 

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
#OBJS := $(subst src,obj,$(patsubst %.c,%.o,$(SRCS)))

$(TARGET) : $(OBJS)
	@$(CCPREFIX)gcc $(CCFLAGS) $^ $(LIBS) -o $@

$(OBJS) : $(SRCS)
	@$(CCPREFIX)gcc -c $(CCFLAGS) $(INCS) $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET)

.PHONY:clean
