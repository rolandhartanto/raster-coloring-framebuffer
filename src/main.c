/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:

retrieved from:
TesXting the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)

http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#define HEIGHT 500
#define WIDTH 800
#define INIT_HEIGHT 100
#define INIT_WIDTH 100
#define FONT_HEIGHT 30
#define FONT_WIDTH 30

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

typedef struct{
    int r1,c1; //koord. titik awal
    int r2,c2; //koord. titik akhir
}line;

typedef struct{
    char font; //huruf apa
    int nline; //jumlah garis
    line border[40];
}letter;

letter alphabet[26];
char bg[1000][1000];

char *fbp = 0;
int fbfd = 0;
long int screensize = 0;
long int location = 0;

void init();
void clearScreen();
void printpixelBG(int x, int y, int colorR, int colorG, int colorB);
void printPixel(int x, int y, int color);
void bresLine(int x1, int y1, int x2, int y2, int thickness);
void drawLetter(int roffset, int coffset, letter x);
void swap(int* a, int* b);

int main() {
    // Open the file for reading and writing

    init();   
    int i,j;
    FILE *ffont;
    ffont = fopen("data/font.txt","r");
    if(ffont == NULL) {
        printf("No data in font.txt\n");
        return 0;
    }
    else {
        int xa,xb,ya,yb;
        int num_of_line;
        printf("ok\n");
        for(i = 0; i < 26; i++){
            char dummy;
            fscanf(ffont,"\n%c",&dummy);
            printf("dummy: %c\n",dummy);
            fscanf(ffont, "%d",&num_of_line);
            alphabet[i].font = dummy;
            alphabet[i].nline = num_of_line;
            for(j = 0; j<num_of_line; j++){
                fscanf(ffont, "%d %d %d %d",&xa,&ya,&xb,&yb);
                //printf("%d %d %d %d\n",xa,ya,xb,yb);
                alphabet[i].border[j].r1 = xa;
                alphabet[i].border[j].c1 = ya;
                alphabet[i].border[j].r2 = xb;
                alphabet[i].border[j].c2 = yb;

            }
        }
    }
    char input[1000];
    scanf("%[^\n]s",input);

    //make upper case
    for(i=0;i<strlen(input);i++){
        char kar = input[i];
        if(kar>=97 && kar<=122){
            kar-=32;
        }
        input[i] = kar;
    }

    //printf("input: %s\n", input);
    clearScreen();

    int kolom = 700;
    //bresLine(500,500,0,0,1);
    //bresLine(0,0,500,100,1);
    //bresLine(500,0,500,100,1);
    //bresLine(0,100,500,100,1);
    //bresLine(100,0,0,300,1);
    //bresLine(316,300,325,329,1);
    //bresLine(216,200,234,258,1);      

    int xstart = 50;
    int ystart = 50;
    int cursor = 0;  
    i = 0;    
    while(i < strlen(input)) {
        for(int j = 0; j < 26; j++) {
            if(input[i] == alphabet[j].font) {
                //printf("%c\n", input[i]);
                drawLetter(ystart,xstart,alphabet[j]);
            }
        }
        xstart+=50;
        i++;
        cursor++;
        if(cursor == 14) {
            ystart+=50;
            xstart=50;
            cursor=0;
        }
    }
    munmap(fbp, screensize);
    close(fbfd);

    return 0;
}

void init(){
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

}

//clearScreen
void clearScreen() {	//BackGround Screen
    for (int h = 0; h < HEIGHT; h++){
        for (int w = 0; w < WIDTH; w++) {
				printpixelBG(w,h,255,255,255);
        }
    }
}

void printpixelBG(int x, int y, int colorR, int colorG, int colorB){	//Print Pixel Color using RGB
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location) = colorB;			//Blue Color
        *(fbp + location + 1) = colorG;		//Green Color
        *(fbp + location + 2) = colorR;		//Red Color
        *(fbp + location + 3) = 0;			//Transparancy
    }
}

void printPixel(int x, int y, int color){
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location) = color;
        *(fbp + location + 1) = color;
        *(fbp + location + 2) = color;
        *(fbp + location + 3) = 0;
    }
}

