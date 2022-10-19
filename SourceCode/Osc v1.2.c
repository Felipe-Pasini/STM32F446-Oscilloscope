#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#include "stdint-gcc.h"
#include "conio.h"
#include "math.h"
#include "locale.h"
#include "time.h"

#define OK 1
#define TIMEOUT_ERROR 15
#define START_OF_CONVERSION 83
#define DEFINE_CONFIG 67
#define MIN 0
#define MAX 1
#define ADC_RESOLUTION 3.3/256

#define BLACK 0
#define WHITE 255
#define RED 249
#define GREEN 250
#define YELLOW 251
#define BLUE 252


FILE *bitmap;
FILE *text;
FILE *serial;
HANDLE p_com;//handle para abertura da porta serial

//================================PRIVATE VARIABLES=================================
char com[5] = "COM3";//default COM 3
char status[10] = "Fechada";
int Num_of_Channels = 1;
int Sampling_Rate = 500000;
int baud=115200;

unsigned char array[10000][3];
unsigned char channel;
unsigned int N_Samples;
unsigned int Period[3];

unsigned char Sent_Sampling_Rate=5;
unsigned char N_Samples_is_Set=FALSE;

unsigned long n;

const char Color[3] = {RED,GREEN,BLUE};//default Osc colors

const unsigned int ColorPalette[]=//pre-defined color pallete for bitmap
{
    0x0,//0
    0x800000,//1
    0x8000,//2
    0x808000,//3
    0x80,//4
    0x800080,//5
    0x8080,//6
    0xc0c0c0,//7
    0xc0dcc0,//8
    0xa6caf0,//9
    0x402000,//10
    0x602000,//11
    0x802000,//12
    0xa02000,//13
    0xc02000,//14
    0xe02000,//15
    0x4000,//16
    0x204000,//17
    0x404000,//18
    0x604000,//19
    0x804000,//20
    0xa04000,//21
    0xc04000,//22
    0xe04000,//23
    0x6000,//24
    0x206000,//25
    0x406000,//26
    0x606000,//27
    0x806000,//28
    0xa06000,//29
    0xc06000,//30
    0xe06000,//31
    0x8000,//32
    0x208000,//33
    0x408000,//34
    0x608000,//35
    0x808000,//36
    0xa08000,//37
    0xc08000,//38
    0xe08000,//39
    0xa000,//40
    0x20a000,//41
    0x40a000,//42
    0x60a000,//43
    0x80a000,//44
    0xa0a000,//45
    0xc0a000,//46
    0xe0a000,//47
    0xc000,//48
    0x20c000,//49
    0x40c000,//50
    0x60c000,//51
    0x80c000,//52
    0xa0c000,//53
    0xc0c000,//54
    0xe0c000,//55
    0xe000,//56
    0x20e000,//57
    0x40e000,//58
    0x60e000,//59
    0x80e000,//60
    0xa0e000,//61
    0xc0e000,//62
    0xe0e000,//63
    0x40,//64
    0x200040,//65
    0x400040,//66
    0x600040,//67
    0x800040,//68
    0xa00040,//69
    0xc00040,//70
    0xe00040,//71
    0x2040,//72
    0x202040,//73
    0x402040,//74
    0x602040,//75
    0x802040,//76
    0xa02040,//77
    0xc02040,//78
    0xe02040,//79
    0x4040,//80
    0x204040,//81
    0x404040,//82
    0x604040,//83
    0x804040,//84
    0xa04040,//85
    0xc04040,//86
    0xe04040,//87
    0x6040,//88
    0x206040,//89
    0x406040,//90
    0x606040,//91
    0x806040,//92
    0xa06040,//93
    0xc06040,//94
    0xe06040,//95
    0x8040,//96
    0x208040,//97
    0x408040,//98
    0x608040,//99
    0x808040,//100
    0xa08040,//101
    0xc08040,//102
    0xe08040,//103
    0xa040,//104
    0x20a040,//105
    0x40a040,//106
    0x60a040,//107
    0x80a040,//108
    0xa0a040,//109
    0xc0a040,//110
    0xe0a040,//111
    0xc040,//112
    0x20c040,//113
    0x40c040,//114
    0x60c040,//115
    0x80c040,//116
    0xa0c040,//117
    0xc0c040,//118
    0xe0c040,//119
    0xe040,//120
    0x20e040,//121
    0x40e040,//122
    0x60e040,//123
    0x80e040,//124
    0xa0e040,//125
    0xc0e040,//126
    0xe0e040,//127
    0x80,//128
    0x200080,//129
    0x400080,//130
    0x600080,//131
    0x800080,//132
    0xa00080,//133
    0xc00080,//134
    0xe00080,//135
    0x2080,//136
    0x202080,//137
    0x402080,//138
    0x602080,//139
    0x802080,//140
    0xa02080,//141
    0xc02080,//142
    0xe02080,//143
    0x4080,//144
    0x204080,//145
    0x404080,//146
    0x604080,//147
    0x804080,//148
    0xa04080,//149
    0xc04080,//150
    0xe04080,//151
    0x6080,//152
    0x206080,//153
    0x406080,//154
    0x606080,//155
    0x806080,//156
    0xa06080,//157
    0xc06080,//158
    0xe06080,//159
    0x8080,//160
    0x208080,//161
    0x408080,//162
    0x608080,//163
    0x808080,//164
    0xa08080,//165
    0xc08080,//166
    0xe08080,//167
    0xa080,//168
    0x20a080,//169
    0x40a080,//170
    0x60a080,//171
    0x80a080,//172
    0xa0a080,//173
    0xc0a080,//174
    0xe0a080,//175
    0xc080,//176
    0x20c080,//177
    0x40c080,//178
    0x60c080,//179
    0x80c080,//180
    0xa0c080,//181
    0xc0c080,//182
    0xe0c080,//183
    0xe080,//184
    0x20e080,//185
    0x40e080,//186
    0x60e080,//187
    0x80e080,//188
    0xa0e080,//189
    0xc0e080,//190
    0xe0e080,//191
    0xc0,//192
    0x2000c0,//193
    0x4000c0,//194
    0x6000c0,//195
    0x8000c0,//196
    0xa000c0,//197
    0xc000c0,//198
    0xe000c0,//199
    0x20c0,//200
    0x2020c0,//201
    0x4020c0,//202
    0x6020c0,//203
    0x8020c0,//204
    0xa020c0,//205
    0xc020c0,//206
    0xe020c0,//207
    0x40c0,//208
    0x2040c0,//209
    0x4040c0,//210
    0x6040c0,//211
    0x8040c0,//212
    0xa040c0,//213
    0xc040c0,//214
    0xe040c0,//215
    0x60c0,//216
    0x2060c0,//217
    0x4060c0,//218
    0x6060c0,//219
    0x8060c0,//220
    0xa060c0,//221
    0xc060c0,//222
    0xe060c0,//223
    0x80c0,//224
    0x2080c0,//225
    0x4080c0,//226
    0x6080c0,//227
    0x8080c0,//228
    0xa080c0,//229
    0xc080c0,//230
    0xe080c0,//231
    0xa0c0,//232
    0x20a0c0,//233
    0x40a0c0,//234
    0x60a0c0,//235
    0x80a0c0,//236
    0xa0a0c0,//237
    0xc0a0c0,//238
    0xe0a0c0,//239
    0xc0c0,//240
    0x20c0c0,//241
    0x40c0c0,//242
    0x60c0c0,//243
    0x80c0c0,//244
    0xa0c0c0,//245
    0xfffbf0,//246
    0xa0a0a4,//247
    0x808080,//248
    0xff0000,//249
    0xff00,//250
    0xffff00,//251
    0xff,//252
    0xff00ff,//253
    0xffff,//254
    0xffffff,//255
};

