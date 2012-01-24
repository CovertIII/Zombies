#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <vorbis/vorbisfile.h>
#include "stream_sound.h"

#define LOAD_BUFFER_SIZE (4096)
#define BUFFER_SIZE (48000)

typedef struct al_stream_type {
	FILE * oggFile;
	OggVorbis_File  oggStream;
    ALsizei freq;
	ALenum format;

	ALuint buffer[3];
    ALuint source;
} al_stream_type;

al_stream al_stream_init(void){
    al_stream alst;

    alst = (al_stream_type*)malloc(sizeof(al_stream_type));

    if(alst == NULL){return NULL;}

    return alst;
}

int al_stream_load_file(al_stream als, const char * file){
	FILE*           oggFile;
	OggVorbis_File  oggStream;
	vorbis_info*    vorbisInfo;
	vorbis_comment* vorbisComment;
	ALenum format;

	int result;

	if(!(oggFile = fopen(file, "rb"))){
		printf("Could not open Ogg file: %s\n", file);
		return 0;
	}

	if((result = ov_open(oggFile, &oggStream, NULL, 0)) < 0)
	{
		fclose(oggFile);
		printf("Could not open Ogg stream.\n");
		return 0;
	}

	vorbisInfo = ov_info(&oggStream, -1);
	vorbisComment = ov_comment(&oggStream, -1);

	if(vorbisInfo->channels == 1){
		als->format = AL_FORMAT_MONO16;
	}
	else{
		als->format = AL_FORMAT_STEREO16;
	}	
    als->freq = vorbisInfo->rate;
    als->oggStream = oggStream;
    als->oggFile = oggFile;

	alGenBuffers(3, als->buffer);
    int error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        printf("alBufferData buffer 0 %d: ", error);
    }
	alGenSources(1, &als->source);
    if ((error = alGetError()) != AL_NO_ERROR) {
        printf("alBufferData buffer 0 %d: ", error);
    }

	alSource3f(als->source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(als->source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(als->source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (als->source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcef (als->source, AL_GAIN,  0.00          );
	alSourcei (als->source, AL_SOURCE_RELATIVE, AL_TRUE      );

    load_buffer(als, als->buffer[0]);
    load_buffer(als, als->buffer[1]);
    load_buffer(als, als->buffer[2]);

    alSourceQueueBuffers(als->source, 3, als->buffer);

    if ((error = alGetError()) != AL_NO_ERROR) {
        printf("alBufferData buffer 0 %d: ", error);
    }

    return 1;
}

int al_stream_play(al_stream als){
    int error;
    alSourcePlay(als->source);
    if ((error = alGetError()) != AL_NO_ERROR) {
        printf("alBufferData buffer 0 %d: ", error);
    }
    ALint state;
    alGetSourcei(als->source, AL_SOURCE_STATE, &state);
    if (state != AL_STOPPED)
    {
        printf("playing\n");
    }
}

int al_stream_update(al_stream als){
    int state;
    int processed;
    alGetSourcei(als->source, AL_BUFFERS_PROCESSED, &processed);
    while(processed > 0){
        ALuint buffer;
        alSourceUnqueueBuffers(als->source, 1, &buffer);
        if(load_buffer(als, buffer)){
            alSourceQueueBuffers(als->source, 1, &buffer);
        }
        processed--;
    }
    alGetSourcei(als->source, AL_SOURCE_STATE, &state);
    return state;
}

int al_stream_transition(al_stream als, double sec, const char * file){

}

int al_stream_free_file(al_stream als){
    //Detached all buffers qued on the source
    int qued_buffers;
    ALuint buffer;
    alGetSourcei(als->source, AL_BUFFERS_QUEUED, &qued_buffers);
    alSourceUnqueueBuffers(als->source, 2, &buffer);

	ov_clear(&als->oggStream);
    fclose(als->oggFile);
}

int load_buffer(al_stream als, ALuint buf){
	char * dyn_data = NULL;
	int  size = 0;
	int  section;
	int result = 1;
	while(result > 0 && size < BUFFER_SIZE){
		char data[LOAD_BUFFER_SIZE];
		result = ov_read(&als->oggStream, data, LOAD_BUFFER_SIZE, 0, 2, 1, & section);
		if(result > 0){
			size += result;
			dyn_data = (char*)realloc(dyn_data, sizeof(char)*(size));
			memcpy(dyn_data+size-result, data, result);
		} else if(result < 0){
			switch(result){
				case OV_HOLE:
					printf("\nInterruption in the data.%d\n", result);
					printf("one of: garbage between pages, loss of sync followed by recapture, or a corrupt page\n");
					break;
				case OV_EBADLINK:
					printf("\nAn invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.\n");
					break;
				case OV_EINVAL:
					printf("\nThe initial file headers can't be read or are corrupt, or the initial open call for vf failed.\n");
					break;
			}

		} else if(size == 0){
			//printf("Data not read.  ");
		}
	}

	//printf("Read %d bytes. \n ", size);
	
	alBufferData(buf, als->format, dyn_data, size, als->freq);
    int error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        printf("alBufferData buffer 0 %d: ", error);
    }

	free(dyn_data);
    return size;
}

