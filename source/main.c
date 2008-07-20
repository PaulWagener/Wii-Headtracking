/**
 * Wii Headtracking with wiimote infrared camera
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "GRRLIB/GRRLIB.h"
Mtx GXmodelView2D;

#include "gfx/view.h"
#include "GRRLIB/fonts/GRRLIB_font1.h"

//The size of the source image (gfx/view.h)
#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600

//The size of the TV
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/**
 * Wraps the GRRLIB_DrawImg function with parameters for width & height instead of scaleX & scaleY
 */
void DrawImg(f32 x, f32 y, f32 width, f32 height, u8 data[])
{
	float scaleX = width / IMAGE_WIDTH;
	float scaleY = height / IMAGE_HEIGHT;

	//Adjust for the fact that images scale at their center
	float newX = x - (IMAGE_WIDTH / 2) + (width / 2);
	float newY = y - (IMAGE_HEIGHT / 2) + (height / 2); 
	
	GRRLIB_DrawImg(newX, newY, IMAGE_WIDTH, IMAGE_HEIGHT, data, 0, scaleX, scaleY, 255);
} 

int main(){
	//Load textures
	u8 *tex_logo = GRRLIB_LoadTexture(view);			//The primary image being displayed
    u8 *tex_font1 = GRRLIB_LoadTexture(GRRLIB_font1);	//Font graphics

	//Initialize controllers (both Wii & Gamecube)
    WPAD_Init();

	//Initialize video
    VIDEO_Init();
    GRRLIB_InitVideo();
    GRRLIB_Start();

	float zoom = 0, absolute_zoom = 0, width = 0, height = 0, x = 0, y = 0, absolute_x = 0, absolute_y = 0;
	int debug = 0;
	
	//Distance between the wiimote and the sensorbar in meters
	float min_distance = 1.5;	//Distance at which the entire image is visible on the screen
	float max_distance = 4.6;	//Distance at which the image is at maximum zoom level

	float max_zoom = 2.0;		//Defines how much bigger the image will be when the sensorbar
								//is at maximum distance
							
    while(1){
        
		//Read Wiimote data
		WPAD_ScanPads();
		WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
		WPAD_SetVRes(0, 640, 480);
		
		u32 type;
		int res = WPAD_Probe(0, &type);
		if(res == WPAD_ERR_NONE) {
			WPADData *wd = WPAD_Data(0);

			if(wd->ir.valid) {
				//Calculate amount of zoom to be used on the image
				//0.0 means whole image is visible (when the sensorbar is closest)
				//1.0 means maximum zoom (when the sensorbar is furthest)
				zoom = (wd->ir.z - min_distance) / (max_distance - min_distance);
				if(zoom < 0)
					zoom = 0;
				
				if(zoom > 1)
					zoom = 1;
				
				//Amount of zoom the image will be stretched, number between 1.0 and max_zoom
				absolute_zoom = zoom * (max_zoom - 1.0) + 1.0;
				
				//width and height of image on the tvscreen
				width = SCREEN_WIDTH * absolute_zoom;
				height = SCREEN_HEIGHT * absolute_zoom;
				
				//Calculate relative placement of the image on the screen
				//(x=0.0, y=0.0 means the topleft part of the zoomed image is visible)
				x = wd->ir.x / SCREEN_WIDTH;
				y = wd->ir.y / SCREEN_HEIGHT;
				x = 1.0 - x;			
				
				//Calculate the absolute topleft position of the image on the tvscreen
				absolute_x = x * -(width - SCREEN_WIDTH);
				absolute_y = y * -(height - SCREEN_HEIGHT);
			}
			
			//Drawing the actual image
			DrawImg(absolute_x, absolute_y, width, height, tex_logo);

			//Debugging info
			if(debug)
			{
				//Variabeles
				GRRLIB_Printf(20,20,tex_font1,0xFFFFFFFF,2,"Distance: %.02f meters", wd->ir.z);
				GRRLIB_Printf(20,40,tex_font1,0xFFFFFFFF,2,"Zoom used: %.03f (%.03f)", zoom, absolute_zoom);
				GRRLIB_Printf(20,60,tex_font1,0xFFFFFFFF,2,"Image placement: %.02f, %.02f", x, y);
				GRRLIB_Printf(20,80,tex_font1,0xFFFFFFFF,2,"Absolute placement: %.02f, %.02f", absolute_x, absolute_y);
				GRRLIB_Printf(20,100,tex_font1,0xFFFFFFFF,2,"Image Size: %.02f, %.02f", width, height);
				
				//Settings
				GRRLIB_Printf(20,200,tex_font1,0xFFFFFFFF,2,"Settings");
				GRRLIB_Printf(20,220,tex_font1,0xFFFFFFFF,2,"min_distance: %0.2f", min_distance);
				GRRLIB_Printf(20,240,tex_font1,0xFFFFFFFF,2,"max_distance: %0.2f", max_distance);
				GRRLIB_Printf(20,260,tex_font1,0xFFFFFFFF,2,"max_zoom: %0.2f", max_zoom);
				
				//Draw the actual pointer
				GRRLIB_Rectangle(wd->ir.x - 5, wd->ir.y - 5, 10, 10, 0xFFFFFFFF, 1);
			}
		}
		
        GRRLIB_Render();

		//Check for input from both Wiimotes
		u32 wpaddown = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1);
		
        if (wpaddown & WPAD_BUTTON_HOME) exit(0);
		if (wpaddown & WPAD_BUTTON_A) debug = (debug == 0) ? 1 : 0;
		
		//Adjust settings
		if (wpaddown & WPAD_BUTTON_LEFT && min_distance > 0.2) min_distance -= 0.2;
		if (wpaddown & WPAD_BUTTON_RIGHT) min_distance += 0.2;
		
		if (wpaddown & WPAD_BUTTON_DOWN && max_distance > min_distance + 0.2) max_distance -= 0.2;
		if (wpaddown & WPAD_BUTTON_UP) max_distance += 0.2;
		
		if (wpaddown & WPAD_BUTTON_MINUS && max_zoom > 1.2) max_zoom -= 0.2;
		if (wpaddown & WPAD_BUTTON_PLUS) max_zoom += 0.2;
    }
    return 0;
}
