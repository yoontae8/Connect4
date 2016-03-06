#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include "img_header.h" 
#include "logic_header.h" 
#include "ts_header.h"

#define FBDEVFILE "/dev/fb0"
#define PSDEVFILE "/dev/con_push"
#define FNDDEVFILE "/dev/con_fnd"
#define LTDEVFILE "/dev/con_led_timer"
#define IPADDR "192.168.10.111"
#define PORT 9000
    
//file descripter for push_button, frame buffer, fnd, led
int ps_fd, fb_fd, fnd_fd, led_fd; 
//game mode - single/multi1/multi2
int mode;
//file descripter for tcp/ip connection
int sock;

//allocate frame buffer memory 
u32 *pmmap;

//array for images
u32 **startPageStore;
u32 **resetStore;
u32 **selectnumPageStore;
u32 **selectwayPageStore;
u32 **waitingStore;
u32 **connectedStore;
u32 **boardStore;
u32 **redStore;
u32 **yellowStore;
u32 **rWinStore;
u32 **yWinStore;
u32 **showStore;
u32 **drawStore;
u32 **minigameStartStore;
u32 **minigameFailStore;
u32 **minigamePlayingStore;
u32 **minigameStageclearStore;
u32 **minigameAllclearStore;


unsigned int breakpoint; //flag for reset
unsigned int pushed_value; // push button input value
unsigned int led_seg_data;  //led save value
unsigned char fnd_seg_data; //fnd save value
char buffer[2];	    // send/recv data string
int xcrd, ycrd;	 // coordinate recv value
int turn;   //turn indicator
unsigned char fnd_hex_data[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90, 0xff} ; // 0~9, off

//for hidden_piece
unsigned char led_hex_data[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f, 0xff}; //
int seq[8]; // save answer value
unsigned int hidden = 0; //set push button control to hidden piece mode
int pushed_num; //save pushed number

void *push_thread(); // push event handler
void quit_signal(int sig); //interrupt event handler
void hidden_piece(); //hidden piece handler