struct Header_BMP//bitmap file header
{
    //short int BM;//0-2
    int FileSize;//2-6
    int Reserved;//6-10
    int DataOffset;//10-14

    int InfoHeaderSize;//14-18
    int Width;//18-22
    int Height;//22-26
    int16_t Planes;//26-28
    int16_t BitsPerPixel;//28-30
    int Compression;//30-34
    int ImageSize;//34-38
    int XPixelsPerM;//38-42
    int YPixelsPerM;//42-46
    int ColorsUsed;//46-50
    int ImportantColors;//50-54

}BMP;

HANDLE AbreComm(char *nomecom, int baudrate)//COM port opening handle
{
    HANDLE hcom; //cria um ponteiro de nome hcom
    DCB cdcb;    //nomeia a estrutura DCB (Device Control Block) utilizada para definir todos os parâmetros da comunicação
    COMMTIMEOUTS comto; //nomeia a estrutura COMMTIMEOUTS (COMMon TIME OUTS) utilizada para definir os timeouts da comunicação
        hcom = CreateFile( nomecom, //nome do arquivo
            GENERIC_READ | GENERIC_WRITE, //abre arquivo para leitura ou escrita
            0, //indica que o arquivo não pode ser compartilhado
            NULL, //utiliza a estrutura default para as funções de segurança
            OPEN_EXISTING, //abre o arquivo, se não existir, retorna erro
            FILE_ATTRIBUTE_NORMAL, //o arquivo deve ser utilizado sozinho
            NULL); //sem gabarito de atributos
            if(hcom == INVALID_HANDLE_VALUE) // testa falha na abertura do arquivo
            {
                return hcom;
            }

    GetCommState(hcom, &cdcb); //le os parâmetros de comunicação atuais
    cdcb.BaudRate    = baudrate; //define a taxa de transmissão
    cdcb.ByteSize    = 8; //define o tamanho do dado - 8 bits
        cdcb.StopBits    = ONESTOPBIT; //define o tamanho do stop bit - 1 stop bit
    cdcb.Parity      = NOPARITY; //define o tipo de paridade - sem paridade
        if(!SetCommState(hcom,&cdcb)) //seta os novos parâmetros de comunicação
        {
            fputs("SetCommState", stderr);
            return INVALID_HANDLE_VALUE;
        }

        GetCommTimeouts(hcom, &comto);
        //Le os parâmetros atuais de COMMTIMEOUTS
        comto.ReadIntervalTimeout         = MAXDWORD;
        //tempo máximo entre a chegada de dois caracters consecutivos(ms)
        comto.ReadTotalTimeoutMultiplier  =    0;
        comto.ReadTotalTimeoutConstant    =    0;
        comto.WriteTotalTimeoutMultiplier =    0;
        comto.WriteTotalTimeoutConstant   =    0;
        SetCommTimeouts(hcom, &comto);
        //seta os parâmetros de COMMTIMEOUTS
        return hcom;
}

