#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "vidctl.h"
#include "scheduler.h"

//#include "filesys.h"
//#include "types.h"
static int key_status;
static int key_flag;
static int terminal_flag;
/* Define the key based on the  scan code set1
 * Alphabet and numbers are printed out as is
 * keys like backspace, tab, shift etc are just 
 * returned with a 0
 * we have 4 types of keys based on shift and caps lock
 */
static char key_pos[4][128]= {
{    //no cap & no shift (STATUS 0)
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,
    0,'q','w','e','r','t','y','u','i','o','p','[',']','\n',                            //0 -> '\n'
    0,'a','s','d','f','g','h','j','k','l',';','\'','`','\\',
    '\\','z','x','c','v','b','n','m',',','.','/',0,'*',
    0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,
    },
{    //no cap & shift (STATUS 1)
    0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,
    0,'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"', '~', '|',
    '\\','Z','X','C','V','B','N','M','<','>','?',0,'*',
    0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,'-',0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,
    },
{    //cap & no shift (STATUS 2)
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,
    0,'Q','W','E','R','T','Y','U','I','O','P','[',']','\n',
    0,'A','S','D','F','G','H','J','K','L',';','\'', '`', '\\',
    '\\','Z','X','C','V','B','N','M',',','.','/',0,'*',
    0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,
    },

{    //cap & shift (STATUS 3)
    0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,
    0,'q','w','e','r','t','y','u','i','o','p','{','}','\n',
    0,'a','s','d','f','g','h','j','k','l',':','"', '~', '|',
    '\\','z','x','c','v','b','n','m','<','>','?',0,'*',
    0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,'-',0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,
    }
};
/*
 * keybaord_init
 * DESCRIPTION : Here we set up initial conditions for the keyboard
 * by assuming no shift keys or caps lock are pressed
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : NONE
 */
void keyboard_init() {
    key_status = 0;
    key_flag = 0;
    flag_keyboard[0] = 0;
	flag_keyboard[1] = 0;
	flag_keyboard[2] = 0;
    counter1 = 0;
	counter2 = 0;
	counter3 = 0;
	terminal_flag = 0;																							//Initialize all relevant global variables
	
	process_array[schedule_index].cur_fd_ptr[0].flags=1;
	process_array[schedule_index].cur_fd_ptr[1].flags=1;
	process_array[schedule_index].cur_fd_ptr[0].table_pointer=&keyboard_table;
	process_array[schedule_index].cur_fd_ptr[0].inode=0;
	process_array[schedule_index].cur_fd_ptr[0].file_position=0;
	process_array[schedule_index].cur_fd_ptr[1].table_pointer=&keyboard_table;
	process_array[schedule_index].cur_fd_ptr[1].inode=0;
	process_array[schedule_index].cur_fd_ptr[1].file_position=0;												//Initialize STDIN and STDOUT
	
}
/*
 * here we set up a state machine like code where the keyboard status
 * changes along with whether the current key pressed is either of the shift
 * keys or the caps lock key
 */
unsigned getScancode() {
    uint8_t code;
    //current key
    code = 0;
    //here we wait until a key is pressed
    while(!code) {
        code = inb(KEYBOARD_PORT); //get input
    }
    if ( key_status == 0 && (code == LEFT_SHIFT_PRESS || code == RIGHT_SHIFT_PRESS)) {
        key_status = 1;
        return 0;
    }
    else if ( key_status == 1 && (code == LEFT_SHIFT_RELEASE || code == RIGHT_SHIFT_RELEASE)) {
        key_status = 0;
        return 0;
    }
    else if ( key_status == 0 && (code == CAPS_LOCK_PRESS)) {
        key_status = 2;
        return 0;
    }
    else if( key_status == 2 && (code == CAPS_LOCK_PRESS)) {
        key_status = 0;
        return 0;
    }
    else if ( key_status == 2 && (code == LEFT_SHIFT_PRESS || code == RIGHT_SHIFT_PRESS)) {
        key_status = 3;
        return 0;
    }
    else if ( key_status == 3 && (code == LEFT_SHIFT_RELEASE || code == RIGHT_SHIFT_RELEASE)) {
        key_status = 2;
        return 0;
    }
    else if (code == CTRL_PRESS) {
        key_flag = 1;
        return 0;
    }
    else if(code == CTRL_RELEASE) {
        key_flag = 0;
        return 0;
    }
	else if(code == LEFT_ALT_PRESS) {
		terminal_flag = 1;
		return 0;
	}
	else if(code == LEFT_ALT_RELEASE) {
		terminal_flag = 0;
		return 0;
	}
	else if (terminal_flag == 1 && code == F1_PRESS) {
		switch_vid(0);
		return 0;
	}
	else if (terminal_flag == 1 && code == F2_PRESS) {
		switch_vid(1);
		return 0;
	}
	else if (terminal_flag == 1 && code == F3_PRESS) {
		switch_vid(2);
		return 0;
	}
    else if (key_flag == 1 && code == L_PRESS) {
        key_clear_screen();
        return 0;
    }
    else if (code == DELETE_PRESS) {
		if(visible_screen == 0 && counter1 > 0){
			delete_char();
			terminal1_buffer[--counter1] = 0;
		}
		else if(visible_screen == 1 && counter2 > 0){									//A check to make sure delete_char is only called when terminal buffer is not empty
			delete_char();
			terminal2_buffer[--counter2] = 0;
		}
		else if(visible_screen == 2 && counter3 > 0){
			delete_char();
			terminal3_buffer[--counter3] = 0;
		}
        return 0;
    }
    else if (code < KEY_PRESS_BOUNDARY) { //keypress boundary
        return code;
    }
    return 0;
}