void bresLine(int x_1, int y_1, int x_2, int y_2, int thickness){
    int x1 = x_1, y1 = y_1, x2 = x_2, y2 = y_2;
    int steep = 0;
    if(abs(x1-x2) < abs(y1-y2)){
        swap(&x1, &y1);
        swap(&x2, &y2);
        steep = 1;
    }
    if(x1 > x2){
        swap(&x1,&x2);
        swap(&y1,&y2);
    }
    int dx = x2-x1;
    int dy = y2-y1;
    int derr = 2 * abs(dy);
    int err = 0;
    int y = y1;
    for(int x = x1; x <= x2; x++){
        if(steep){
            printPixel(y,x,0);
        }else{
            printPixel(x,y,0);
        }
        err+=derr;
        if(err > dx){
            y += (y2>y1)?1:-1;
            err -= 2 * dx;
        }
    }
}

void getPixelColor(int x, int y, int *rColor, int *gColor, int *bColor) {
      location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;
            *rColor = *(fbp+location);
            *gColor = *(fbp+location+1);
            *bColor = *(fbp+location+2);
}

int isBlack(int x, int y) {
    int rColor;
    int gColor;
    int bColor;
    getPixelColor(x,y,&rColor, &gColor, &bColor);
    return(rColor == 0 && gColor == 0 && bColor == 0);
}

void rasterize(int roffset, int coffset, char font) {
    unsigned char onFlag = 0;
    unsigned char started = 0;
    for(int i = 0; i < FONT_HEIGHT; i++) {
        int arr[30];
        int nPoint = 0;
        int y = roffset + i;

        for(int j = 0; j < FONT_WIDTH; j++) {
            int x = coffset + j;
            if(isBlack(x,y)) {
                arr[nPoint] = j;
                while(isBlack(x,y)) {
                    j++;
                    x = coffset + j;
                }
                nPoint++;
            }
        }
        //printf("i: %d, nPoint: %d\n", i, nPoint);
        int median = -1;
        if(font == 'G' && i==15) {
            nPoint--;
        } else if(font == 'J' && i==18) {
            arr[0] = arr[1];
            arr[1] = arr[2];
            nPoint--;
        } else if(font == 'W' && i==22) {
            arr[1] = arr[3];
            nPoint-=2;
        }
        else if (font == 'M' && i==3) {
            //printf("nPoint = %d\n", nPoint);
            arr[1] = arr[2];
            arr[4] = arr[5];
            //arr[6] = arr[2];
            nPoint--;
        }
        else if (font == 'M' && i==10) {
            //printf("nPoint = %d\n", nPoint);
            arr[3] = arr[4];
            arr[4] = arr[5];
            arr[5] = arr[6];
            nPoint--;
        }
        else if(font == 'N' && i == 7) {
            //arr[0] = arr[1];
            arr[1] = arr[2];
            arr[2] = arr[3];
            arr[3] = arr[4];
            nPoint--;
        }
        else if (font == 'N' && i==22) {
            //printf("nPoint = %d\n", nPoint);
            arr[3] = arr[4];
            arr[4] = arr[5];
            nPoint--;
        }
        else if (font == 'W' && i==22) {
            //printf("nPoint = %d\n", nPoint);
            arr[2] = arr[3];
            arr[3] = arr[4];
            nPoint--;
        }
        if(nPoint % 2 != 0) {
            median = nPoint / 2;
        }
       
        //printf("median: %d\n", median);
        //printf("Start Printing::\n");
        if(nPoint > 1 && i!=0 && i!=29) {
            for(int it = 0; it < nPoint-1; it+=2) {
                if(it == median) {
                    it++;
                }
                int startPoint = it;
                int endPoint = it+1;
                if(endPoint == median)
                    endPoint++;
                //printf("SP: %d %d EP %d %d\n", startPoint, arr[startPoint], endPoint, arr[endPoint]);
                if(endPoint < nPoint){
                    if(arr[endPoint] > arr[startPoint]){
                        for(int jt = arr[startPoint]; jt < arr[endPoint];jt++){
                            int x = coffset + jt;
                            printpixelBG(x,y,0,0,0);
                        }
                    }
                }
            }
        }
    }
}

void drawLetter(int roffset, int coffset, letter x){
    int i,j;
    for(i = 0; i < x.nline; i++){
        // printf("%d %d\n",x.border[i].r1 + roffset, x.border[i].c1 + coffset);
        // printf("%d %d\n",x.border[i].r2 + roffset, x.border[i].c2 + coffset);
        bresLine(x.border[i].c1 + coffset, x.border[i].r1 + roffset,
                 x.border[i].c2 + coffset, x.border[i].r2 + roffset, 1);
    }
    rasterize(roffset, coffset, x.font);
}


void swap(int* a, int* b){
    int temp = *a;
    *a = *b;
    *b = temp;
}