void CreateBMP(char* name, int H, int W)//Creates empty bitmap
{
    if(W%4)
    {
        W=(W+(4-W%4));//sets width as a multiple of 4 (avoid errors in file size)
    }

    BMP.FileSize=54+1024+(H*W);
    BMP.Reserved=0x0;
    BMP.DataOffset=1078;//14 + 40 + 256*4 bytes (color pallete)
    BMP.InfoHeaderSize=40;
    BMP.Width=W;
    BMP.Height=H;
    BMP.Planes=1;
    BMP.BitsPerPixel=8;
    BMP.Compression=0;
    BMP.ImageSize= H*W;
    BMP.XPixelsPerM=0;
    BMP.YPixelsPerM=0;
    BMP.ColorsUsed=0;
    BMP.ImportantColors=0;

    bitmap=fopen(name,"wb");//opens bitmap (if not yet created, it will be)

    fwrite("BM",2,1,bitmap);//write bitmap format indicator

    fwrite(&BMP,sizeof(BMP),1,bitmap);//write all bitmap headers

    fwrite(&ColorPalette,sizeof(ColorPalette),1,bitmap);//write pre-defined color pallete


    for(int aux=0;aux<(BMP.Width*BMP.Height);aux++)//fill bitmap with white pixels
    {
        fputc(WHITE,bitmap);
    }

    fclose(bitmap);//closes created bitmap
}

