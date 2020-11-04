#include <stdio.h>
#include <stdlib.h>

#include "D400s/d400s.h"

int main(void)
{
	printf("Sample D455\n");
	D400s d455;
	int ret = d455.init();
	if(ret == -1) return -1;

	printf("Opened RealSense device\n");
	Frames_t frames;
	int key = 0;
	while(key != 27){
		if(d455.getFrames(frames) == -1) break;
		cv::imshow("RGB", frames.imgRGB);
		key = cv::waitKey(1);
	}

	return 0;
}
