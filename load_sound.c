#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <vorbis/vorbisfile.h>

#define BUFFER_SIZE (48000)

int snd_load_file(char const * file, ALuint buffer){
	FILE*           oggFile;
	OggVorbis_File  oggStream;
	vorbis_info*    vorbisInfo;
	vorbis_comment* vorbisComment;
	ALenum format;

	int result;

	if(!(oggFile = fopen(file, "rb"))){
		printf("Could not open Ogg file %s.", file);
        return 1;
    }

	if((result = ov_open(oggFile, &oggStream, NULL, 0)) < 0)
	{
		fclose(oggFile);
		printf("Could not open Ogg stream.\n");
		return 1;	
	}

	vorbisInfo = ov_info(&oggStream, -1);
	vorbisComment = ov_comment(&oggStream, -1);

	if(vorbisInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;
		
		
	char * dyn_data = NULL;
	int  size = 0;
	int  section;
	result = 1;
	printf("Loading sound file %s\n", file);
	while(result > 0){
		char data[BUFFER_SIZE];
		result = ov_read(&oggStream, data, BUFFER_SIZE, 0, 2, 1, & section);
		if(result > 0){
			size += result;
			dyn_data = (char*)realloc(dyn_data, sizeof(char)*(size));
			memcpy(dyn_data+size-result, data, result);
		} else if(result < 0){
			switch(result){
				case OV_HOLE:
					printf("Interruption in the data.%d\n", result);
					printf("one of: garbage between pages, loss of sync followed by recapture, or a corrupt page\n");
					break;
				case OV_EBADLINK:
					printf("an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.\n");
					break;
				case OV_EINVAL:
					printf("the initial file headers can't be read or are corrupt, or the initial open call for vf failed.\n");
					break;
			}

		} else if(size == 0){
			printf("Date not read.\n");
		}
	}

	
	alBufferData(buffer, format, dyn_data, size, vorbisInfo->rate);
	free(dyn_data);
	ov_clear(&oggStream);
    fclose(oggFile);
	return 0;
}

