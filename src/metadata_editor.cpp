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

#include <metadata_editor.h>
#include <metadata_editor_private.h>
#include <aul.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int __ID3_getTwixFrameByName(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* frameID, char** value);
static int __ID3_setTwixFrameByName(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* frameID, const char* value);
static int __ID3_getFrameByName(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* frameID, char** value);
static int __ID3_setFrameByName(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* frameID, const char* value);
static int __ID3_getNumberOfPictures(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, char** value);
static int __ID3_getLyricsFrame(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, char** value);
static int __ID3_setTwixCommentFrame(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* value);
static int __ID3_setLyricsFrame(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* value);
static int __MP4_getStringItem(metadata_editor_s* _metadata, const char* itemname, char** value);
static int __MP4_getIntegerItem(metadata_editor_s* _metadata, const char* itemname, char** value);
static int __MP4_updateStringItem(metadata_editor_s* _metadata, const char* itemname, const char* value);
static int __MP4_updateIntegerItem(metadata_editor_s* _metadata, const char* itemname, const char* value);
static int __MP4_getNumberOfPictures(metadata_editor_s* _metadata, char** value);
#if 0
static int __xiph_getFieldValue(metadata_editor_s* _metadata, TagLib::Ogg::XiphComment* xtag, const char* fieldname, char** value);
static int __xiph_updateFieldValue(metadata_editor_s* _metadata, TagLib::Ogg::XiphComment* xtag, const char* fieldname, const char* value);
static int __FLAC_getNumberOfPictures(metadata_editor_s* _metadata, char** value);
#endif
typedef enum {
	METADATA_EDITOR_FORMAT_MP3 = 0,			/**< MP3 File */
	METADATA_EDITOR_FORMAT_MP4,				/**< MP4 File */
	METADATA_EDITOR_FORMAT_FLAC,				/**< FLAC File */
	METADATA_EDITOR_FORMAT_OGG_VORBIS,			/**< Vorbis Audio in Ogg container */
	METADATA_EDITOR_FORMAT_OGG_FLAC,			/**< FLAC Audio in Ogg container */
	METADATA_EDITOR_FORMAT_WAV,				/**< WAV file */
	METADATA_EDITOR_FORMAT_NOTYPE = 0xFF		/**< Error type. File type is not correct or not specified */
} metadata_editor_format_e;


// *** This is an auxiliary function that is used to get the frame value *** //
// *** It operates with frames that exists both in ID3v1 and ID3v2 tags *** //
static int __ID3_getTwixFrameByName(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* frameID, char** value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(frameID == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	// Check if the frame is empty (nothing to read) or ID3v2 tag does not exist
	if (!tag2 || tag2->frameListMap()[frameID].isEmpty()) {
		metadata_editor_info("The frame %s in ID3v2 tag is empty\n", frameID);
		// Check if the tag ID3v1 is also empty or does not exist
		if (!tag1 || tag1->isEmpty()) {
			metadata_editor_info("The frame %s in ID3v1 tag is empty as well\n", frameID);
			return METADATA_EDITOR_ERROR_NONE;
		} else {	// if not - read the frame you need there
			metadata_editor_info("Reading data from ID3v1 tag\n");

			TagLib::String str;
			uint length;
			bool found = false;

			if (!strcmp(frameID, "TPE1")) {			/* artist */
				str = tag1->artist();
				found = true;
			} else if (!strcmp(frameID, "TALB")) {	/* album */
				str = tag1->album();
				found = true;
			} else if (!strcmp(frameID, "COMM")) {	/* comment */
				str = tag1->comment();
				found = true;
			} else if (!strcmp(frameID, "TCON")) {	/* genre */
				str = tag1->genre();
				found = true;
			} else if (!strcmp(frameID, "TIT2")) {		/* title */
				str = tag1->title();
				found = true;
			}

			/* Check if we have already found the frame */
			if (found) {
				bool isUTF = false;
				if (!str.isLatin1()) isUTF = true;
				metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));
				length = strlen(str.toCString(isUTF));
				metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
				*value = strndup(str.toCString(isUTF), length);
				return METADATA_EDITOR_ERROR_NONE;
			}

			char buf[META_MAX_BUF_LEN] = {0, };

			if (!strcmp(frameID, "TRCK")) {			/* track */
				snprintf(buf, META_MAX_BUF_LEN, "%u", tag1->track());
				found = true;
			} else if (!strcmp(frameID, "TDRC")) {	/* data (year) */
				snprintf(buf, META_MAX_BUF_LEN, "%u", tag1->year());
				found = true;
			}

			if (found) {
				length = strlen(buf);
				metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
				*value = strndup(buf, length);
				return METADATA_EDITOR_ERROR_NONE;
			}

			/* The desired frame was not found */
			return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
	} else {		// or frame has data to read
		metadata_editor_info("The frame %s exists in ID3v2 tag\n", frameID);

		// This string is used to copy the value in the frame
		TagLib::String str = tag2->frameListMap()[frameID][0]->toString();
		bool isUTF = false;

		if (!str.isLatin1()) isUTF = true;
		metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));
		uint length = strlen(str.toCString(isUTF));
		metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");

		*value = strndup(str.toCString(isUTF), length);

		return METADATA_EDITOR_ERROR_NONE;
	}
}

