CC = arm-linux-gcc
AR = ar -r
RL = ranlib

GAME_SRCS = game_logic
IMG_SRCS = img_to_arr
TS_SRCS = ts_input
GAME_LIB = liblogic.a
IMG_LIB = libimg.a
TS_LIB = libts.a

all: game img ts 

game:
	$(CC) -c $(GAME_SRCS).c
	$(AR) $(GAME_LIB) $(GAME_SRCS).o
	$(RL) $(GAME_LIB) 
img:
	$(CC) -c $(IMG_SRCS).c
	$(AR) $(IMG_LIB) $(IMG_SRCS).o
	$(RL) $(IMG_LIB)
ts:
	$(CC) -c $(TS_SRCS).c
	$(AR) $(TS_LIB) $(TS_SRCS).o
	$(RL) $(TS_LIB)
clean:
	rm -rf *.o *.a