int main(int argc, char* argv[]) {

    struct fb_var_screeninfo fbvar;
    int ioctl_ret, i,j;
    int xpos, ypos; //position in the board
    int result; // game result
    int xmax, ymax;  // save highest weight for AI
    int maxVal;	//coordinate for highest weight
    int fall;	// increment value for falling effect
    
    // save score
    int single_red_cnt = 0;
    int single_yellow_cnt = 0;
    int multi_red_cnt = 0;
    int multi_yellow_cnt = 0;
    int remote_red_cnt = 0;
    int remote_yellow_cnt = 0;
    u8 a, r, g, b; //color value for painting the player color on each side

    pthread_t p_thread; //thread for push_button

    //socket handle
    struct sockaddr_in serv_addr, clnt_addr;
    
    //board memory allocation
    int table[6][7];

    //image array allocation
    startPageStore = (u32 **)malloc(sizeof(u32*) * 480);
    selectnumPageStore = (u32 **)malloc(sizeof(u32*) * 480);
    selectwayPageStore = (u32 **)malloc(sizeof(u32*) * 480);
    waitingStore = (u32 **)malloc(sizeof(u32*) * 480);
    connectedStore = (u32 **)malloc(sizeof(u32*) * 480);
    resetStore = (u32 **)malloc(sizeof(u32*) * 480);
    boardStore = (u32 **)malloc(sizeof(u32*) * 480);
    redStore = (u32 **)malloc(sizeof(u32*) * 480);
    yellowStore = (u32 **)malloc(sizeof(u32*) * 480);
    rWinStore = (u32 **)malloc(sizeof(u32*) * 480);
    yWinStore = (u32 **)malloc(sizeof(u32*) * 480);
    showStore = (u32 **)malloc(sizeof(u32*) * 480);
    drawStore = (u32 **)malloc(sizeof(u32*) * 480);
    minigameStartStore= (u32 **)malloc(sizeof(u32*) * 480);
    minigameFailStore= (u32 **)malloc(sizeof(u32*) * 480);
    minigamePlayingStore= (u32 **)malloc(sizeof(u32*) * 480);
    minigameStageclearStore= (u32 **)malloc(sizeof(u32*) * 480);
    minigameAllclearStore= (u32 **)malloc(sizeof(u32*) * 480);
    for( i =0; i < 800; i++) {
	startPageStore[i] = (u32*)malloc(sizeof(u32) * 800);
	selectnumPageStore[i] = (u32*)malloc(sizeof(u32) * 800);
	selectwayPageStore[i] = (u32*)malloc(sizeof(u32) * 800);
	waitingStore[i] = (u32*)malloc(sizeof(u32) * 800);
	connectedStore[i] = (u32*)malloc(sizeof(u32) * 800);
	resetStore[i] = (u32*)malloc(sizeof(u32) * 800);
	boardStore[i] = (u32*)malloc(sizeof(u32) * 800);
	redStore[i] = (u32*)malloc(sizeof(u32) * 800);
	yellowStore[i] = (u32*)malloc(sizeof(u32) * 800);
	rWinStore[i] = (u32*)malloc(sizeof(u32) * 800);
	yWinStore[i] = (u32*)malloc(sizeof(u32) * 800);
	showStore[i] = (u32*)malloc(sizeof(u32) * 800);
	drawStore[i] = (u32*)malloc(sizeof(u32) * 800);
	minigameStartStore[i] = (u32*)malloc(sizeof(u32) * 800);
	minigameFailStore[i] = (u32*)malloc(sizeof(u32) * 800);
	minigamePlayingStore[i] = (u32*)malloc(sizeof(u32) * 800);
	minigameStageclearStore[i] = (u32*)malloc(sizeof(u32) * 800);
	minigameAllclearStore[i] = (u32*)malloc(sizeof(u32) * 800);
    }


    //save image in the array
    saveImgInArray("images/Board.bmp",boardStore);
    saveImgInArray("images/Red_full.bmp", redStore);
    saveImgInArray("images/Yellow_full.bmp", yellowStore);
    saveImgInArray("images/Startpage_with_texts.bmp", selectnumPageStore);
    saveImgInArray("images/Startpage_start.bmp", startPageStore);
    saveImgInArray("images/Startpage_selectboard.bmp", selectwayPageStore);
    saveImgInArray("images/Startpage_waitingforcon.bmp", waitingStore);
    saveImgInArray("images/Startpage_connected.bmp", connectedStore);
    saveImgInArray("images/Startpage_reset.bmp", resetStore);
    saveImgInArray("images/Result_redfacewins.bmp", rWinStore);
    saveImgInArray("images/Result_yellowfacewins.bmp", yWinStore);
    saveImgInArray("images/Result_Draw.bmp", drawStore);
    saveImgInArray("images/whitestones_full.bmp", showStore);
    saveImgInArray("images/Minigame_main.bmp", minigameStartStore);
    saveImgInArray("images/Minigame_fail.bmp", minigameFailStore);
    saveImgInArray("images/Minigame_playing.bmp", minigamePlayingStore);
    saveImgInArray("images/Minigame_stageclear.bmp", minigameStageclearStore);
    saveImgInArray("images/Minigame_allclear.bmp", minigameAllclearStore);

    //set the random seed for the minigame
    srand(time(NULL));

    //open device nodes 
    fb_fd = open(FBDEVFILE, O_RDWR);
    if(fb_fd < 0) {
	printf("fb_fd open error\n");
	exit(1);
    }

    ioctl_ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &fbvar);
    if(ioctl_ret < 0) {
	perror("Error: fb dev ioctl(VSCREENINFO GET)");
	return -1;
    }

    ps_fd = open(PSDEVFILE, O_RDONLY);
    if(ps_fd < 0){
	printf("ps_fd open error\n");
	return -1;
    }


    fnd_fd = open(FNDDEVFILE, O_WRONLY, O_SYNC);
    if(fnd_fd < 0) {
	printf("fnd_fd open error\n");
	return -1;
    }

    led_fd = open(LTDEVFILE, O_WRONLY, O_SYNC);
    if(led_fd < 0) {
	printf("led_fd open error\n");
	return -1;
    }

    //assign signal function for SIGINT
    signal(SIGINT, quit_signal);

    //allocate frame buffer memory to pmmap
    pmmap = (u32*)mmap(0, fbvar.xres * fbvar.yres * (fbvar.bits_per_pixel / 8), PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);

    //thread start
    pthread_create(&p_thread, NULL, push_thread, NULL);

    //game loop
    while(1) { 
        turn = 0;
        breakpoint = 0;

	//if 'O' of connect4 logo is pressed, start minigame
	if((xcrd > 268 && xcrd < 303) && (ycrd > 162 && ycrd < 196))
	{
	    hidden = 1;
	    hidden_piece();
	    hidden = 0;
        }
	
	
	//initialize the board array to 0 
	for(j = 0; j < 6; j++) {
	    for(i = 0; i < 7; i++) {
		table[j][i] = 0;
	    }
	}

	//display startpage image
	for(i = 0; i < 480; i++) {
	    for(j = 0; j < 800; j++)  {
		*(pmmap + (i * 800) + j) = startPageStore[i][j];
	    }
	}

	//proceed when 'start' is pressed
	do{
	   ts_read(&xcrd, &ycrd);
	} while((xcrd < 345 || xcrd > 465) || (ycrd < 290 || ycrd > 345) && (breakpoint != 1));

	//when reset activated(push)
	if(breakpoint == 1){
	    usleep(400000);
	    continue;
	}

	//sleep 0.2 sec to block unrequired input
	usleep(200000);

	//display menu for 1p/2p
	for(i = 270; i < 360; i++) {
	    for(j = 330; j < 475; j++)  {
		*(pmmap + (i * 800) + j) = selectnumPageStore[i][j];
	    }
	}

	//read selection and set mode
	do{
	   ts_read(&xcrd, &ycrd);

	} while((xcrd < 330 || xcrd > 475) || (ycrd < 270 || ycrd > 360) && (breakpoint != 1));

       //when reset activated(push)
	if(breakpoint == 1) {
	    usleep(400000);
	    continue;
	}
	// single play
	if(ycrd < 313)
	{
	    fnd_seg_data = fnd_hex_data[single_red_cnt];
	    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

	    led_seg_data = 256 - pow(2, single_yellow_cnt);
	    write(led_fd, &led_seg_data, sizeof(led_seg_data));
	    mode = 1;
	}
	//multi play
	else
	{
	    // display multiplay mode - one board / two board
	     for(i = 274; i < 360; i++) {
		for(j = 270; j < 557; j++)  {
		    *(pmmap + (i * 800) + j) = selectwayPageStore[i][j];
		}
	     }
	     usleep(400000);
	     //read selection
	     do{
		 ts_read(&xcrd, &ycrd);
	     } while((xcrd < 270 || xcrd > 557) || (ycrd < 274 || ycrd > 360) && (breakpoint != 1));
	     //when reset activated(push)
	     if(breakpoint == 1) {
		 usleep(400000);
		 continue;
	     }
	     if(ycrd < 313) {
		 //display win number with led & fnd
		 fnd_seg_data = fnd_hex_data[multi_red_cnt];
		 write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

		 led_seg_data = 256 - pow(2, multi_yellow_cnt);
		 write(led_fd, &led_seg_data, sizeof(led_seg_data));

		 //set mode
		 mode = 2;
	     }
	     //remote multiplay mode
	     else
	     {
		//create socket
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock == -1) {
		    printf("socket() error.\n");
		    exit(1);
		}

		//set address
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(IPADDR);
		serv_addr.sin_port = htons(PORT);
		
		//set led to flow mode
		led_seg_data = 1;
		write(led_fd, &led_seg_data, sizeof(led_seg_data));

		//print waiting for connection 
		for(i = 274; i < 360; i++) {
		    for(j = 210; j < 602; j++)  {
			*(pmmap + (i * 800) + j) = waitingStore[i][j];
		    }
		 }

		//try connection
		while(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		    usleep(500000);
		    //if reset, break
		    if(breakpoint == 1) 
			break;
		}
		if(breakpoint == 1) {
		    close(sock);
		    continue;
		}

		//connection success!!
		
		//turn off the led
		led_seg_data = 255;
		write(led_fd, &led_seg_data, sizeof(led_seg_data));

		//print 'connected'
		 for(i = 296; i < 338; i++) {
		    for(j = 210; j < 602; j++)  {
			*(pmmap + (i * 800) + j) = connectedStore[i][j];
		    }
		 }
		 sleep(1);

		 
		 //display win number with led & fnd
		 fnd_seg_data = fnd_hex_data[remote_red_cnt];
		 write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

		 led_seg_data = 256 - pow(2, remote_yellow_cnt);
		 write(led_fd, &led_seg_data, sizeof(led_seg_data));

		 mode = 3;
	     }
	}

	usleep(400000);

	// print gameboard
	for(i = 0; i < 480; i++) {
	    for(j = 0; j < 800; j++)  {
		*(pmmap + (i * 800) + j) = boardStore[i][j];
	    }
	}

	//color the side with red(player color)
	if(mode == 1) {
	    a = 0;
	    r = 255;
	    g = 0;
	    b = 0;
	    for(i = 0; i < 480; i++) {
		for(j = 0; j < 120; j++) {
		    *(pmmap + (i * 800) + j) = ((u32) ( (a<<24) | (r<<16) | (g << 8) | b));
		}
	    }
	    for(i = 0; i < 480; i++) {
		for(j = 680; j < 800; j++) {
		    *(pmmap + (i * 800) + j) = ((u32) ( (a<<24) | (r<<16) | (g << 8) | b));
		}
	    }
	}

	//set turn to red(always red starts first)
        turn = RED;
	
	//loop for one turn
	while(1) {
	    // case of 1p->user, 2p oneboard->all, 2p twoboard->client(this board's player)
	    if((turn == RED && mode == 1) || mode == 2 || (mode == 3 && turn == YELLOW))
	    {
		//color the side with player color
		if(mode == 2 || mode == 3) {
		    a = 0;
		    r = 255;
		    g = (turn == RED) ? 0 : 255;
		    b = 0;
		    //left side
		    for(i = 0; i < 480; i++) {
			for(j = 0; j < 120; j++) {
			    *(pmmap + (i * 800) + j) = ((u32) ( (a<<24) | (r<<16) | (g << 8) | b));
			}
		    }
		    //right side
		    for(i = 0; i < 480; i++) {
			for(j = 680; j < 800; j++) {
			    *(pmmap + (i * 800) + j) = ((u32) ( (a<<24) | (r<<16) | (g << 8) | b));
			}
		    }
		}

		ts_read(&xcrd, &ycrd); //read touched coordinate

		//break if upper button is pushed
		if(breakpoint == 1)
		{
		    usleep(400000);
		    break;
		}
		
		// get x coordinate's corresponding position in board
		xpos = get_colnum(xcrd);
		if(xpos == -1)
		    continue;
		// insert the marker and get horizontal position
		ypos = insert(table, xpos, turn);
		
		// send x position to the server if two board mode
		if(mode == 3) {
		   // save x position in the buffer
		   sprintf(buffer, "%d", xpos);
		   if(send(sock, buffer, sizeof(buffer), 0) == -1)
		   {
		       printf("send() error.\n");
		       exit(1);
		   }
		}
	    }
	    // 1p->Computer's turn
	    else if(mode == 1)
	    {
		maxVal = 0;
		//for each position available
		for(i = 0; i < 7; i++) {
		    //get y position and save in j
		    int temp;
		    j = 5;
		    while(table[j][i] != 0 && j >= 0)
			j--;
		    if(j == -1)
			continue;
		    //get weight
		    temp = getWeight(table, i, j, turn);
		    
		    //save x, y position in the xmax, ymax
		    if(temp > maxVal) {
			maxVal = temp;
			xmax = i;
			ymax = j;
		    }
		    else if(temp == maxVal) {
			if(i <= 3) {
			    temp = maxVal;
			    xmax = i;
			    ymax = j;
			}
		    }
		}
		
		//insert marker in the position with biggest weight
		ypos = ymax;
		xpos = xmax;
		insert(table, xpos, turn); 
	    }

	    //if 2p twoboard -> server's turn(opponent board)
	    if(turn == RED && mode == 3) {
		//color the side with black(opponent's turn)
		for(i = 0; i < 480; i++) {
		    for(j = 0; j < 120; j++) {
			*(pmmap + (i * 800) + j) = (u32) 0;
		    }
		}
		for(i = 0; i < 480; i++) {
		    for(j = 680; j < 800; j++) {
			*(pmmap + (i * 800) + j) = (u32) 0;
		    }
		}
		//receive the data sent from the opponent
		if((recv(sock, buffer, 2, 0)) == -1) {
		    printf("recv() error.\n");
		    exit(1);
		}
		xpos = atoi(buffer);
		//if xpos == 9, opponent resign. player wins
		if(xpos == 9)
		{
		    for(i = 180; i < 270; i++) {
			for(j = 120; j < 680; j++) {
			    *(pmmap + (i * 800) + j) = yWinStore[i][j];
			}
		    }
		    ts_read(&xcrd, &ycrd);
		    usleep(400000);
		    close(sock);
		    break;
		}
		//insert the opponent's marker in the x position
		ypos = insert(table, xpos, turn);
	    }
	    
	    //column full
	    if(ypos == -1) 
		continue;
	   
	    //draw the falling marker 
	    for(fall = 0; ; fall++)
	    {
		//draw the marker
		for(i = fall * 80; i < (fall + 1) * 80; i++) {
		    for(j = xpos * 80 + 120; j < (xpos + 1) * 80 + 120; j++) {
			*(pmmap + (i * 800) + j) = (turn == RED)? redStore[i][j] : yellowStore[i][j];
		    }
		}
		usleep(50000);

		//if marker is in right position, break
		if(fall == ypos)
		    break;

		//erase the marker
		for(i = fall * 80; i < (fall + 1) * 80; i++) {
		    for(j = xpos * 80 + 120; j < (xpos + 1) * 80 + 120; j++) {
			*(pmmap + (i * 800) + j) = boardStore[i][j];
		    }
		}
	    }

	    // get the result of the player
	    result = check(table, xpos, ypos, turn);
	    
	    //game over
	    if(result == 0) {
		//1p
		if(mode == 1)
		{
		    //red win
		    if(turn == RED) {
			single_red_cnt++;
			//if red wins 8 times, reset
			if(single_red_cnt == 9) {
			    single_red_cnt = 0;
			    single_yellow_cnt = 0;
			}
			//write the changed fnd value
			else {
			    fnd_seg_data = fnd_hex_data[single_red_cnt];
			    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));
			}
		    }
		    //yellow win
		    else {
			single_yellow_cnt++;
			if(single_yellow_cnt == 9) {
			    single_red_cnt = 0;
			    single_yellow_cnt = 0;
			}
			else {
			    led_seg_data = 256 - pow(2, single_yellow_cnt);
			    write(led_fd, &led_seg_data, sizeof(led_seg_data));
			}
		    }
		}
		//2p oneboard
		else if(mode == 2)
		{
		    if(turn == RED) {
			multi_red_cnt++;
			if(multi_red_cnt == 9) {
			    multi_red_cnt = 0;
			    multi_yellow_cnt = 0;
			}
			else {
			    fnd_seg_data = fnd_hex_data[multi_red_cnt];
			    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));
			}
		    }
		    else {
			multi_yellow_cnt++;
			if(multi_yellow_cnt == 9) {
			    multi_red_cnt = 0;
			    multi_yellow_cnt = 0;
			}
			else {
			    led_seg_data = 256 - pow(2, multi_yellow_cnt);
			    write(led_fd, &led_seg_data, sizeof(led_seg_data));
			}
		    }
		}
		//2p twoboard
		else
		{
		    if(turn == RED) {
			remote_red_cnt++;
			if(remote_red_cnt == 9) {
			    remote_red_cnt = 0;
			    remote_yellow_cnt = 0;
			}
			else {
			    fnd_seg_data = fnd_hex_data[remote_red_cnt];
			    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));
			}
		    }
		    else {
			remote_yellow_cnt++;
			if(remote_yellow_cnt == 9) {
			    remote_red_cnt = 0;
			    remote_yellow_cnt = 0;
			}
			else {
			    led_seg_data = 256 - pow(2, remote_yellow_cnt);
			    write(led_fd, &led_seg_data, sizeof(led_seg_data));
			}
		    }
		}
			
		//for debugging
		printf("%s wins!\n", (turn == RED)? "RED" : "YELLOW");
		
		for(j = 0; j < 6; j++) {
		    for(i = 0; i < 7; i++) {
			printf("%d ", table[j][i]);
		    }
		    printf("\n");
		}
		printf("\n");

		//show the connected 4 markers with sequentially
		for(ypos = 0; ypos < 6; ypos++) {
		    for(xpos = 0; xpos < 7; xpos++) {
			if(table[ypos][xpos] == 3) {
			    for(i = ypos * 80; i < (ypos + 1) * 80; i++) {
				for(j = xpos * 80 + 120; j < (xpos+ 1) * 80 + 120; j++) {
				    *(pmmap + (i * 800) + j) = showStore[i][j];
				}
			    }
			    usleep(500000);
			}
		    }
		}
		// print win image
		for(i = 180; i < 270; i++) {
		    for(j = 120; j < 680; j++) {
			    *(pmmap + (i * 800) + j) = (turn==RED)? rWinStore[i][j] : yWinStore[i][j];
			}
		}

		//wait until touch
		ts_read(&xcrd, &ycrd);
		usleep(400000);

		//socket close
		if(mode == 3) 
		    close(sock);
		
		//turn off the led & fnd
		fnd_seg_data = fnd_hex_data[10];
		write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

		led_seg_data = 255;
		write(led_fd, &led_seg_data, sizeof(led_seg_data));

		break;
	    }
	    //draw - board is full
	    if(result == 1) {
		printf("draw..\n");
		
		// print draw image
		for(i = 180; i < 270; i++) {
		    for(j = 120; j < 680; j++) {
			    *(pmmap + (i * 800) + j) = drawStore[i][j];
			}
		}

		usleep(400000);
		ts_read(&xcrd, &ycrd);
		usleep(400000);

		if(mode == 3)
		    close(sock);
		
		//turn off the led & fnd
		fnd_seg_data = fnd_hex_data[10];
		write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

		led_seg_data = 255;
		write(led_fd, &led_seg_data, sizeof(led_seg_data));

		break;
	    }
	    //change the turn
	    if(turn == RED)
		turn = YELLOW;
	    else
		turn = RED;

	    //for debugging
	    for(j = 0; j < 6; j++) {
		for(i = 0; i < 7; i++) {
		    printf("%d ", table[j][i]);
		}
		printf("\n");
	    }
	    printf("\n");

	    //sleep to prevent rapid input
	    if(mode == 2)
		usleep(100000);
	}
   }

    return 0;
    
}
// thread for push button event
void *push_thread()
{
    int i, j;
    while(1) {
	usleep(400000);
	read(ps_fd, &pushed_value, 1);

	//minigame mode
	if(hidden == 1)
	{
	    for(i = 0; i < 9; i++) {
		//get number of pushed button
		if(pushed_value == led_hex_data[i]) {
		   pushed_num = i;
		   continue;
		} 
	    }
	}
	//usual mode
	else if(pushed_value != 0xFF && breakpoint == 0) {
	    //5, 6, 7, 8th button is pressed, close the game
	    if(pushed_value < 0xf7)
		raise(SIGINT);
	    //1, 2, 3, 4th button is pressed, reset and go to main menu
	    else {
		//2p two board mode
	       if(mode == 3 && turn != 0) 
	       {
		   //block when opponent player's turn
		   if(turn == RED)
		       continue;
		   //send '9'
		   sprintf(buffer, "%d", 9);
		   if(send(sock, buffer, sizeof(buffer), 0) == -1)
		   {
		       printf("send() error.\n");
		       exit(1);
		   }
		    close(sock);
	       }
	       //display reset images
		for(i = 0; i < 480; i++) {
		    for(j = 0; j < 800; j++) {
			*(pmmap + (i * 800) + j) = resetStore[i][j];
		    }
		} 
		//turn off fnd & led
		fnd_seg_data = fnd_hex_data[10];
		write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

		led_seg_data = 255;
		write(led_fd, &led_seg_data, sizeof(led_seg_data));

		breakpoint = 1;
	    }
	}
    }
}
//close the program
void quit_signal(int sig) 
{
    int i, j;

    //paint the screen black
    for(i = 0; i < 480; i++) {
	for(j = 0; j < 800; j++) {
	    *(pmmap + (i * 800) + j) = (u32)0;
	}
    } 

    if(mode == 3)
	close(sock);

    fnd_seg_data = fnd_hex_data[10];
    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

    led_seg_data = 255;
    write(led_fd, &led_seg_data, sizeof(led_seg_data));

    close(fb_fd);
    close(ps_fd);
    close(fnd_fd);
    close(led_fd);
    if(munmap(pmmap, 800*480*4) == -1) {
	perror("munmap() error.\n");
	exit(1);
    }
    exit(0);
}