// *** This is an auxiliary function that is used to write the new value to the frame *** //
// *** It operates with frames that exists both in ID3v1 and ID3v2 tags *** //
static int __ID3_setTwixFrameByName(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* frameID, const char* value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(frameID == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	// Check if the valid tag pointer exists
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");

	// If the pointer is NULL or c-string is empty - handle as request for deletion
	if (!value || (*value == '\0')) {
		metadata_editor_info("Request for frame %s deletion\n", frameID);
		tag2->removeFrames(frameID);
		if (tag1 && !tag1->isEmpty()) {
			if (!strcmp(frameID, "TPE1"))
				tag1->setArtist("");
			else if (!strcmp(frameID, "TALB"))
				tag1->setAlbum("");
			else if (!strcmp(frameID, "TCON"))
				tag1->setGenre("");
			else if (!strcmp(frameID, "TIT2"))
				tag1->setTitle("");
			else if (!strcmp(frameID, "TRCK"))
				tag1->setTrack(0);
			else if (!strcmp(frameID, "TDRC"))
				tag1->setYear(0);
		}

		return METADATA_EDITOR_ERROR_NONE;
	}

	// Check if the frame is empty (must create the frame before writing the data)
	if (tag2->frameListMap()[frameID].isEmpty()) {
		metadata_editor_info("The frame %s does not exist. Creating.\n", frameID);
		// This is a common frame type for textural frames except comment frame
		TagLib::ID3v2::TextIdentificationFrame* fr = new TagLib::ID3v2::TextIdentificationFrame(frameID);
		metadata_editor_retvm_if(fr == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

		fr->setTextEncoding(TagLib::String::UTF8);
		fr->setText(TagLib::String(value, TagLib::String::UTF8));
		tag2->addFrame(fr);
	} else {		// if not - just modify the data in the existing frame
		metadata_editor_info("The frame %s exists. Changing.\n", frameID);
		tag2->frameListMap()[frameID][0]->setText(TagLib::String(value, TagLib::String::UTF8));
	}

	if (tag1 && !tag1->isEmpty()) {				// Check if ID3v1 tag exists. Must copy data if yes.
		metadata_editor_info("ID3v1 tag also exists. Copying frame\n");
		if (!strcmp(frameID, "TPE1"))
				tag1->setArtist(value);
		else if (!strcmp(frameID, "TALB"))
				tag1->setAlbum(value);
		else if (!strcmp(frameID, "TCON"))		// Genre in ID3v1 is enumeration, so can not write it with "value"
				tag1->setGenre("");
		else if (!strcmp(frameID, "TIT2"))
				tag1->setTitle(value);
		else if (!strcmp(frameID, "TRCK"))
				tag1->setTrack(atoi(value));
		else if (!strcmp(frameID, "TDRC"))
				tag1->setYear(atoi(value));
	}

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function reads frames that exist only in ID3v2 tag *** //
static int __ID3_getFrameByName(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* frameID, char** value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(frameID == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	// Check if the frame is empty (nothing to read) or ID3v2 tag does not exist
	metadata_editor_retvm_if(!tag2 || tag2->frameListMap()[frameID].isEmpty(), METADATA_EDITOR_ERROR_NONE, "The frame %s does not exist\n", frameID);

	metadata_editor_info("The frame %s exists\n", frameID);
	// This string is used to copy the value in the frame
	TagLib::String str = tag2->frameListMap()[frameID][0]->toString();
	bool isUTF = false;
	if (!str.isLatin1()) isUTF = true;
	metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));

	uint length = strlen(str.toCString(isUTF));
	metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");

	*value = strndup(str.toCString(isUTF), length);

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function writes frames that exist only in ID3v2 tag *** //
static int __ID3_setFrameByName(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* frameID, const char* value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(frameID == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	// Check if the valid tag pointer exist
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");

	// If the pointer is NULL or c-string is empty - handle as request for deletion
	if (!value || (*value == '\0')) {
		metadata_editor_info("Request for frame %s deletion\n", frameID);
		tag2->removeFrames(frameID);
		return METADATA_EDITOR_ERROR_NONE;
	}

	// Check if the ID3v2 tag exists
	if (tag2->frameListMap()[frameID].isEmpty()) {
		metadata_editor_info("The frame %s does not exist. Creating.\n", frameID);
		// This is a common frame type for textural frames except comment frame
		TagLib::ID3v2::TextIdentificationFrame* fr = new TagLib::ID3v2::TextIdentificationFrame(frameID);
		metadata_editor_retvm_if(fr == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

		fr->setTextEncoding(TagLib::String::UTF8);
		fr->setText(TagLib::String(value, TagLib::String::UTF8));
		tag2->addFrame(fr);
	} else {		// if not - just modify the data in the existing frame
		metadata_editor_info("The frame %s exists. Changing.\n", frameID);
		tag2->frameListMap()[frameID][0]->setText(TagLib::String(value, TagLib::String::UTF8));
	}

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to receive the number of pictures stored in ID3v2 tag of file *** //
static int __ID3_getNumberOfPictures(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	// Check if the valid tag pointer exist
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag does not exist. Can not process further");

	TagLib::ID3v2::FrameList lst = tag2->frameListMap()["APIC"];		// link to picture frames in tag
	// Check if the frames exist
	metadata_editor_retvm_if(lst.isEmpty(), METADATA_EDITOR_ERROR_NONE, "No pictures in file\n");

	metadata_editor_info("APIC frames exist in file\n");
	char buf[META_MAX_BUF_LEN] = {0, };
	// Convert the number of frames (lst.size()) to c-string
	snprintf(buf, META_MAX_BUF_LEN, "%u", lst.size());
	*value = strndup(buf, strlen(buf));
	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to receive unsynchronized lyrics from ID3v2 tag in file *** //
// *** This frame differs from other string-type frames and uses UnsynchronizedLyricsFrame instead of TextIdentificationFrame *** //
static int __ID3_getLyricsFrame(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	// Check if the valid tag pointer exist
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag does not exist. Can not process further");

	TagLib::ID3v2::FrameList lst = tag2->frameListMap()["USLT"];		// link to unsynchronized lyric frames in tag
	// Check if frames exist in file
	metadata_editor_retvm_if(lst.isEmpty(), METADATA_EDITOR_ERROR_NONE, "The frame USLT does not exist\n");

	metadata_editor_info("The frame USLT exists\n");
	TagLib::ID3v2::FrameList::Iterator it = lst.begin();
	TagLib::ID3v2::UnsynchronizedLyricsFrame* frame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(*it);
	TagLib::String str = frame->text();
	bool isUTF = false;
	if (!str.isLatin1()) isUTF = true;
	metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));
	uint length = strlen(str.toCString(isUTF));
	metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
	*value = strndup(str.toCString(isUTF), length);
	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to set text in comment frame. It processes both ID3v1 and ID3v2 tags *** //
// *** Comment frame is different from other string-type frames. It uses CommentsFrame instead of TextIdentificationFrame *** //
static int __ID3_setTwixCommentFrame(metadata_editor_s* _metadata, TagLib::ID3v1::Tag* tag1, TagLib::ID3v2::Tag* tag2, const char* value)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	// Check if the valid tag pointer exist
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");

	// If the pointer is NULL or c-string is empty - handle as request for deletion
	if (!value || (*value == '\0')) {
		metadata_editor_info("Request for frame COMM deletion\n");
		tag2->removeFrames("COMM");
		if (tag1 && !tag1->isEmpty())
			tag1->setComment("");
		return METADATA_EDITOR_ERROR_NONE;
	}
	// If the comment frame is empty - create the frame and add it to the list
	if (tag2->frameListMap()["COMM"].isEmpty()) {
		metadata_editor_info("The frame COMM does not exist. Creating.\n");
		TagLib::ID3v2::CommentsFrame* fr = new TagLib::ID3v2::CommentsFrame;
		metadata_editor_retvm_if(fr == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY\n");
		fr->setText(TagLib::String(value, TagLib::String::UTF8));
		fr->setTextEncoding(TagLib::String::UTF8);
		tag2->addFrame(fr);
	} else {						// If the frame already exists - just modify its value
		metadata_editor_info("The frame COMM exists. Changing.\n");
		tag2->frameListMap()["COMM"][0]->setText(TagLib::String(value, TagLib::String::UTF8));
	}

	if (tag1 && !tag1->isEmpty()) {			// Copy the value to ID3v1 tag comment
		metadata_editor_info("ID3v1 tag also exists. Copying frame\n");
		tag1->setComment(value);
	}

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to set text in Lyrics frame *** //
// *** Lyrics frame is different from other string-type frames and uses UnsynchronizedLyricsFrame instead of TextIdentificationFrame *** //
static int __ID3_setLyricsFrame(metadata_editor_s* _metadata, TagLib::ID3v2::Tag* tag2, const char* value)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	// Check if the valid tag pointer exist
	metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");

	TagLib::ID3v2::FrameList lst = tag2->frameListMap()["USLT"];	// link to unsynchronized lyric frames in tag
	// If the pointer is NULL or c-string is empty - handle as request for deletion
	if (!value || (*value == '\0')) {
		metadata_editor_info("Request for frame USLT deletion\n");
		tag2->removeFrames("USLT");
		return METADATA_EDITOR_ERROR_NONE;
	}
	// Check if lyrics frames exist
	if (lst.isEmpty()) {
		// No lyrics - create the frame and add it to the ID3v2 tag
		metadata_editor_info("The frame USLT does not exist. Creating.\n");
		TagLib::ID3v2::UnsynchronizedLyricsFrame* frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame;
		metadata_editor_retvm_if(frame == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

		frame->setTextEncoding(TagLib::String::UTF8);
		frame->setText(TagLib::String(value, TagLib::String::UTF8));
		tag2->addFrame(frame);
	} else {									// the lyrics frames exist - change the existing one
		metadata_editor_info("USLT frames exist in file. Changing.\n");
		TagLib::ID3v2::FrameList::Iterator it = lst.begin();
		TagLib::ID3v2::UnsynchronizedLyricsFrame* frame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(*it);
		frame->setTextEncoding(TagLib::String::UTF8);
		frame->setText(TagLib::String(value, TagLib::String::UTF8));
	}

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function extracts string values from tag in MP4 file *** //
static int __MP4_getStringItem(metadata_editor_s* _metadata, const char* itemname, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(itemname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
	TagLib::MP4::Tag* tag = _file->tag();
	metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist");

	// Get map of items directly from tag and launch a search of specific item
	TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
	TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find(itemname);
	if (it != itemMap.end()) {								// Item was found
		TagLib::String str = it->second.toStringList()[0];			// Get the first string in item
		// Check the encoding of the string (1252 or not)
		bool isUTF = false;
		if (!str.isLatin1()) isUTF = true;
		metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));
		// Get the length of the string and check if it is empty or not
		uint length = strlen(str.toCString(isUTF));
		metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
		*value = strndup(str.toCString(isUTF), length);
		return METADATA_EDITOR_ERROR_NONE;
	} else {										// Item was not found
		metadata_editor_info("No item <%s> in file\n", itemname);
		return METADATA_EDITOR_ERROR_NONE;
	}
}

// *** This function extracts integer value from item in MP4 tag *** //
static int __MP4_getIntegerItem(metadata_editor_s* _metadata, const char* itemname, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(itemname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
	TagLib::MP4::Tag* tag = _file->tag();
	metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist");

	// Get map of items directly from tag and launch a search of specific item
	TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
	TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find(itemname);
	if (it != itemMap.end()) {								// Item was found
		char buf[META_MAX_BUF_LEN] = {0, };
		int num = it->second.toInt();						// Get integer value in item
		snprintf(buf, META_MAX_BUF_LEN, "%u", num);						// Convert int into char[]
		// Determine the length of created c-string and copy it into the output variable
		int length = strlen(buf);
		metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
		*value = strndup(buf, length);
		return METADATA_EDITOR_ERROR_NONE;
	} else {										// Item was not found
		metadata_editor_info("No item <%s> in file\n", itemname);
		return METADATA_EDITOR_ERROR_NONE;
	}
}

// *** This function adds (or changes) string item of itemname type *** //
static int __MP4_updateStringItem(metadata_editor_s* _metadata, const char* itemname, const char* value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(itemname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
	TagLib::MP4::Tag* tag = _file->tag();
	metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag was not created");

	// Get map of items directly from tag and launch a search of specific item
	TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
	// Check if it is a request for deletion
	if ((value == NULL) || value[0] == '\0') {
		metadata_editor_info("Request for deleting of item <%s>\n", itemname);
		TagLib::MP4::ItemListMap::Iterator it = itemMap.find(itemname);
		if (it != itemMap.end())
			itemMap.erase(it);
		return METADATA_EDITOR_ERROR_NONE;
	}
	metadata_editor_info("The item <%s> will be added\n", itemname);
	itemMap[itemname] = TagLib::MP4::Item(TagLib::String(value, TagLib::String::UTF8));

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function adds (or changes) integer item of itemname type *** //
static int __MP4_updateIntegerItem(metadata_editor_s* _metadata, const char* itemname, const char* value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(itemname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
	TagLib::MP4::Tag* tag = _file->tag();
	metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag was not created");

	// Get map of items directly from tag and launch a search of specific item
	TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
	// Check if it is a request for deletion
	if ((value == NULL) || value[0] == '\0') {
		metadata_editor_info("Request for deleting of item <%s>\n", itemname);
		TagLib::MP4::ItemListMap::Iterator it = itemMap.find(itemname);
		if (it != itemMap.end())
			itemMap.erase(it);
		return METADATA_EDITOR_ERROR_NONE;
	}
	// Check if the value is integer string then it can be successfully converted into integer
	if (isdigit(value[0])) {
		metadata_editor_info("The item <%s> will be added\n", itemname);
		int number = atoi(value);
		itemMap[itemname] = TagLib::MP4::Item(number);
		return METADATA_EDITOR_ERROR_NONE;
	} else {										// Notify that string is not a number to process
		metadata_editor_error("Error. String does not contain a number\n");
		return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function is used to find the number of pictures stored in MP4 file *** //
static int __MP4_getNumberOfPictures(metadata_editor_s* _metadata, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
	TagLib::MP4::Tag* tag = _file->tag();
	metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist\n");

	// Get map of items directly from tag and launch a search of specific item
	TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
	TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find("covr");
	if (it != itemMap.end()) {								// Item was found
		uint number = it->second.toCoverArtList().size();			// Get the size of CoverList (i.e. number of pictures in file)
		metadata_editor_info("There are %u picture(s) in file\n", number);
		char buf[META_MAX_BUF_LEN] = {0, };
		snprintf(buf, META_MAX_BUF_LEN, "%u", number);					// Convert integer value of size to its c-string representation
		int length = strlen(buf);
		metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
		*value = strndup(buf, length);
		return METADATA_EDITOR_ERROR_NONE;
	} else {										// Item was not found
		metadata_editor_info("No item <covr> in file\n");
		return METADATA_EDITOR_ERROR_NONE;
	}
}
#if 0
// *** This function is used to extract string from Xiph Comment field *** //
static int __xiph_getFieldValue(metadata_editor_s* _metadata, TagLib::Ogg::XiphComment* xtag, const char* fieldname, char** value)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(fieldname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Value Pointer");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(!xtag, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist\n");

	const TagLib::Ogg::FieldListMap& fieldMap = xtag->fieldListMap();
	TagLib::Ogg::FieldListMap::ConstIterator it = fieldMap.find(fieldname);

	if ((xtag->contains(fieldname)) && (it != fieldMap.end())) {			// Field was found
		metadata_editor_info("Field %s was found. Extracting\n", fieldname);
		TagLib::String str = it->second[0];					// Get the first string in xiph field
		// Check the encoding of the string (1252 or not)
		bool isUTF = false;
		if (!str.isLatin1()) isUTF = true;
		metadata_editor_info("String is %sUTF\n", (isUTF ? "" : "not "));
		// Get the length of the string and check if it is empty or not
		uint length = strlen(str.toCString(isUTF));
		metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
		*value = strndup(str.toCString(isUTF), length);
		return METADATA_EDITOR_ERROR_NONE;
	} else {										// Field was not found
		metadata_editor_info("No field %s in Xiph Comment\n", fieldname);
		return METADATA_EDITOR_ERROR_NONE;
	}
}

// *** This function is used to write string into Xiph Comment fields *** //
static int __xiph_updateFieldValue(metadata_editor_s* _metadata, TagLib::Ogg::XiphComment* xtag, const char* fieldname, const char* value)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(fieldname == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");
	metadata_editor_retvm_if(!xtag, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist\n");

	// Check if it is a request for deletion
	if ((value == NULL) || value[0] == '\0') {
		metadata_editor_info("Request for deleting of field %s\n", fieldname);
		xtag->removeField(fieldname);
		return METADATA_EDITOR_ERROR_NONE;
	}
	metadata_editor_info("The field %s will be added\n", fieldname);
	// "true" is used to remove other strings of the same "fieldname" first
	xtag->addField(fieldname, TagLib::String(value, TagLib::String::UTF8), true);
	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to receive the number of pictures in FLAC file *** //
static int __FLAC_getNumberOfPictures(metadata_editor_s* _metadata, char** value)
{
	// Check if parameters are valid
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Value Pointer");
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	TagLib::FLAC::File* _file = (TagLib::FLAC::File*) _metadata->file;
	if (_file->pictureList().isEmpty()) {
		metadata_editor_info("No pictures in FLAC file\n");
		return METADATA_EDITOR_ERROR_NONE;
	}
	uint number = _file->pictureList().size();
	metadata_editor_info("There are %u picture(s) in file\n", number);
	char buf[META_MAX_BUF_LEN] = {0, };
	snprintf(buf, META_MAX_BUF_LEN, "%u", number);						// Convert integer value of size to its c-string representation
	uint length = strlen(buf);
	metadata_editor_retvm_if(length == 0, METADATA_EDITOR_ERROR_NONE, "Empty string...\n");
	*value = strndup(buf, length);
	return METADATA_EDITOR_ERROR_NONE;
}
#endif
int __metadata_editor_get_file_ext(const char *file_path, char *file_ext, int max_len)
{
	int i = 0;
	unsigned int path_len = strlen(file_path);

	for (i = (int)path_len; i >= 0; i--) {
		if ((file_path[i] == '.') && (i < (int)path_len)) {
			strncpy(file_ext, &file_path[i + 1], max_len);
			return 0;
		}

		/* meet the dir. no ext */
		if (file_path[i] == '/')
			return -1;
	}

	return -1;
}

int __metadata_editor_get_file_type(const char *path)
{
	int ret = 0;
	char mimetype[255] = {0, };

	/* get content type and mime type from file. */
	ret = aul_get_mime_from_file(path, mimetype, sizeof(mimetype));
	if (ret < 0) {
		metadata_editor_debug("aul_get_mime_from_file fail.. Now trying to get type by extension");

		char ext[255] = { 0 };
		int ret = __metadata_editor_get_file_ext(path, ext, sizeof(ext));
		metadata_editor_retvm_if(ret < 0, METADATA_EDITOR_FORMAT_NOTYPE, "__metadata_editor_get_file_type failed");

		if (strcasecmp(ext, "MP3") == 0)
			return METADATA_EDITOR_FORMAT_MP3;
		else if (strcasecmp(ext, "MP4") == 0)
			return METADATA_EDITOR_FORMAT_MP4;
		else
			return METADATA_EDITOR_FORMAT_NOTYPE;
	}

	metadata_editor_debug("mime type : %s", mimetype);

	/* categorize from mimetype */
	if (strstr(mimetype, "mpeg") != NULL)
		return METADATA_EDITOR_FORMAT_MP3;
	else if (strstr(mimetype, "mp4") != NULL)
		return METADATA_EDITOR_FORMAT_MP4;

	return METADATA_EDITOR_FORMAT_NOTYPE;
}

int __metadata_editor_get_picture_type(const char *path, char **type)
{
	int ret = 0;
	char mimetype[255] = {0, };
	const char *type_jpeg = "image/jpeg";
	const char *type_png = "image/png";

	/* get content type and mime type from file. */
	ret = aul_get_mime_from_file(path, mimetype, sizeof(mimetype));
	if (ret < 0) {
		metadata_editor_debug("aul_get_mime_from_file fail.. Now trying to get type by extension");

		char ext[255] = { 0 };
		int ret = __metadata_editor_get_file_ext(path, ext, sizeof(ext));
		metadata_editor_retvm_if(ret < 0, METADATA_EDITOR_ERROR_OPERATION_FAILED, "__metadata_editor_get_file_type failed");

		if (strcasecmp(ext, "JPG") == 0 || strcasecmp(ext, "JPEG") == 0) {
			*type = strndup(type_jpeg, strlen(type_jpeg));
			return METADATA_EDITOR_ERROR_NONE;
		} else if (strcasecmp(ext, "PNG") == 0) {
			*type = strndup(type_png, strlen(type_png));
			return METADATA_EDITOR_ERROR_NONE;
		} else {
			return METADATA_EDITOR_ERROR_NOT_SUPPORTED;
		}
	}

	metadata_editor_debug("mime type : %s", mimetype);

	/* categorize from mimetype */
	if (strstr(mimetype, "jpeg") != NULL) {
		*type = strndup(mimetype, strlen(mimetype));
		return METADATA_EDITOR_ERROR_NONE;
	} else if (strstr(mimetype, "png") != NULL) {
		*type = strndup(mimetype, strlen(mimetype));
		return METADATA_EDITOR_ERROR_NONE;
	}

	return METADATA_EDITOR_ERROR_NOT_SUPPORTED;
}

int __metadata_editor_get_picture_info(const char *path, void **picture, int *size, char **type)
{
	int ret;

	ret = __metadata_editor_get_picture_type(path, type);
	if (ret != METADATA_EDITOR_ERROR_NONE)
		return METADATA_EDITOR_ERROR_OPERATION_FAILED;

	//IF ok.. read file
	FILE* fin = fopen(path, "rb");
	int file_size = 0;

	if (fin) {
		while (fgetc(fin) != EOF)
			file_size++;

		fclose(fin);
		char picture_buffer[file_size] = {0, };
		memset(picture_buffer, 0, file_size * sizeof(char));
		fin = fopen(path, "rb");
		if (fin) {
			ret = fread(picture_buffer, file_size, 1, fin);
			fclose(fin);
		}
		if (*picture == NULL) {
			*picture = malloc(file_size * sizeof(char));
			memset(*picture, 0, file_size * sizeof(char));
			memcpy(*picture, picture_buffer, file_size);
			*size = file_size;
		}
	}

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to allocate the metadata_editor_s in memory		*** //
// *** The structure metadata_editor_s contains all information about the file	*** //
extern "C" int metadata_editor_create(metadata_editor_h *metadata)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	metadata_editor_s *_metadata = new metadata_editor_s;		// Allocate a structure for handler
	metadata_editor_retvm_if(_metadata == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

	_metadata->file = NULL;
	_metadata->filetype = METADATA_EDITOR_FORMAT_NOTYPE;			// Specify file type out of range
	_metadata->isOpen = false;						// File is not opened yet
	_metadata->isReadOnly = true;						// Handle unexisting file as readonly

	// Save the structure in the metadata
	*metadata = (metadata_editor_h)_metadata;

	return METADATA_EDITOR_ERROR_NONE;
}

// *** This function is used to open the file. It creates the instance that is responsible for connection with file *** //
extern "C" int metadata_editor_set_path(metadata_editor_h metadata, const char *path)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle");
	metadata_editor_retvm_if(path == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle");

	int exist;

	/* check the file exits actually */
	exist = open(path, O_RDONLY);
	if(exist < 0) {
		metadata_editor_error("Not exist file\n");
		if (errno == EACCES || errno == EPERM) {
			return METADATA_EDITOR_ERROR_PERMISSION_DENIED;
		} else {
			return METADATA_EDITOR_ERROR_FILE_EXISTS;
		}
	}

	close(exist);

	metadata_editor_s *_metadata = (metadata_editor_s*)metadata;
	int media_type = METADATA_EDITOR_FORMAT_NOTYPE;

	media_type = __metadata_editor_get_file_type(path);

	switch (media_type) {							// Parse file according the specified type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			if (_metadata->file) {
				TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
				metadata_editor_info("file free [%lX]", _metadata->file);
				delete _file;
				_metadata->file = NULL;
				_metadata->filetype = METADATA_EDITOR_FORMAT_NOTYPE;
				_metadata->isOpen = false;
				_metadata->isReadOnly = true;
			}

			// Allocate the file object in memory to work with it later on
			TagLib::MPEG::File* _file = new TagLib::MPEG::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_MP3;

			if (_file->isOpen()) {					// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_PERMISSION_DENIED;
			}

			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}

			return METADATA_EDITOR_ERROR_NONE;
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			if (_metadata->file) {
				TagLib::MP4::File* _file = (TagLib::MP4::File*)_metadata->file;
				metadata_editor_info("file free [%lX]", _metadata->file);
				delete _file;
				_metadata->file = NULL;
				_metadata->filetype = METADATA_EDITOR_FORMAT_NOTYPE;
				_metadata->isOpen = false;
				_metadata->isReadOnly = true;
			}

			// Allocate the file object in memory to work with it later on
			TagLib::MP4::File* _file = new TagLib::MP4::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_MP4;

			if (_file->isOpen()) {					// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_FILE_EXISTS;
			}
			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}
			return METADATA_EDITOR_ERROR_NONE;
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			// Allocate the file object in memory to work with it later on
			TagLib::FLAC::File* _file = new TagLib::FLAC::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_FLAC;

			if (_file->isOpen()) {				// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_FILE_EXISTS;
			}
			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}
			return METADATA_EDITOR_ERROR_NONE;
		}
		case METADATA_EDITOR_FORMAT_OGG_VORBIS:
		{
			// Allocate the file object in memory to work with it later on
			TagLib::Ogg::Vorbis::File* _file = new TagLib::Ogg::Vorbis::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_OGG_VORBIS;

			if (_file->isOpen()) {				// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_FILE_EXISTS;
			}
			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}
			return METADATA_EDITOR_ERROR_NONE;
		}
		case METADATA_EDITOR_FORMAT_OGG_FLAC:
		{
			// Allocate the file object in memory to work with it later on
			TagLib::Ogg::FLAC::File* _file = new TagLib::Ogg::FLAC::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_OGG_FLAC;

			if (_file->isOpen()) {					// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_FILE_EXISTS;
			}
			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}
			return METADATA_EDITOR_ERROR_NONE;
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			// Allocate the file object in memory to work with it later on
			TagLib::RIFF::WAV::File* _file = new TagLib::RIFF::WAV::File(path);

			metadata_editor_retvm_if(_file == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

			_metadata->file = _file;				// Copy file pointer to the structure

			_metadata->filetype = METADATA_EDITOR_FORMAT_WAV;

			if (_file->isOpen()) {					// Check if the file was opened successfully
				metadata_editor_info("The file is successfully opened. Address is %lX\n", _metadata->file);
				_metadata->isOpen = true;
			} else {							// The file does not exist or you have no permission to process it
				metadata_editor_error("The file was not found. Pointer address is %lX\n", _metadata->file);
				_metadata->isOpen = false;
				return METADATA_EDITOR_ERROR_FILE_EXISTS;
			}
			if (_file->readOnly()) {					// Check if the file is readonly
				metadata_editor_info("File is readonly\n");
				_metadata->isReadOnly = true;
			} else {							// or not
				metadata_editor_info("The file is writable\n");
				_metadata->isReadOnly = false;
			}
			return METADATA_EDITOR_ERROR_NONE;
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_NOT_SUPPORTED;
	}
}

// *** This function is used to get the tag frame (field, item - each tag has its own name for data unit) from file *** //
extern "C" int metadata_editor_get_metadata(metadata_editor_h metadata, metadata_editor_attr_e attribute, char **value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle");
	metadata_editor_retvm_if(value == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Value Pointer");

	metadata_editor_s *_metadata = (metadata_editor_s*)metadata;
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	*value = NULL;
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	switch (_metadata->filetype) {				// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type and make tag pointers
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
			TagLib::ID3v1::Tag* tag1 = _file->ID3v1Tag();
			TagLib::ID3v2::Tag* tag2 = _file->ID3v2Tag();

			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TPE1", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TIT2", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TALB", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TCON", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __ID3_getFrameByName(_metadata, tag2, "TCOM", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __ID3_getFrameByName(_metadata, tag2, "TCOP", value);
				case METADATA_EDITOR_ATTR_DATE:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TDRC", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __ID3_getFrameByName(_metadata, tag2, "TIT3", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "COMM", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __ID3_getTwixFrameByName(_metadata, tag1, tag2, "TRCK", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __ID3_getFrameByName(_metadata, tag2, "TPE3", value);
				case METADATA_EDITOR_ATTR_PICTURE_NUM:			return __ID3_getNumberOfPictures(_metadata, tag2, value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __ID3_getLyricsFrame(_metadata, tag2, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __MP4_getStringItem(_metadata, "\xA9""ART", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __MP4_getStringItem(_metadata, "\xA9""nam", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __MP4_getStringItem(_metadata, "\xA9""alb", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __MP4_getStringItem(_metadata, "\xA9""gen", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __MP4_getStringItem(_metadata, "\xA9""wrt", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __MP4_getStringItem(_metadata, "cprt", value);
				case METADATA_EDITOR_ATTR_DATE:			return __MP4_getStringItem(_metadata, "\xA9""day", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __MP4_getStringItem(_metadata, "desc", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __MP4_getStringItem(_metadata, "\xA9""cmt", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __MP4_getIntegerItem(_metadata, "trkn", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __MP4_getStringItem(_metadata, "cond", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __MP4_getStringItem(_metadata, "\xA9""lyr", value);
				case METADATA_EDITOR_ATTR_PICTURE_NUM:			return __MP4_getNumberOfPictures(_metadata, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->xiphComment(false);
			if (!xtag) {									// Check if we have a valid tag for processing
				metadata_editor_error("Tag does not exist\n");
				*value = NULL;
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_getFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_getFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_getFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_getFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_getFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_getFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_getFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_getFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_getFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_getFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_getFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_getFieldValue(_metadata, xtag, "LYRICS", value);
				case METADATA_EDITOR_ATTR_PICTURE_NUM:			return __FLAC_getNumberOfPictures(_metadata, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_OGG_VORBIS:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::Ogg::Vorbis::File* _file = (TagLib::Ogg::Vorbis::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->tag();
			if (!xtag) {									// Check if we have a valid tag for processing
				metadata_editor_error("Tag does not exist\n");
				*value = NULL;
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_getFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_getFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_getFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_getFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_getFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_getFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_getFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_getFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_getFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_getFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_getFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_getFieldValue(_metadata, xtag, "LYRICS", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_OGG_FLAC:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::Ogg::FLAC::File* _file = (TagLib::Ogg::FLAC::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->tag();
			if (!xtag) {									// Check if we have a valid tag for processing
				metadata_editor_error("Tag does not exist\n");
				*value = NULL;
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_getFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_getFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_getFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_getFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_getFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_getFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_getFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_getFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_getFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_getFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_getFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_getFieldValue(_metadata, xtag, "LYRICS", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			// Bring the pointer to actual file type and make tag pointers
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->tag();

			if (tag2 == NULL) {								// Check if we have a valid tag for processing
				metadata_editor_error("Error. ID3v2 tag does not exist. Can not proceed metadata extraction\n");
				*value = NULL;
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}

			switch (attribute) {					// Check which one of frame types was given to the function for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __ID3_getFrameByName(_metadata, tag2, "TPE1", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __ID3_getFrameByName(_metadata, tag2, "TIT2", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __ID3_getFrameByName(_metadata, tag2, "TALB", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __ID3_getFrameByName(_metadata, tag2, "TCON", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __ID3_getFrameByName(_metadata, tag2, "TCOM", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __ID3_getFrameByName(_metadata, tag2, "TCOP", value);
				case METADATA_EDITOR_ATTR_DATE:			return __ID3_getFrameByName(_metadata, tag2, "TDRC", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __ID3_getFrameByName(_metadata, tag2, "TIT3", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __ID3_getFrameByName(_metadata, tag2, "COMM", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __ID3_getFrameByName(_metadata, tag2, "TRCK", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __ID3_getFrameByName(_metadata, tag2, "TPE3", value);
				case METADATA_EDITOR_ATTR_PICTURE_NUM:			return __ID3_getNumberOfPictures(_metadata, tag2, value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __ID3_getLyricsFrame(_metadata, tag2, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function is used to modify the metadata (frame in tag). But it does not apply changes to file *** //
extern "C" int metadata_editor_set_metadata(metadata_editor_h metadata, metadata_editor_attr_e attribute, const char* value)
{
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle");

	metadata_editor_s* _metadata = (metadata_editor_s*) metadata;

	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	switch (_metadata->filetype) {						// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
			TagLib::ID3v1::Tag* tag1 = _file->ID3v1Tag();
			TagLib::ID3v2::Tag* tag2 = _file->ID3v2Tag(true);

			metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");

			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TPE1", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TIT2", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TALB", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TCON", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __ID3_setFrameByName(_metadata, tag2, "TCOM", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:		return __ID3_setFrameByName(_metadata, tag2, "TCOP", value);
				case METADATA_EDITOR_ATTR_DATE:			return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TDRC", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __ID3_setFrameByName(_metadata, tag2, "TIT3", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:		return __ID3_setTwixFrameByName(_metadata, tag1, tag2, "TRCK", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:		return __ID3_setFrameByName(_metadata, tag2, "TPE3", value);
				case METADATA_EDITOR_ATTR_COMMENT:		return __ID3_setTwixCommentFrame(_metadata, tag1, tag2, value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:	return __ID3_setLyricsFrame(_metadata, tag2, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __MP4_updateStringItem(_metadata, "\xA9""ART", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __MP4_updateStringItem(_metadata, "\xA9""nam", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __MP4_updateStringItem(_metadata, "\xA9""alb", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __MP4_updateStringItem(_metadata, "\xA9""gen", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __MP4_updateStringItem(_metadata, "\xA9""wrt", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:		return __MP4_updateStringItem(_metadata, "cprt", value);
				case METADATA_EDITOR_ATTR_DATE:				return __MP4_updateStringItem(_metadata, "\xA9""day", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __MP4_updateStringItem(_metadata, "desc", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __MP4_updateStringItem(_metadata, "\xA9""cmt", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:		return __MP4_updateIntegerItem(_metadata, "trkn", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:		return __MP4_updateStringItem(_metadata, "cond", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:		return __MP4_updateStringItem(_metadata, "\xA9""lyr", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->xiphComment(true);
			if (!xtag) {								// Check if we have a valid tag for processing
				metadata_editor_error("Error. Xiph Comment was not created. Can not proceed metadata updating\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_updateFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_updateFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_updateFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_updateFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_updateFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_updateFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_updateFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_updateFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_updateFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_updateFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_updateFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_updateFieldValue(_metadata, xtag, "LYRICS", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_OGG_VORBIS:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::Ogg::Vorbis::File* _file = (TagLib::Ogg::Vorbis::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->tag();
			if (!xtag) {								// Check if we have a valid tag for processing
				metadata_editor_error("Error. Xiph Comment was not created. Can not proceed metadata updating\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_updateFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_updateFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_updateFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_updateFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_updateFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_updateFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_updateFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_updateFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_updateFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_updateFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_updateFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_updateFieldValue(_metadata, xtag, "LYRICS", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_OGG_FLAC:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::Ogg::FLAC::File* _file = (TagLib::Ogg::FLAC::File*)_metadata->file;
			TagLib::Ogg::XiphComment* xtag = _file->tag();
			if (!xtag) {							// Check if we have a valid tag for processing
				metadata_editor_error("Error. Xiph Comment was not created. Can not proceed metadata updating\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __xiph_updateFieldValue(_metadata, xtag, "ARTIST", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __xiph_updateFieldValue(_metadata, xtag, "TITLE", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __xiph_updateFieldValue(_metadata, xtag, "ALBUM", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __xiph_updateFieldValue(_metadata, xtag, "GENRE", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __xiph_updateFieldValue(_metadata, xtag, "COMPOSER", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:			return __xiph_updateFieldValue(_metadata, xtag, "COPYRIGHT", value);
				case METADATA_EDITOR_ATTR_DATE:			return __xiph_updateFieldValue(_metadata, xtag, "DATE", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __xiph_updateFieldValue(_metadata, xtag, "DESCRIPTION", value);
				case METADATA_EDITOR_ATTR_COMMENT:			return __xiph_updateFieldValue(_metadata, xtag, "COMMENT", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:			return __xiph_updateFieldValue(_metadata, xtag, "TRACKNUMBER", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:			return __xiph_updateFieldValue(_metadata, xtag, "CONDUCTOR", value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:			return __xiph_updateFieldValue(_metadata, xtag, "LYRICS", value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->tag();
			// Check if the valid tag pointer exist
			if (tag2 == NULL) {
				metadata_editor_error("Error. ID3v2 tag was not created. Can not proceed metadata updating\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}

			switch (attribute) {					// Check which one of frame type was given for processing
				case METADATA_EDITOR_ATTR_ARTIST:			return __ID3_setFrameByName(_metadata, tag2, "TPE1", value);
				case METADATA_EDITOR_ATTR_TITLE:			return __ID3_setFrameByName(_metadata, tag2, "TIT2", value);
				case METADATA_EDITOR_ATTR_ALBUM:			return __ID3_setFrameByName(_metadata, tag2, "TALB", value);
				case METADATA_EDITOR_ATTR_GENRE:			return __ID3_setFrameByName(_metadata, tag2, "TCON", value);
				case METADATA_EDITOR_ATTR_AUTHOR:			return __ID3_setFrameByName(_metadata, tag2, "TCOM", value);
				case METADATA_EDITOR_ATTR_COPYRIGHT:		return __ID3_setFrameByName(_metadata, tag2, "TCOP", value);
				case METADATA_EDITOR_ATTR_DATE:			return __ID3_setFrameByName(_metadata, tag2, "TDRC", value);
				case METADATA_EDITOR_ATTR_DESCRIPTION:		return __ID3_setFrameByName(_metadata, tag2, "TIT3", value);
				case METADATA_EDITOR_ATTR_TRACK_NUM:		return __ID3_setFrameByName(_metadata, tag2, "TRCK", value);
				case METADATA_EDITOR_ATTR_CONDUCTOR:		return __ID3_setFrameByName(_metadata, tag2, "TPE3", value);
				case METADATA_EDITOR_ATTR_COMMENT:		return __ID3_setTwixCommentFrame(_metadata, NULL, tag2, value);
				case METADATA_EDITOR_ATTR_UNSYNCLYRICS:	return __ID3_setLyricsFrame(_metadata, tag2, value);
				default:
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			}
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function apply all changes done in the tag(s) and update them to file *** //
extern "C" int metadata_editor_update_metadata(metadata_editor_h metadata)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle\n");

	metadata_editor_s *_metadata = (metadata_editor_s*)metadata;
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	switch (_metadata->filetype) {						// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;

			TagLib::ID3v1::Tag* tag1 = _file->ID3v1Tag();

			if (!tag1 || tag1->isEmpty()) {							// If no ID3v1 tag - prevent its creation
				if (_file->save(TagLib::MPEG::File::ID3v2 | TagLib::MPEG::File::APE))
					return METADATA_EDITOR_ERROR_NONE;
				else
					return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {										// otherwise - save all tags in file
				if (_file->save(TagLib::MPEG::File::AllTags))
					return METADATA_EDITOR_ERROR_NONE;
				else
					return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			TagLib::MP4::File* _file = (TagLib::MP4::File*)_metadata->file;
			if (_file->save())
				return METADATA_EDITOR_ERROR_NONE;
			else
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*)_metadata->file;
			if (_file->save())
				return METADATA_EDITOR_ERROR_NONE;
			else
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
		case METADATA_EDITOR_FORMAT_OGG_VORBIS:
		{
			TagLib::Ogg::Vorbis::File* _file = (TagLib::Ogg::Vorbis::File*)_metadata->file;
			if (_file->save())
				return METADATA_EDITOR_ERROR_NONE;
			else
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
		case METADATA_EDITOR_FORMAT_OGG_FLAC:
		{
			TagLib::Ogg::FLAC::File* _file = (TagLib::Ogg::FLAC::File*)_metadata->file;
			if (_file->save())
				return METADATA_EDITOR_ERROR_NONE;
			else
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			if (_file->save())
				return METADATA_EDITOR_ERROR_NONE;
			else
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function returns buffer with picture under the specified index and buffer's (picture's) size *** //
extern "C" int metadata_editor_get_picture(metadata_editor_h metadata, int index, void **picture, int *size, char **mime_type)
{
	const char *TYPE_JPEG = "image/jpeg";
	const char *TYPE_PNG = "image/png";
	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID Handle\n");
	metadata_editor_retvm_if(picture == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(size == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	metadata_editor_s* _metadata = (metadata_editor_s*) metadata;
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	*picture = NULL;
	*size = 0;
	*mime_type = NULL;

	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	switch (_metadata->filetype) {									// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;		// Bring the pointer to actual file type
			TagLib::ID3v2::Tag* tag2 = _file->ID3v2Tag();
			metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. No ID3v2 tag in file.");

			TagLib::ID3v2::FrameList lst = tag2->frameListMap()["APIC"];
			// Check if there are pictures in the tag
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {		// pictures exist in file
				// Check if index is correct or not
				if ((index < 0) || (lst.size() <= (uint)index)) {
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {		// everything is correct - begin extraction
					metadata_editor_info("There are %u pictures in file. Start of picture number %d extraction", lst.size(), index);
					int i = 0;
					// Among all frames we must choose that one with specified index. "i" will be counter
					for (TagLib::ID3v2::FrameList::Iterator it = lst.begin(); it != lst.end(); ++it, ++i) {
						if (i != index) continue;
						TagLib::ID3v2::AttachedPictureFrame* pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
						uint pictureSize = pictureFrame->picture().size();
						metadata_editor_retvm_if(pictureSize == 0, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Size of picture is 0");
						META_MALLOC(*picture, pictureSize);
						metadata_editor_retvm_if(*picture == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

						memcpy(*picture, pictureFrame->picture().data(), pictureSize);
						*size = pictureSize;
						TagLib::String mime = pictureFrame->mimeType();
						if (!strcmp(mime.toCString(), "image/jpeg"))
							*mime_type = strndup(TYPE_JPEG, strlen(TYPE_JPEG));
						else if (!strcmp(mime.toCString(), "image/png"))
							*mime_type = strndup(TYPE_PNG, strlen(TYPE_PNG));
						else
							*mime_type = NULL;
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			}
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
			TagLib::MP4::Tag* tag = _file->tag();
			metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Tag does not exist\n");

			// Get map of items directly from tag and launch a search of specific item
			TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
			TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find("covr");
			if (it != itemMap.end()) {								// Item was found
				TagLib::MP4::CoverArtList lst = it->second.toCoverArtList();
				// Check if the index is in range of CoverArtList Item
				if ((index < 0) || ((uint)index >= lst.size())) {				// it is not
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {									// index is in range
					int i = 0;
					for (TagLib::MP4::CoverArtList::ConstIterator picIt = lst.begin(); picIt != lst.end(); ++picIt, ++i) {
						if (i != index) continue;
						int pictureSize = picIt->data().size();
						metadata_editor_retvm_if(pictureSize == 0, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Size of picture is 0");
						META_MALLOC(*picture, pictureSize);
						metadata_editor_retvm_if(*picture == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

						memcpy(*picture, picIt->data().data(), pictureSize);
						*size = pictureSize;
						if (picIt->format() == TagLib::MP4::CoverArt::JPEG)	*mime_type = strndup(TYPE_JPEG, strlen(TYPE_JPEG));
						else if (picIt->format() == TagLib::MP4::CoverArt::PNG)	*mime_type = strndup(TYPE_PNG, strlen(TYPE_PNG));
						else							*mime_type = NULL;
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			} else {										// Item was not found - no pictures in file
				metadata_editor_error("No item <covr> in file. No pictures in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*) _metadata->file;
			TagLib::List<TagLib::FLAC::Picture*> lst = _file->pictureList();
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in FLAC file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {
				// Check if the index is in range of CoverArtList Item
				if ((index < 0) || ((uint)index >= lst.size())) {			// it is not
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {													// index is in range
					// Consecutive check of all pictures until the desired one is found
					int i = 0;
					for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator picIt = lst.begin(); picIt != lst.end(); ++picIt, ++i) {
						if (i != index) continue;
						// picture can be received as ByteVector (picIt->data()).
						// ByteVector has data() - picture itself and size() - the size of picture in data() method
						int pictureSize = (*picIt)->data().size();
						metadata_editor_retvm_if(pictureSize == 0, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Size of picture is 0");
						META_MALLOC(*picture, pictureSize);
						metadata_editor_retvm_if(*picture == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

						memcpy(*picture, (*picIt)->data().data(), pictureSize);
						*size = pictureSize;
						TagLib::String mime = (*picIt)->mimeType();
						if (!strcmp(mime.toCString(), "image/jpeg"))
							*mime_type = strndup(TYPE_JPEG, strlen(TYPE_JPEG));
						else if (!strcmp(mime.toCString(), "image/png"))
							*mime_type = strndup(TYPE_PNG, strlen(TYPE_PNG));
						else
							*mime_type = NULL;
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			}
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;		// Bring the pointer to actual file type
			TagLib::ID3v2::Tag* tag2 = _file->tag();
			if (!tag2) {
				metadata_editor_error("No ID3v2 tag in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			TagLib::ID3v2::FrameList lst = tag2->frameListMap()["APIC"];
			// Check if there are pictures in the tag
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {						// pictures exist in file
				// Check if index is correct or not
				if ((index < 0) || (lst.size() <= (uint)index)) {
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {					// everything is correct - begin extraction
					metadata_editor_info("There are %u pictures in file. Start of picture number %d extraction", lst.size(), index);
					int i = 0;
					// Among all frames we must choose that one with specified index. "i" will be counter
					for (TagLib::ID3v2::FrameList::Iterator it = lst.begin(); it != lst.end(); ++it, ++i) {
						if (i != index) continue;
						TagLib::ID3v2::AttachedPictureFrame* pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);
						uint pictureSize = pictureFrame->picture().size();
						metadata_editor_retvm_if(pictureSize == 0, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Size of picture is 0");

						META_MALLOC(*picture, pictureSize);
						metadata_editor_retvm_if(*picture == NULL, METADATA_EDITOR_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

						memcpy(*picture, pictureFrame->picture().data(), pictureSize);
						*size = pictureSize;
						TagLib::String mime = pictureFrame->mimeType();
						if (!strcmp(mime.toCString(), "image/jpeg"))
							*mime_type = strndup(TYPE_JPEG, strlen(TYPE_JPEG));
						else if (!strcmp(mime.toCString(), "image/png"))
							*mime_type = strndup(TYPE_PNG, strlen(TYPE_PNG));
						else
							*mime_type = NULL;
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			}
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function appends a cover art picture to the file *** //
extern "C" int metadata_editor_append_picture(metadata_editor_h metadata, const char *path)
{
	int ret = METADATA_EDITOR_ERROR_NONE;
	void *picture = NULL;
	int size = 0;
	char *type = NULL;

	// Check if we have valid arguments to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");
	metadata_editor_retvm_if(path == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	metadata_editor_s* _metadata = (metadata_editor_s*) metadata;
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	ret = __metadata_editor_get_picture_info(path, &picture, &size, &type);
	metadata_editor_retvm_if(ret != METADATA_EDITOR_ERROR_NONE, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");

	switch (_metadata->filetype) {					// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->ID3v2Tag(true);
			// Check if the valid tag pointer exists
			if (tag2 == NULL) {
				metadata_editor_error("Error. ID3v2 tag was not created. Can not proceed metadata updating\n");
				ret = METADATA_EDITOR_ERROR_OPERATION_FAILED;
				break;
			}
			TagLib::ID3v2::AttachedPictureFrame* pictureFrame = new TagLib::ID3v2::AttachedPictureFrame();
			if (pictureFrame == NULL) {
				metadata_editor_error("OUT_OF_MEMORY\n");
				ret = METADATA_EDITOR_ERROR_OUT_OF_MEMORY;
				break;
			}
			metadata_editor_info("New APIC frame will be added to the ID3v2 tag\n");
			pictureFrame->setPicture(TagLib::ByteVector((char*)picture, size));
			pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
			pictureFrame->setMimeType(type);

			tag2->addFrame(pictureFrame);

			ret = METADATA_EDITOR_ERROR_NONE;
			break;
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
			TagLib::MP4::Tag* tag = _file->tag();
			if (!tag) {
				metadata_editor_error("Tag was not created\n");
				ret = METADATA_EDITOR_ERROR_OPERATION_FAILED;
				break;
			}

			// Get map of items directly from tag and launch a search of specific item
			TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
			TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find("covr");
			if (it != itemMap.end()) {									// Item was found
				metadata_editor_info("The item <covr> exists. Adding picture\n");
				TagLib::MP4::CoverArtList lst = it->second.toCoverArtList();
				TagLib::MP4::CoverArt::Format format;
				if (strncmp(type, "image/jpeg", strlen("image/jpeg")) == 0)
					format = TagLib::MP4::CoverArt::JPEG;
				else if (strncmp(type, "image/png", strlen("image/jpeg")) == 0)
					format = TagLib::MP4::CoverArt::PNG;
				else
					format = (TagLib::MP4::CoverArt::Format)0xFFFF;
				TagLib::MP4::CoverArt cover(format, TagLib::ByteVector((char*)picture, size));
				lst.append(cover);
				itemMap.insert("covr", TagLib::MP4::Item(lst));

				ret = METADATA_EDITOR_ERROR_NONE;
				break;
			} else {											// Item was not found
				metadata_editor_info("The item <covr> does not exist. Adding picture\n");
				TagLib::MP4::CoverArt::Format format;
				if (strncmp(type, "image/jpeg", strlen("image/jpeg")) == 0)
					format = TagLib::MP4::CoverArt::JPEG;
				else if (strncmp(type, "image/png", strlen("image/jpeg")) == 0)
					format = TagLib::MP4::CoverArt::PNG;
				else
					format = (TagLib::MP4::CoverArt::Format)0xFFFF;
				TagLib::MP4::CoverArt cover(format, TagLib::ByteVector((char*)picture, size));
				TagLib::MP4::CoverArtList lst;
				lst.append(cover);
				itemMap.insert("covr", TagLib::MP4::Item(lst));

				ret = METADATA_EDITOR_ERROR_NONE;
				break;
			}
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*) _metadata->file;
			TagLib::FLAC::Picture* frontCover = new TagLib::FLAC::Picture;
			if (frontCover == NULL) {
				metadata_editor_error("OUT_OF_MEMORY\n");
				ret = METADATA_EDITOR_ERROR_OUT_OF_MEMORY;
				break;
			}
			frontCover->setData(TagLib::ByteVector((char*)picture, size));
			frontCover->setType(TagLib::FLAC::Picture::FrontCover);
			frontCover->setMimeType(type);

			metadata_editor_info("Picture will be added to FLAC file\n");
			_file->addPicture(frontCover);
			ret = METADATA_EDITOR_ERROR_NONE;
			break;
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->tag();
			// Check if the valid tag pointer exists
			if (tag2 == NULL) {
				metadata_editor_error("Error. ID3v2 tag was not created. Can not proceed metadata updating\n");
				ret = METADATA_EDITOR_ERROR_OPERATION_FAILED;
				break;
			}
			TagLib::ID3v2::AttachedPictureFrame* pictureFrame = new TagLib::ID3v2::AttachedPictureFrame();
			if (pictureFrame == NULL) {
				metadata_editor_error("OUT_OF_MEMORY\n");
				ret = METADATA_EDITOR_ERROR_OUT_OF_MEMORY;
				break;
			}
			metadata_editor_info("New APIC frame will be added to the ID3v2 tag\n");
			pictureFrame->setPicture(TagLib::ByteVector((char*)picture, size));
			pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
			pictureFrame->setMimeType(type);
			tag2->addFrame(pictureFrame);
			ret = METADATA_EDITOR_ERROR_NONE;
			break;
		}
#endif
		default:
		{
			metadata_editor_error("Wrong file type\n");
			ret = METADATA_EDITOR_ERROR_NOT_SUPPORTED;
			break;
		}
	}

	META_SAFE_FREE(picture);
	META_SAFE_FREE(type);

	return ret;
}

// *** This function is used to delete picture with specified index *** //
extern "C" int metadata_editor_remove_picture(metadata_editor_h metadata, int index)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID handler.");

	metadata_editor_s* _metadata = (metadata_editor_s*) metadata;
	metadata_editor_retvm_if(_metadata->file == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "File loading fail.\n");

	// Check if the file, given through metadata, exists and is opened correctly
	metadata_editor_retvm_if(_metadata->file && _metadata->isOpen == false, METADATA_EDITOR_ERROR_PERMISSION_DENIED, "File does not exist or you have no rights to open it\n");
	metadata_editor_retvm_if(_metadata->isReadOnly, METADATA_EDITOR_ERROR_OPERATION_FAILED, "File is readonly. Unable to modify\n");

	switch (_metadata->filetype) {					// Process the file according to the specified file type
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->ID3v2Tag(true);
			// Check if the valid tag pointer exists
			metadata_editor_retvm_if(tag2 == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. ID3v2 tag was not created. Can not proceed metadata updating");
			TagLib::ID3v2::FrameList lst = tag2->frameListMap()["APIC"];
			// Check if there are pictures in the tag
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {		// pictures exist in file
				// Check if index is correct or not
				if ((index < 0) || (lst.size() <= (uint)index)) {
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {		// everything is correct - begin extraction
					metadata_editor_info("Removing of picture number %d\n", index);
					int i = 0;
					// Among all frames we must choose that one with specified index. "i" will be counter
					for (TagLib::ID3v2::FrameList::Iterator it = lst.begin(); it != lst.end(); ++it, ++i) {
						if (i != index) continue;
						tag2->removeFrame(*it);
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			}
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			TagLib::MP4::File* _file = (TagLib::MP4::File*) _metadata->file;
			TagLib::MP4::Tag* tag = _file->tag();
			metadata_editor_retvm_if(tag == NULL, METADATA_EDITOR_ERROR_OPERATION_FAILED, "Error. tag not exist.");

			// Get map of items directly from tag and launch a search of specific item
			TagLib::MP4::ItemListMap& itemMap = tag->itemListMap();
			TagLib::MP4::ItemListMap::ConstIterator it = itemMap.find("covr");
			if (it != itemMap.end()) {									// Item was found
				TagLib::MP4::CoverArtList lst = it->second.toCoverArtList();
				// Check if the index is in range of CoverArtList Item
				if ((index < 0) || ((uint)index >= lst.size())) {					// it is not
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {										// index is in range
					metadata_editor_info("The picture number %d will be deleted\n", index);
					int i = 0;
					for (TagLib::MP4::CoverArtList::Iterator picIt = lst.begin(); picIt != lst.end(); ++picIt, ++i) {
						if (i != index) continue;
						lst.erase(picIt);
						break;
					}
					itemMap.insert("covr", TagLib::MP4::Item(lst));
					return METADATA_EDITOR_ERROR_NONE;
				}
			} else {				// Item was not found
				metadata_editor_error("The item <covr> does not exist. Nothing to delete\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*) _metadata->file;
			TagLib::List<TagLib::FLAC::Picture*> lst = _file->pictureList();
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in file. Nothing to delete\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			// Check if the index is in range of CoverArtList Item
			if ((index < 0) || ((uint)index >= lst.size())) {						// it is not
				metadata_editor_error("Index of picture is out of range\n");
				return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
			} else {											// index is in range
				metadata_editor_info("The picture number %d will be deleted\n", index);
				int i = 0;
				for (TagLib::List<TagLib::FLAC::Picture*>::Iterator picIt = lst.begin(); picIt != lst.end(); ++picIt, ++i) {
					if (i != index) continue;
					_file->removePicture(*picIt, true);
					break;
				}
				return METADATA_EDITOR_ERROR_NONE;
			}
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			// Bring the pointer to actual file type and make tags pointers
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			TagLib::ID3v2::Tag* tag2 = _file->tag();
			// Check if the valid tag pointer exists
			if (tag2 == NULL) {
				metadata_editor_error("Error. ID3v2 tag does not exist. Can not remove picture\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			}
			TagLib::ID3v2::FrameList lst = tag2->frameListMap()["APIC"];
			// Check if there are pictures in the tag
			if (lst.isEmpty()) {
				metadata_editor_error("No pictures in file\n");
				return METADATA_EDITOR_ERROR_OPERATION_FAILED;
			} else {		// pictures exist in file
				// Check if index is correct or not
				if ((index < 0) || (lst.size() <= (uint)index)) {
					metadata_editor_error("Index of picture is out of range\n");
					return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
				} else {		// everything is correct - begin extraction
					metadata_editor_info("Removing of picture number %d\n", index);
					int i = 0;
					// Among all frames we must choose that one with specified index. "i" will be counter
					for (TagLib::ID3v2::FrameList::Iterator it = lst.begin(); it != lst.end(); ++it, ++i) {
						if (i != index) continue;
						tag2->removeFrame(*it);
						break;
					}
					return METADATA_EDITOR_ERROR_NONE;
				}
			}
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}
}

// *** This function is used to free memory that was allocated with metadata_editor_create(...) and metadata_editor_set_path(...) functions *** //
extern "C" int metadata_editor_destroy(metadata_editor_h metadata)
{
	// Check if we have a valid argument to work with
	metadata_editor_retvm_if(metadata == NULL, METADATA_EDITOR_ERROR_INVALID_PARAMETER, "INVALID PARAMETER");

	metadata_editor_s *_metadata = (metadata_editor_s*)metadata;

	switch (_metadata->filetype) {
		case METADATA_EDITOR_FORMAT_MP3:
		{
			// Bring the pointer to actual file type
			TagLib::MPEG::File* _file = (TagLib::MPEG::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
		case METADATA_EDITOR_FORMAT_MP4:
		{
			TagLib::MP4::File* _file = (TagLib::MP4::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
#if 0
		case METADATA_EDITOR_FORMAT_FLAC:
		{
			TagLib::FLAC::File* _file = (TagLib::FLAC::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
		case METADATA_EDITOR_FORMAT_OGG_VORBIS:
		{
			TagLib::Ogg::Vorbis::File* _file = (TagLib::Ogg::Vorbis::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
		case METADATA_EDITOR_FORMAT_OGG_FLAC:
		{
			TagLib::Ogg::FLAC::File* _file = (TagLib::Ogg::FLAC::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
		case METADATA_EDITOR_FORMAT_WAV:
		{
			TagLib::RIFF::WAV::File* _file = (TagLib::RIFF::WAV::File*)_metadata->file;
			metadata_editor_info("file free [%lX]", _metadata->file);
			delete _file;
			break;
		}
#endif
		default:
			metadata_editor_error("Wrong file type\n");
			return METADATA_EDITOR_ERROR_INVALID_PARAMETER;
	}

	metadata_editor_info("<metadata_editor_s> with address %lX will be freed\n", metadata);
	delete _metadata;

	return METADATA_EDITOR_ERROR_NONE;
}