void PrintScreen (void)//Prints Oscilloscope Info in screen
{
    system("cls");//clear screen
    printf("OSCILOSCÓPIO v1.2");
    printf("\nPorta Serial: %s",com);
    printf("\nStatus da porta: %s",status);

    if(N_Samples_is_Set==FALSE){printf("\nNúmero de amostras exibidas: Auto");}//number of samples is being auto-set
    else{printf("\nNúmero de amostras exibidas: %d",N_Samples);}//number of samples is pre-set

    printf("\nTaxa de amostragem: %dkHz",Sampling_Rate/1000);
    printf("\nNúmero de canais: %d",Num_of_Channels);
}

int ConfigOSC (void)//Osc configuration process
{
    char key=0;

    PrintScreen();
    printf("\n\nDigite o número de canais (máximo 3): ");//number of channel
    fflush(stdin);
    scanf("%d",&Num_of_Channels);
    while(Num_of_Channels>3 || Num_of_Channels<=0)//assure valid number (0-3)
    {
        printf("\nNúmero inválido. Por favor digite um número de 1 a 3: ");
        scanf("%d",&Num_of_Channels);
    }

    PrintScreen();//Refresh Screen with selected number of channels
    printf("\n\nEscolha a taxa de amostragem do sinal:");//sampling rate


    printf("\n[1]1kHz\n[2]5kHz\n[3]10kHz\n[4]50kHz\n[5]100kHz\n[6]500kHz\n");

    if(Num_of_Channels!=3)printf("[7]1MHZ\n");//1 MHz option is enabled for 1 or 2 channels
    if(Num_of_Channels==1)printf("[8]2MHz\n");//2 MHs option is enabled for 1 channel only

    while(key>'8' || key<'1')//assures valid sampling rate
    {
        key=getch();
        switch (key)
        {
        case '1':
            Sampling_Rate=1000;
            Sent_Sampling_Rate=0;
            break;
        case '2':
            Sampling_Rate=5000;
            Sent_Sampling_Rate=1;
            break;
        case '3':
            Sampling_Rate=10000;
            Sent_Sampling_Rate=2;
            break;
        case '4':
            Sampling_Rate=50000;
            Sent_Sampling_Rate=3;
            break;
        case '5':
            Sampling_Rate=100000;
            Sent_Sampling_Rate=4;
            break;
        case '6':
            Sampling_Rate=500000;
            Sent_Sampling_Rate=5;
            break;
        case '7':
            if(Num_of_Channels!=3)
            {
                Sampling_Rate=1000000;
                Sent_Sampling_Rate=6;
                break;
            }
        case '8':
            if(Num_of_Channels==1)
            {
                Sampling_Rate=2000000;
                Sent_Sampling_Rate=7;
                break;
            }
        default:
            if(Num_of_Channels==1)
            {
                printf("\n\nEste não é um valor válido, por favor escolha um número de 1 a 8:");
            }
            else if(Num_of_Channels==2)
            {
                printf("\n\nEste não é um valor válido, por favor escolha um número de 1 a 7:");
            }
            else{
            printf("\n\nEste não é um valor válido, por favor escolha um número de 1 a 6:");
            }
            key='A';
        }
    }

    return OK;
}

int ConfigSamples (void)//Number of samples to be displayed configuration process
{
    PrintScreen();//refreshes screen info
    printf("\n\nDigite o número de amostras a serem exibidas no gráfico (de 1 a 10000):");
    printf("\n(Digite 0 para usar o modo automático)\n");
    fflush(stdin);
    scanf("%d",&N_Samples);

    while(N_Samples>10000 || N_Samples<0)//assures valid number of samples
    {
        printf("\nNúmero inválido. Por favor digite um número de 0 a 10000: ");
        fflush(stdin);
        scanf("%d",&N_Samples);
    }
    if(N_Samples==0)
    {
        N_Samples_is_Set=FALSE;
    }else
    {
        N_Samples_is_Set=TRUE;
    }

    return OK;
}