/*
 * getchar
 * DESCRIPTION : based on the getScancode function we are able to
 * get the correct key from the key_pos array that we
 * have set up
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : key position based on scancode and status
 */
unsigned char getchar() {
    return key_pos[key_status][getScancode()];
}

/*
 * keyboard_open
 * DESCRIPTION : based on the getScancode function we are able to
 * get the correct key from the key_pos array that we
 * have set up
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : key position based on scancode and status
 */
int32_t keyboard_open(uint32_t fd, uint32_t inode){
    int i;
    counter1 = 0;
	counter2 = 0;
	counter3 = 0;																				//Reset counters
	
    for(i = 0; i < BUFFER_SIZE; i++){
        terminal1_buffer[i] = 0;																//Clear keyboard buffer
		terminal2_buffer[i] = 0;
		terminal3_buffer[i] = 0;
    }
    process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&keyboard_table;
    process_array[schedule_index].cur_fd_ptr[fd].inode=0;
    process_array[schedule_index].cur_fd_ptr[fd].file_position=0;
    process_array[schedule_index].cur_fd_ptr[fd].flags=1;										//Mark keyboard in use
    return 0;
}
/*
 * keyboard_close
 * DESCRIPTION : based on the getScancode function we are able to
 * get the correct key from the key_pos array that we
 * have set up
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : key position based on scancode and status
 */
int32_t keyboard_close(int32_t fd){
    if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)
        return -1;
	
	int i;

    for(i = 0; i < BUFFER_SIZE; i++){
        terminal1_buffer[i] = 0;																//Clear keyboard buffer
		terminal2_buffer[i] = 0;
		terminal3_buffer[i] = 0;
    }
	
    process_array[schedule_index].cur_fd_ptr[fd].table_pointer=0;
    process_array[schedule_index].cur_fd_ptr[fd].inode=0;
    process_array[schedule_index].cur_fd_ptr[fd].file_position=0;								//Mark keyboard as not in usage
    process_array[schedule_index].cur_fd_ptr[fd].flags=0;
    return 0;
}
/*
 * keyboard_read
 * DESCRIPTION : based on the getScancode function we are able to
 * get the correct key from the key_pos array that we
 * have set up
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : key position based on scancode and status
 */
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes){
    
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)
        return -1;
	
	
	int bytes_read;
	
	pcb_t* current_pcb = get_pcb(process_array[schedule_index].cur_pid);
	
	while(current_pcb->screen_idx != visible_screen || (!flag_keyboard[visible_screen])){				//Only return after enter is pressed, or buffer is full
		
	}
	flag_keyboard[visible_screen] = 0;
	
    int i;
	if(visible_screen == 0){
		bytes_read = strlen(terminal1_buffer);		
		for(i = 0; i < bytes_read; i++){
			((char*)buf)[i] = terminal1_buffer[i];							//Read buffer into buf
		}
		((char*)buf)[i]=0;											//Add 0 to the end, to indicate end of string
		for(i = 0; i < 128; i++){
			terminal1_buffer[i] = 0;							//Clear buffer
		}
		counter1 = 0;
	}
	else if(visible_screen == 1){
		bytes_read = strlen(terminal2_buffer);
		for(i = 0; i < bytes_read; i++){
			((char*)buf)[i] = terminal2_buffer[i];							//Read buffer into buf
		}
		((char*)buf)[i]=0;
		for(i = 0; i < 128; i++){
			terminal2_buffer[i] = 0;							//Clear buffer
		}
		counter2 = 0;
	}
	else if(visible_screen == 2){
		bytes_read = strlen(terminal3_buffer);
		for(i = 0; i < bytes_read; i++){
			((char*)buf)[i] = terminal3_buffer[i];							//Read buffer into buf
		}
		((char*)buf)[i]=0;
		for(i = 0; i < 128; i++){
			terminal3_buffer[i] = 0;							//Clear buffer
		}
		counter3 = 0;
	}
    return bytes_read;
}
/*
 * keyboard_write
 * DESCRIPTION : based on the getScancode function we are able to
 * get the correct key from the key_pos array that we
 * have set up
 * INPUT : NONE
 * OUTPUT : NONE
 * RETURN : key position based on scancode and status
 */
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;
	
	for(i = 0; i < nbytes; i++){
		putc(((char*)buf)[i]);											//Display buffer onto screen
	}

    return i;
}

