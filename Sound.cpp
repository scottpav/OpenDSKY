/*
 * Sound.cpp
 *
 * Copyright 2018 Jeffrey Marten Gillmor
 *
 * OpenDSKY is a trademark of S&T GeoTronics LLC  https://opendsky.com
 *
 * Driver to allow sounds to be played either the skip to next track button
 * can be used, or optionally with some wire mods to the openDSKY, the
 * softare serial can be used to TX commands.
 *
 * NASA have made some great sounds avialable here:
 * https://www.nasa.gov/connect/sounds/index.html
 *
 * Don't forget to update the TracksEnum in Sound.h to match the tracks you
 * have put onto the microSDcard.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * (GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <DFPlayerMini_Fast.h>
#include "Sound.h"

DFPlayerMini_Fast myMP3;

void soundSetup() 
{
	Serial.begin(9600);
  
  myMP3.begin(Serial);
 
 

  myMP3.sleep();
  delay(2000);

  myMP3.wakeUp();
}

void playTrack(uint16_t track)
{
  myMP3.volume(30);
  delay(20);
  myMP3.play(track);
  delay(1000);
}
