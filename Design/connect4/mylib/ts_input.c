#include "ts_header.h"


//read touched coordinate
int ts_read(int *xcrd, int *ycrd) {
    //structure to save touch value
    struct input_event evt;

    int ts_fd = open(EVDEVFILE, O_RDWR);
    if(ts_fd < 0){
	return -2;
    }

    while(1) {
	//read touched info
	read(ts_fd, &evt, sizeof(evt));
	if(evt.type == EV_ABS) {
	    //if it is x coordinate info
	    if(evt.code == ABS_MT_POSITION_X){
		*xcrd= evt.value;
	    }
	    //if it is y coordinate info
	    else if(evt.code == ABS_MT_POSITION_Y) {
		*ycrd = evt.value;
		//reading done
		close(ts_fd);
		return 0;
	    }
	}
    }
    close(ts_fd);
    return -1;
}

//with x coordinate, return x position in board
int get_colnum(int xcrd) {
    if (xcrd > 120 && xcrd < 680)
	return (xcrd - 120) / 80;
    else
	return -1;
}