//minigame
void hidden_piece() 
{
    int i, j;
    int count = 0;
    //interval between led on
    unsigned long interval = 500000;

    //display minigame instruction
    for(i = 240; i < 390; i++) {
	for(j = 68; j < 740; j++) {
	    *(pmmap + (i * 800) + j) = minigameStartStore[i][j];
	}
    } 

    //read touch on the word 'start' 
    do{
       ts_read(&xcrd, &ycrd);
    } while((xcrd < 343 || xcrd > 427) || (ycrd < 360 || ycrd > 390));

    //turn on the led with random order
    while(1) 
    {
	//turn off the fnd
	fnd_seg_data = fnd_hex_data[10];
	write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

	//display game playing image
	for(i = 240; i < 390; i++) {
	    for(j = 68; j < 740; j++) {
		*(pmmap + (i * 800) + j) = minigamePlayingStore[i][j];
	    }
	} 
	sleep(1);
	
	//turn on eight leds sequencially, ramdomly
	for(i = 0; i < 8; i++) {
	    seq[i] = rand() % 8;    //save the random number in array
	    printf("%d\n", seq[i]+1); //for debugging

	    led_seg_data = led_hex_data[seq[i]]; //turn on the random led
	    write(led_fd, &led_seg_data, sizeof(led_seg_data));

	    usleep(interval);

	    //turn off the led
	    led_seg_data = 255;
	    write(led_fd, &led_seg_data, sizeof(led_seg_data));

	    usleep(interval / 5);
	}
	printf("\n");

	//show '8' with fnd which is remaining number
	fnd_seg_data = fnd_hex_data[8];
	write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

	for(i = 0; i < 8; i++) {
	    //keep reading until number is between 0 and 7
	    while(pushed_num == 8);

	    //if wrong button is pushed
	    if(pushed_num != seq[i]) {
		//turn off the fnd 
		fnd_seg_data = fnd_hex_data[10];
		write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));
		
		//display 'game over'
		for(i = 240; i < 390; i++) {
		    for(j = 68; j < 740; j++) {
			*(pmmap + (i * 800) + j) = minigameFailStore[i][j];
		    }
		} 
		ts_read(&xcrd, &ycrd);
		usleep(400000);
		return;
	    }
	    //correct button
	    else {
		//display decremented number in fnd
		fnd_seg_data = fnd_hex_data[7-i];
		write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));
		usleep(400000);
	    }
	}
	//if 5 stages are cleared, finish
	if(count == 5)
	    break;

	//display the stage number 
	fnd_seg_data = fnd_hex_data[count+1];
	write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

	//display stage clear image
	for(i = 240; i < 390; i++) {
	    for(j = 220; j < 590; j++) {
		*(pmmap + (i * 800) + j) = minigameStageclearStore[i][j];
	    }
	} 
	//press 'proceed' to go on 
	do{
	   ts_read(&xcrd, &ycrd);
	} while((xcrd < 333 || xcrd > 444) || (ycrd < 355 || ycrd > 390));

	usleep(500000);

	//reduce the time interval for harder game
	interval -= 50000;
	count++;
	
    }
    //trn off the fnd
    fnd_seg_data = fnd_hex_data[10];
    write(fnd_fd, &fnd_seg_data, sizeof(fnd_seg_data));

    //display all clear image
    for(i = 0; i < 480; i++) {
	for(j = 0; j < 800; j++) {
	    *(pmmap + (i * 800) + j) = minigameAllclearStore[i][j];
	}
    } 
    //press any button to proceed
    ts_read(&xcrd, &ycrd);
    usleep(400000);
}
