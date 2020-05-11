#ifndef SOUND_H
#define SOUND_H


typedef enum TRACKS_ENUM
{
		HOUSTON,
		COUNTDOWN,
		LANDING,
		NUM_TRACKS
}TracksEnum;

extern void soundSetup(void);
extern void playTrack(uint16_t track);
#endif