int ConfigCOM (void)//COM port configuration process
{
    char porta[2]={0,0};//port number

    PrintScreen();//refreshes screen info
    printf("\n\nDigite o número da COM que deve ser utilizada:");

    com[3]=0;
    com[4]=0;
    fflush(stdin);
    gets(&porta);
    while(((porta[0]<'1')||(porta[0]>'9'))||(((porta[1]<'0')||(porta[1]>'9'))&&(porta[1]!=0)))//assures valid port number (x or xx where x= 1 to 9)
    {
        printf("Número inválido. Por favor digite o número da COM que deve ser utilizada:");
        fflush(stdin);
        gets(porta);
    }

    strcat(com,porta);//sets COM port name COMxx

    CloseHandle(p_com);//closes COM handle for safety

    p_com=AbreComm(com,baud); //opens COM handle

    if(p_com==INVALID_HANDLE_VALUE) {//port could not be opened
        printf("A porta %s não pôde ser aberta\n", com);
        getch();
        strcpy(status,"Fechada");
        return ERROR;
    }

    return OK;
}

int StartAcquire (void)//Start data acquisition process
{
    unsigned char buffer[2]; //buffer for serial communication
    unsigned int buffer_int[2];
    unsigned int aux=0;
    time_t e_time;//elapsed time, for timeout diagnosis

    PrintScreen();//refreshes screen info
    printf("\n\nAguarde enquanto os dados estão sendo recebidos...");

    for(aux=0;aux<Num_of_Channels;aux++)//resets period info for each channel
    {
        Period[aux]=0;
    }

    e_time=time(NULL);//time start counting

    buffer[0]=DEFINE_CONFIG;//sends byte to micro with the DEFINE_CONFIG flag (next byte, acquisition will be configured)
    WriteFile(p_com,buffer, 1, &n, NULL);

    /*
    ====Configuration Byte Format====
    bit 0-3 (4 bits) -> Sampling Rate
    bit 4-6 (3 bits) -> Number of Channels
    bit 7 (1 bit) -> Number of Samples is set

    */

    buffer[0]=(Sent_Sampling_Rate|((char)Num_of_Channels<<4)|(N_Samples_is_Set<<7));//Sends acquisition configuration data
    WriteFile(p_com,buffer, 1, &n, NULL);

    for(aux=0;aux<2;aux++)//sends number of samples defined by user
    {
        buffer[0]=(N_Samples>>(aux*8));
        WriteFile(p_com,buffer,1,&n,NULL);
    }

    buffer[0]=START_OF_CONVERSION;//start conversion
    WriteFile(p_com,buffer, 1, &n, NULL);

    for(channel=0;channel<Num_of_Channels;channel++)//receives signal period (acquired by the micro), for each channel
    {
        for(aux=0;aux<2;aux++)
        {
            do{
                ReadFile(p_com,buffer,1,&n,NULL);
                if((time(NULL)-e_time)>TIMEOUT_ERROR)return ERROR;//communication timeout error
            }while(n==0);
            Period[channel]|=(buffer[0]<<(8*aux));
        }
    }

    N_Samples=0;
    for(aux=0;aux<2;aux++)//receives total number of samples acquired in each channel
    {
        do{
            ReadFile(p_com,buffer,1,&n,NULL);
            if((time(NULL)-e_time)>TIMEOUT_ERROR)return ERROR;
        }while(n==0);
        N_Samples|=(buffer[0]<<(8*aux));
    }

    channel=0;
    aux=0;
    while(aux<N_Samples)//receive samples (1st sample of all channels, 2nd sample of all channels and so on)
    {
        do{
            ReadFile(p_com,buffer,1,&n,NULL);
        }while(n==0);
        if(n)
        {
            array[aux][channel]=buffer[0];
            channel++;
            if(channel==Num_of_Channels)
            {
                channel=0;
                aux++;
            }
        }
        if((time(NULL)-e_time)>TIMEOUT_ERROR)return ERROR;
    }

    PrintScreen();//refreshes screen info
    printf("\n\nDados recebidos (disponíveis no arquivo \"Info\"):");

    for(channel=0;channel<Num_of_Channels;channel++)//exhibits period and frequency obtained by the micro (for each channel)
    {
        printf("\n\nCanal %d:",channel);
        if(Period[channel]==0)
        {
            printf("\nPeríodo: Indeterminado\nFrequência: Indeterminada");
        }else{
            printf("\nPeríodo: %fmS\nFrequência: %.2fHz",((float)Period[channel]/(float)Sampling_Rate*1000),((float)Sampling_Rate/(float)Period[channel]));
        }
    }

    return OK;
}

