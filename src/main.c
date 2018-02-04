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

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

typedef struct{
    int r1,c1; //koord. titik awal
    int r2,c2; //koord. titik akhir
}line;

typedef struct{
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

int main() {
    // Open the file for reading and writing

    init();   
    int i,j;
    FILE *ffont;
    ffont = fopen("../data/font.txt","r");
    if(ffont == NULL) {
        printf("No data in font.txt\n");
        return 0;
    }
    else {
        int xa,xb,ya,yb;
        int num_of_line;
        printf("ok\n");
        for(i = 0; i < 7; i++){
            char dummy;
            fscanf(ffont,"\n%c",&dummy);
            printf("dummy: %c\n",dummy);
            fscanf(ffont, "%d",&num_of_line);
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

    clearScreen();
    int kolom = 700;
    //bresLine(0,0,400,200,1);   
    while(1){
    	drawLetter(100,100,alphabet[6]);
    }
    /*
    while(1){
        clearScreen();
        int idx = i+1;
        int i = 0;
        printf("%d\n", i);
        // Garis vertikal atau miring
        while(i < idx) {
            printf("%d\n", i);
            bresLine(atoi(bg[i]), atoi(bg[i+1]), atoi(bg[i+2]), atoi(bg[i+3]), 2);
            printf("%d\n", i);
            i = i + 4;
        }
        i = 0;
        // TODO: Garis horizontal
        

    }*/
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

void bresLine(int x1, int y1, int x2, int y2, int thickness){
    int dx, dy, x, y, x_end, y_end, p, const1, const2, i;
    for(i = 0; i < thickness; i++){
	if(((x1-x2 > 0)&&(y1-y2 > 0))||((x1-x2 < 0)&&(y1-y2 < 0))){
            dx = abs(x1-x2);
            dy = abs(y1-y2);
    
            p = 2 * dy - dx;
            const1 = 2 * dy;
            const2 = 2 * (dy-dx);

            if(x1 > x2){
                x = x2 + i;
                y = y2;
                x_end = x1 + i;
            }else{
                x = x1 + i;
                y = y1;
                x_end = x2 + i;
            }
    
            printPixel(x,y,0);
            while(x < x_end){
                x++;
                if(p < 0){
                    p = p + const1;
                }else{
                    y++;
                    p = p + const2;
                }
        
                printPixel(x,y,0);
            }
        }else if(((x1-x2 < 0)&&(y1-y2 > 0))||((x1-x2 > 0)&&(y1-y2 < 0))){ //gradien negatif
            dx = abs(x1-x2);
            dy = abs(y1-y2);
    
            p = 2 * dy - dx;
            const1 = 2 * dy;
            const2 = 2 * (dy-dx);

            if(x1 > x2){
                x = x2 + i;
                y = y2;
                x_end = x1 + i;
            }else{
                x = x1 + i;
                y = y1;
                x_end = x2 + i;
            }
    
            printPixel(x,y,0);
            while(x < x_end){
                x++;
                if(p < 0){
                    p = p + const1;
                }else{
                    y--;
                    p = p + const2;
                }
        
                printPixel(x,y,0);
            }
        }else if(x1-x2 == 0){ //gradien tak hingga
            y_end = (y1 > y2)? y1:y2;
            y = (y1 > y2)? y2:y1;
            for(int j=y; j<y_end; j++){
                printPixel(x1+i,j,0);
            }
        }else if(y1-y2 == 0){ //gradien 0
            x_end = (x1 > x2)? x1:x2;
            x = (x1 > x2)? x2:x1;
            for(int j=x; j<x_end; j++){
                printPixel(j,y1+i,0);
            }
        }
        
    }
}

void drawLetter(int roffset, int coffset, letter x){
    int i,j;
    for(i = 0; i < x.nline; i++){
        //printf("%d %d\n",x.border[i].r1 + roffset, x.border[i].c1 + coffset);
        //printf("%d %d\n",x.border[i].r2 + roffset, x.border[i].c2 + coffset);
        bresLine(x.border[i].c1 + coffset, x.border[i].r1 + roffset,
                 x.border[i].c2 + coffset, x.border[i].r2 + roffset, 1);
    }
}
