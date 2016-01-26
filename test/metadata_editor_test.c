/*
* Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <metadata_editor.h>

#define SAFE_FREE(src)		{ if (src) {free(src); src = NULL; } }

int dummy;

void __flush();
void __printRetValue(const char *func_name, int result);
static bool __get_tag_info(metadata_editor_h metadata);
static bool __write_tag_info(metadata_editor_h metadata);
static bool __add_picture(metadata_editor_h metadata);
static bool __delete_pictures(metadata_editor_h metadata);

void __flush()
{
	int c;
	while ((c = getc(stdin)) != 10);
}

void __printRetValue(const char *func_name, int result)
{
	printf("In function %s result is ", func_name);
	if (result == METADATA_EDITOR_ERROR_NONE)				printf("METADATA_EDITOR_ERROR_NONE\n");
	else if (result == METADATA_EDITOR_ERROR_INVALID_PARAMETER)	printf("METADATA_EDITOR_ERROR_INVALID_PARAMETER\n");
	else if (result == METADATA_EDITOR_ERROR_OUT_OF_MEMORY)		printf("METADATA_EDITOR_ERROR_OUT_OF_MEMORY\n");
	else if (result == METADATA_EDITOR_ERROR_FILE_EXISTS)		printf("METADATA_EDITOR_ERROR_FILE_EXISTS\n");
	else if (result == METADATA_EDITOR_ERROR_OPERATION_FAILED)	printf("METADATA_EDITOR_ERROR_OPERATION_FAILED\n");
	else								printf("Unallowed value. Error!\n");
}

static bool __get_tag_info(metadata_editor_h metadata)
{
	int ret = METADATA_EDITOR_ERROR_NONE;

	char *artist = NULL;
	char *title = NULL;
	char *album = NULL;
	char *genre = NULL;
	char *composer = NULL;
	char *copyright = NULL;
	char *date = NULL;
	char *description = NULL;
	char *comment = NULL;
	char *track_num = NULL;
	char *picture_index = NULL;
	int picture_size = 0;
	char *picture_type = NULL;
	void *picture = NULL;
	char *conductor = NULL;
	char *lyric = NULL;

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_ARTIST, &artist);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("1. - artist = [%s]\n", artist);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_TITLE, &title);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("2. - title = [%s]\n", title);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_ALBUM, &album);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("3. - album = [%s]\n", album);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_GENRE, &genre);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("4. - genre = [%s]\n", genre);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_AUTHOR, &composer);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("5. - composer = [%s]\n", composer);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_COPYRIGHT, &copyright);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("6. - copyright = [%s]\n", copyright);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_DATE, &date);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("7. - date = [%s]\n", date);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_DESCRIPTION, &description);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("8. - description = [%s]\n", description);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_COMMENT, &comment);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("9. - comment = [%s]\n", comment);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_TRACK_NUM, &track_num);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("10. - track_num = [%s]\n", track_num);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_PICTURE_NUM, &picture_index);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	printf("Picture index is [%s]\n", picture_index);
#if 0
	if ((filetype == METADATA_EDITOR_TYPE_OGG_VORBIS) || (filetype == METADATA_EDITOR_TYPE_OGG_FLAC)) {
		if (ret != METADATA_EDITOR_ERROR_INVALID_PARAMETER)
			printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	} else {
#endif
		if (ret != METADATA_EDITOR_ERROR_NONE)		printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
		else if ((ret == METADATA_EDITOR_ERROR_NONE) && picture_index) {
			uint num, i;
			num = atoi(picture_index);
			printf("Number of pictures: %u\n", num);
			for (i = 0; i < num; ++i) {
				ret = metadata_editor_get_picture(metadata, i, &picture, &picture_size, &picture_type);
				/*__printRetValue("metadata_editor_get_picture(...)", ret); */
				if (ret == METADATA_EDITOR_ERROR_NONE && picture) {
					printf("Saving picture number %u\n", i);
					int size = 30;
					char picture_file_name[size];
					snprintf(picture_file_name, size, "outputFile_%u" , i + 1);
					if (strncmp(picture_type, "image/jpeg", strlen("image/jpeg")) == 0)		strncat(picture_file_name, ".jpg", strlen(".jpg"));
					else if (strncmp(picture_type, "image/png", strlen("image/jpeg")) == 0)	strncat(picture_file_name, ".png", strlen(".png"));
					FILE *fout = fopen(picture_file_name, "wb");
					if (fout) {
						fwrite(picture, picture_size, 1, fout);
						fclose(fout);
					}
				} else
					printf("Error occured while picture extraction\n");
			}
			free(picture_index);
		}