int PlotGraph (char* name)//Plots data received in the bitmap file
{
    unsigned int buffer_int[2];
    unsigned char buffer_char[2];
    int line=0;
    int column=0;
    int cont=0;

    unsigned int DataOffset;
    unsigned int Width;
    unsigned int Height;
    unsigned int ImageSize;

    printf("\n\nPor favor, aguarde enquanto o gráfico está sendo feito...");

    bitmap=fopen(name,"r+b");//opens binary file for read/write in binary

    fseek(bitmap,10,SEEK_SET);//gets Data Offset from header
    fread(buffer_int,4,1,bitmap);
    DataOffset=buffer_int[0];

    fseek(bitmap,18,SEEK_SET);//gets bitmap Width from header
    fread(buffer_int,4,1,bitmap);
    Width=buffer_int[0];

    fseek(bitmap,22,SEEK_SET);//gets bitmap Height from header
    fread(buffer_int,4,1,bitmap);
    Height=buffer_int[0];

    fseek(bitmap,34,SEEK_SET);//gets Image Size from header
    fread(buffer_int,4,1,bitmap);
    ImageSize=buffer_int[0];

    fseek(bitmap,DataOffset,SEEK_SET);//Sets cursor back to the beginning of file

    buffer_char[0]=BLACK;//Sets current color as black (axis and grid)

    for(line=1;line<=256;line++)//plots axis and grid in bitmap
    {
        for(column=1;column<=Width;column++)
        {
            if((column%10)==0)//dashed column in 10 time steps
            {
                if((line%3)==0)
                {
                    fwrite(buffer_char,1,1,bitmap);
                    fseek(bitmap,-1,SEEK_CUR);
                }
            }
            if((column%100)==0)//constant color column in 100 time steps
            {
                fwrite(buffer_char,1,1,bitmap);
                fseek(bitmap,-1,SEEK_CUR);
            }
            if((int)((float)line/(0.2/(ADC_RESOLUTION)))-(int)((float)(line-1)/(0.2/(ADC_RESOLUTION))))//dashed line in 0.2v steps
            {
                if(column%3==0)
                {
                    fwrite(buffer_char,1,1,bitmap);
                    fseek(bitmap,-1,SEEK_CUR);
                }
            }
            if((int)((float)line/(1/(ADC_RESOLUTION)))-(int)((float)(line-1)/(1/(ADC_RESOLUTION))))//constant color line in 1v steps
            {
                fwrite(buffer_char,1,1,bitmap);
                fseek(bitmap,-1,SEEK_CUR);
            }
            fseek(bitmap,1,SEEK_CUR);
        }
    }

    for(channel=0;channel<Num_of_Channels;channel++)//plots channels traces
    {
        fseek(bitmap,DataOffset,SEEK_SET);//beggining of data in th bitmap
        buffer_char[0]=Color[channel];//Sets current color as the channel color
        for(int aux=0;aux<N_Samples;aux++)//plot channel pixels
        {
            fseek(bitmap,DataOffset,SEEK_SET);//beggining of data in th bitmap
            fseek(bitmap,(array[aux][channel]*Width),SEEK_CUR);//sets cursor in correct line
            fseek(bitmap,aux,SEEK_CUR);//sets cursor in correct column
            fwrite(buffer_char,1,1,bitmap);//set pixel
            fseek(bitmap,-1,SEEK_CUR);//goes back 1 position (just written)

            if((aux>1) && (aux<9999))//if it's not the first or last sample, connect the current sample to the one next to it (always connects the lower one to the higher one)
            {
                while((array[aux][channel]<(array[aux-1][channel]-1))||(array[aux][channel]<(array[aux+1][channel]-1)))
                {
                    array[aux][channel]++;
                    cont++;//count number of pixels added
                    fseek(bitmap,Width,SEEK_CUR);
                    fwrite(buffer_char,1,1,bitmap);
                    fseek(bitmap,-1,SEEK_CUR);
                }
                array[aux][channel]-=cont;//returns back to the initial pixel
                cont=0;
            }
        }
    }

    fclose(bitmap);//closes ready bitmap
    printf("\n\nGráfico concluído, pressione qualquer tecla para continuar");
    getch();

    return OK;
}

