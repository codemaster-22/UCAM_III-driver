#include "ucam.h"

void send(const uint8_t* str,int len){
  int i;
  flush_uart(CAM);
  for(i =0;i<len;i++){
    printf("%d",(int)(*str));
    write_uart_character(CAM,*str);
    str++;
  }
  printf("\n");
}

void recieve_ack(int cmdno,int pid)
{
  if(cmdno==4){
    uint8_t str[12]; 
    int i = 0;
    for(i=0;i<12;i++){
      read_uart_character(CAM,(str+i));
      printf("%d\n",(int)*(str+i)); 
    }
  }else {
    uint8_t str[6]; 
    int i = 0;
    for(i=0;i<6;i++){
      read_uart_character(CAM,(str+i));
      printf("%d",(int)*(str+i));
    }
    printf("\n");
    uint8_t cc = (uint8_t)cmdno; 
    uint8_t pn[] = {(uint8_t)(pid>>8),(uint8_t)(pid%256)};
    uint8_t ack_ex[] = {(uint8_t)(0xAA),(uint8_t)(0x0E),cc,str[3],pn[0],pn[1]};
    assert(strcmp(ack_ex,str)==0);
  }
}

void get_ack(int cmdno,int pid, uint8_t str[])
{
  str[0] = (uint8_t)(0xAA);
  str[1] = (uint8_t)(0x0E);
  str[2] = (uint8_t)cmdno;
  str[3] = 0;
  str[4] = (uint8_t)(pid>>8);
  str[5] = (uint8_t) (pid%256);
}


void recieve_gen(uint8_t str[],int len)
{
  int i;
  for(i = 0;i < len; i++)
  {
    read_uart_character(CAM,str+i);
  }
}

void recieve_img(uint8_t *ptr){
  int i;
  uint8_t str[6];
  for(i=0;i<6;i++){
    read_uart_character(CAM,str+i);
  }
  for(i=0;i<506;i++){
    read_uart_character(CAM,ptr+i);
  }
}


void init_cam()
{
  int delay_time = 5;
  int no=0;
  const uint8_t  sync_command[] = {(uint8_t)(0xAA),(uint8_t)(0x0D), (uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  const uint8_t  ack_command[] = {(uint8_t)(0xAA),(uint8_t)(0x0E), (uint8_t)(0x0D),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};

  //SET_BAUDRATE_command : AA 07 31 00 00 00  ( specific for 115200 )
  start:
  {
    no++;
    printf("Round : %d\n",no);
    send(sync_command,6);
    delay((delay_time++)/1000); 
    if(CAM->status & STS_RX_NOT_EMPTY)
    {
        // code to read 12 characters from the buffer and if it matches with the ACK and sync commands go to succes
        int i;
        for(i = 0;i<6;i++) 
        {
           uint8_t ch;
           read_uart_character(CAM,&ch);
           printf("%d\n",(int)ch);
           if(i == 3)continue; // don't care xx thing
           if(ch != ack_command[i]) goto fail;
        }
        for(i = 0;i<6;i++)
        {
            uint8_t ch;
            read_uart_character(CAM,&ch);
            printf("%d\n",(int)ch);
            if(ch != sync_command[i]) goto fail;
        }
        goto success;
    }
  }
  fail:
    goto start;
  success:
    flush_uart(CAM);
    send(ack_command,6);
    printf("\n synchronisation process done waiting for 2 seconds for stabilisation \n");
    delay(2);
    printf("\n you are all set \n");
    const uint8_t  baud_command[] = {(uint8_t)(0xAA),(uint8_t)(0x07), (uint8_t)(0x31),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
    send(baud_command,6);
    uint8_t ack_recieve[7];
    recieve_gen(ack_recieve,6);
    ack_recieve[6] = 0;
    flush_uart(CAM);
    printf("\nbaud rate set set 2 115200 successfully \n");
    uint8_t reset_command[] = {(uint8_t)(0xAA),(uint8_t)(0x08),(uint8_t)(0x1),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
    send(reset_command,6);
    recieve_ack(8,0);
}

void get_pic()
{

  //INITIAL_command : AA 01 00 07 07 ( specific for JPEG,640x480 )
  uint8_t initial_command[] = {(uint8_t)(0xAA),(uint8_t)(0x01),(uint8_t)(0x0),(uint8_t)(0x07),(uint8_t)(0x09),(uint8_t)(0x07)};
  send(initial_command,6);
  recieve_ack(1,0);

  //SNAPSHOT_command : AA 05 00 00 00 00 ( jpg format, keep current frame)
  uint8_t snapshot_command[] = {(uint8_t)(0xAA),(uint8_t)(0x05),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  send(snapshot_command,6);
  recieve_ack(5,0);

  //SET_PACKAGE_SIZE_command : AA 06 08 00 02 00 (size=512)
  uint8_t set_package_command[] = {(uint8_t)(0xAA),(uint8_t)(0x06),(uint8_t)(0x08),(uint8_t)(0x0),(uint8_t)(0x02),(uint8_t)(0x0)};
  send(set_package_command,6);
  recieve_ack(6,0);


  //GET_PICTURE_command : AA 04 01 00 00 00 ( snapshot mode )
  uint8_t get_pic_command[] = {(uint8_t)(0xAA),(uint8_t)(0x4),(uint8_t)(0x1),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  send(get_pic_command,6);
  printf("LOL\n");
  recieve_ack(4,0);
  printf("LOL\n");
  uint8_t data[6];
  recieve_gen(data,6);
  printf("Data[0] :%d\n",(int)data[0]);
  printf("Data[1] :%d\n",(int)data[1]);
  printf("Data[2] :%d\n",(int)data[2]);
  //DATA_command : AA 0A 01 xx yy zz ( snapshot mode )
  // assert(data[0]==0xAA);
  // assert(data[1]==0xA);
  // assert(data[2]==0x1);

  // uint8_t ack_temp[6];
  // get_ack(0,0,ack_temp);
  // send(ack_temp, 6);
  // int no_of_pg = data[3] + (data[4]<<8) + (data[5]<<16) ;
  // uint8_t *image =(uint8_t*) malloc(no_of_pg);
  // no_of_pg = no_of_pg /(506) ;
  // int i = 0;
  // for(i=0;i<=no_of_pg;i++){
  //   recieve_img(image+506*i);
  //   get_ack(0,i+1,ack_temp);
  //   send(ack_temp, 6);
  // }
}