#if 0
	}
#endif
	printf("11. - picture size = [%u]\n", picture_size);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_CONDUCTOR, &conductor);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("12. - conductor = [%s]\n", conductor);

	ret = metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_UNSYNCLYRICS, &lyric);
	/*__printRetValue("metadata_editor_get_metadata(...)",ret); */
	if (ret != METADATA_EDITOR_ERROR_NONE)	printf("Fail metadata_editor_get_metadata() at line [%d]\n", __LINE__);
	printf("13. - lyric = [%s]\n", lyric);

	SAFE_FREE(artist);
	SAFE_FREE(title);
	SAFE_FREE(album);
	SAFE_FREE(genre);
	SAFE_FREE(composer);
	SAFE_FREE(copyright);
	SAFE_FREE(date);
	SAFE_FREE(description);
	SAFE_FREE(comment);
	SAFE_FREE(track_num);
	SAFE_FREE(picture);
	SAFE_FREE(conductor);
	SAFE_FREE(lyric);

	return true;
}

static bool __write_tag_info(metadata_editor_h metadata)
{

	char input_data[400];

	printf("\n===========================================");
	printf("\nPlease, type in the following tag data: ");
	printf("\n===========================================\n");

	printf("\n 1. - Writing artist: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_ARTIST, input_data);
	*input_data = '\0';
	printf("2. - Writing title: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_TITLE, input_data);
	*input_data = '\0';
	printf("3. - Writing album: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_ALBUM, input_data);
	*input_data = '\0';
	printf("4. - Writing genre: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_GENRE, input_data);
	*input_data = '\0';
	printf("5. - Writing composer: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_AUTHOR, input_data);
	*input_data = '\0';
	printf("6. - Writing copyright: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_COPYRIGHT, input_data);
	*input_data = '\0';
	printf("7. - Writing year: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_DATE, input_data);
	*input_data = '\0';
	printf("8. - Writing description: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_DESCRIPTION, input_data);
	*input_data = '\0';
	printf("9. - Writing track: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_TRACK_NUM, input_data);
	*input_data = '\0';
	printf("10. - Writing comment: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_COMMENT, input_data);
	*input_data = '\0';
	printf("11. - Writing conductor: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_CONDUCTOR, input_data);
	*input_data = '\0';
	printf("12. - Writing lyrics: ");
	dummy = scanf("%[^\n]", input_data);
	__flush();
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_UNSYNCLYRICS, input_data);
	*input_data = '\0';

	metadata_editor_update_metadata(metadata);

	return true;
}

static bool __delete_tag_info(metadata_editor_h metadata)
{
	printf("\n 1. - Deleting artist: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_ARTIST, "");
	printf("\n 2. - Deleting title: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_TITLE, 0);
	printf("\n 3. - Deleting album: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_ALBUM, "");
	printf("\n 4. - Deleting genre: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_GENRE, 0);
	printf("\n 5. - Deleting composer: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_AUTHOR, "");
	printf("\n 6. - Deleting copyright: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_COPYRIGHT, 0);
	printf("\n 7. - Deleting year: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_DATE, "");
	printf("\n 8. - Deleting description: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_DESCRIPTION, 0);
	printf("\n 9. - Deleting track: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_TRACK_NUM, "");
	printf("\n 10. - Deleting comment: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_COMMENT, 0);
	printf("\n 11. - Deleting conductor: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_CONDUCTOR, "");
	printf("\n 12. - Deleting lyrics: ");
	metadata_editor_set_metadata(metadata, METADATA_EDITOR_ATTR_UNSYNCLYRICS, 0);

	metadata_editor_update_metadata(metadata);
	return true;
}

static bool __add_picture(metadata_editor_h metadata)
{
	uint c2 = 0;
	char *picture_filename = NULL;

	printf("\n=============================");
	printf("\n Choose picture: PNG or JPG ");
	printf("\n=============================");
	printf("\n |  TestImage.png  ->  1    | ");
	printf("\n |  TestImage.jpg  ->  2    | ");
	printf("\n Your choice : ");

	dummy = scanf("%u",  &c2);
	__flush();

	switch (c2) {
	case 1:
		printf("\n===========================");
		printf("\n Your choice is TestImage.png\n");
		picture_filename = strdup("TestImage.png");
		break;

	case 2:
		printf("\n===========================");
		printf("\n Your choice is TestImage.jpg\n");
		picture_filename = strdup("TestImage.jpg");
		break;
	default:
		break;
	}

	metadata_editor_append_picture(metadata, picture_filename);
	SAFE_FREE(picture_filename);
	return true;
}

static bool __delete_pictures(metadata_editor_h metadata)
{
	uint num, i;
	char *picture_index = NULL;

	metadata_editor_get_metadata(metadata, METADATA_EDITOR_ATTR_PICTURE_NUM, &picture_index);
	printf("The number of pictures is [%s]\n", picture_index);

	if (picture_index) {
		num = atoi(picture_index);
		printf("Number of pictures: %u\n", num);

		for (i = 0; i < num; ++i) {
			metadata_editor_remove_picture(metadata, 0);
			/*__printRetValue("metadata_editor_remove_picture(...)", ret); */
		}
	} else
		printf("There are no pictures to delete\n");
	return true;
}

static bool __save_tags(metadata_editor_h metadata)
{

	metadata_editor_update_metadata(metadata);

	return true;
}


int main(int argc, char *argv[])
{
	int ret = METADATA_EDITOR_ERROR_NONE;
	metadata_editor_h metadata = NULL;
	int cnt = argc - 1;
	uint c2 = 0;

	if (cnt < 1) {
		printf("\n====================================================================================================");
		printf("\n Please type file path and media type as media-metadata-test /opt/usr/media/Sounds/<filename.mp3>");
		printf("\n====================================================================================================");
		return 0;
	}
	printf("\n\n --- media metadata test start ---\n\n");

	ret = metadata_editor_create(&metadata);
	/*__printRetValue("metadata_editor_create(...)",ret); */

	if (ret != METADATA_EDITOR_ERROR_NONE) {
		printf("Fail metadata_editor_create() at line [%d]\n", __LINE__);
		return 0;
	}

	ret = metadata_editor_set_path(metadata, argv[1]);
	/*__printRetValue("metadata_editor_set_path(...)",ret); */

	if (ret != METADATA_EDITOR_ERROR_NONE) {
		printf("Fail metadata_editor_set_path() at line [%d]\n", __LINE__);
		goto exception;
	}

	while (c2 != 9) {
		c2 = 0;
		printf("\n========================================================================");
		printf("\n The file you are working with is %s", argv[1]);
		printf("\n======================================================================== ");
		printf("\n Choose desired operation for testing:\n");
		printf("1 - Read tags.       Press 1 \n");
		printf("2 - Write tags.      Press 2 \n");
		printf("3 - Delete tags.     Press 3 \n");
		printf("4 - Add picture.     Press 4 \n");
		printf("5 - Delete picture.  Press 5 \n");
		printf("6 - Save tags.       Press 6 \n");
		printf("9 - Quit.            Press 9 ");
		printf("\n=========================\n");
		printf("\n Your choice : ");
		dummy = scanf("%u", &c2);
		__flush();

		switch (c2) {
		case 1:
			printf("\n==============");
			printf("\n Reading tags \n");
			__get_tag_info(metadata);
			break;
		case 2:
			printf("\n==============");
			printf("\n Writing tags ");
			__write_tag_info(metadata);
			break;
		case 3:
			printf("\n==============");
			printf("\n Deleting tags ");
			__delete_tag_info(metadata);
			break;
		case 4:
			printf("\n==============");
			printf("\n Adding picture ");
			__add_picture(metadata);
			break;
		case 5:
			printf("\n==============");
			printf("\n Deleting pictures \n");
			__delete_pictures(metadata);
			break;
		case 6:
			printf("\n==============");
			printf("\n Saving updated tags \n");
			__save_tags(metadata);
			break;
		default:
			break;
		}
	}

exception:
	ret = metadata_editor_destroy(metadata);
	if (ret != METADATA_EDITOR_ERROR_NONE) {
		printf("Fail metadata_editor_destroy [%d]\n", ret);
		return 0;
	}

	printf("\n\n--- metadata writer test end ---\n\n");

	return ret;
}