int GenerateInfo (char* name)//Generates .txt file with acquisition info
{
    unsigned char value[2]={255,0};//value for the calculation of the Vpp in each channel

    text=fopen(name, "w");//opens file for writing

    fprintf(text,"Intervalo de tempo por divisão: %fmS",(float)(10000.f/Sampling_Rate));
    fprintf(text,"\nVolts por divisão: 0.2v");

    for(channel=0;channel<Num_of_Channels;channel++)
    {
        fprintf(text,"\n\nCanal %d",channel);
        value[MIN]=255;
        value[MAX]=0;
        if(Period[channel] == 0)//could not capture entire period
        {
            fprintf(text,"\nPeríodo: Indeterminado");
            fprintf(text,"\nFrequência: Indeterminada");
        }else//captured at least 1 complete period
        {
            fprintf(text,"\nPeríodo: %fmS (%d amostras)",((float)Period[channel]*1000/(float)Sampling_Rate),Period[channel]);
            fprintf(text,"\nFrequência: %.2fHz",((float)Sampling_Rate/(float)Period[channel]));
        }
        fprintf(text,"\nNúmero de amostras do gráfico: %d",N_Samples);

        for(int aux=0;aux<N_Samples;aux++)//gets min and max values of voltage
        {
            if(array[aux][channel]<value[MIN])value[MIN]=array[aux][channel];
            if(array[aux][channel]>value[MAX])value[MAX]=array[aux][channel];
        }

        fprintf(text,"\nTensão Pico a Pico (Vpp): %.4fV",(float)(value[MAX]-value[MIN])*(float)(ADC_RESOLUTION));//prints out Vpp in the .txt
    }

    fclose(text);//close file
    return OK;
}

int main(void)//main program menu
{
    char buffer =0;

    setlocale(LC_ALL,"Portuguese");

    char name[20]="Grafico";//name of the bitmap file
    strcat(name,".bmp");

    while(1)
    {
        PrintScreen();//refreshes screen info
        printf("\n\nPressione:");
        printf("\n[1] para iniciar a amostragem do sinal");
        printf("\n[2] para alterar as configurações do osciloscópio");
        printf("\n[3] para alterar o número de amostras");
        printf("\n[4] para editar a porta COM utilizada");

        buffer=getch();

        if(buffer=='1')//start acquisition
        {
            if(status[0]=='F')//tries to open COM port, if closed
            {
                p_com=AbreComm(com,baud);

                if(p_com==INVALID_HANDLE_VALUE){//could not open port
                    printf("\n\nA porta %s não pôde ser aberta\n", com);
                    getch();
                    strcpy(status,"Fechada");
                }else //port opnened sucessfully
                {
                    strcpy(status,"Aberta");
                }
            }
            if(status[0]=='A')//if COM port is opened
            {
                if(StartAcquire()==OK)//acquisition succesful
                {
                    CreateBMP(name,256,N_Samples);//creates bitmap
                    GenerateInfo("Info.txt");//creates info text file
                    PlotGraph(name);//plots graph in bitmap file
                }else//some error occurred during the acquisition
                {
                    CloseHandle(p_com);
                    strcpy(status,"Fechada");
                    PrintScreen();
                    printf("\n\nParece que ocorreu algum erro. Confira a conexão da porta COM e tente novamente.");
                    getch();
                }
            }
        }

        if(buffer=='2')//Osc configuration edit
        {
            ConfigOSC();
        }

        if(buffer=='3')//Number of samples edit
        {
            ConfigSamples();
        }

        if(buffer=='4')//Com port configuration edit
        {
            if(ConfigCOM()==OK)
            {
                strcpy(status,"Aberta");
            }
        }

        if(buffer==27)//closes application if 'Esc' is pressed
        {
            break;
        }
    }

    CloseHandle(p_com); //closes COM handle

    return 0;
}
